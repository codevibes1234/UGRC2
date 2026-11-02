// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-        
#include <math.h>
#include <iostream>
#include <sstream>
#include "switch.h"
#include "queue_lossless_output.h"
#include "queue_lossless_input.h"
#include <algorithm>

mem_b    LosslessOutputQueue::stat_peak_bytes = 0;
mem_b    LosslessOutputQueue::stat_peak_transit_bytes = 0;
uint64_t LosslessOutputQueue::stat_overflow_local = 0;
uint64_t LosslessOutputQueue::stat_overflow_transit = 0;
uint64_t LosslessOutputQueue::stat_overflow_transit_only = 0;

void LosslessOutputQueue::print_stats(std::ostream &os)
{
    const double p = (double)Packet::data_packet_size();
    os << "EGRESS_peak_occupancy_pkts " << stat_peak_bytes / p << "\n"
       << "EGRESS_peak_transit_pkts " << stat_peak_transit_bytes / p << "\n"
       << "EGRESS_overflow_local_pkts " << stat_overflow_local << "\n"
       << "EGRESS_overflow_transit_pkts " << stat_overflow_transit << "\n"
       << "EGRESS_overflow_transit_only_pkts " << stat_overflow_transit_only << "\n";
}

LosslessOutputQueue::LosslessOutputQueue(linkspeed_bps bitrate, mem_b maxsize, 
					 EventList& eventlist, QueueLogger* logger, int ECN, int K)
    : Queue(bitrate,maxsize,eventlist,logger), 
      _state_send(READY)
{
    //assume worst case: PAUSE frame waits for one MSS packet to be sent to other switch, and there is 
    //an MSS just beginning to be sent when PAUSE frame arrives; this means 2 packets per incoming
    //port, and we must have buffering for all ports except this one (assuming no one hop cycles!)

    _sending = 0;

    _ecn_enabled = ECN;
    _K = K;

    _nclasses = LosslessInputQueue::cfg_classes;
    if (_nclasses > 1) {
	_class_q.resize(_nclasses);
	_class_vq.resize(_nclasses);
	_class_state.assign(_nclasses, READY);
    }

    stringstream ss;
    ss << "queue lossless output(" << bitrate/1000000 << "Mb/s," << maxsize << "bytes)";
    _nodename = ss.str();
}


void
LosslessOutputQueue::receivePacket(Packet& pkt){
    receivePacket(pkt,NULL);
}

void
LosslessOutputQueue::receivePacket(Packet& pkt,VirtualQueue* prev) 
{
    //is this a PAUSE frame? 
    if (pkt.type()==ETH_PAUSE && _nclasses > 1){
	EthPausePacket* p = (EthPausePacket*)&pkt;
	const int c = std::min((int)p->pfcClass(), _nclasses - 1);
	if (p->sleepTime() > 0)
	    _class_state[c] = PAUSED;
	else {
	    _class_state[c] = READY;
	    if (!_sending && _circuit_up && pick_class(false) >= 0)
		beginService();
	}
	pkt.free();
	return;
    }

    if (pkt.type()==ETH_PAUSE){
	EthPausePacket* p = (EthPausePacket*)&pkt;

	if (p->sleepTime()>0){
	    //remote end is telling us to shut up.
	    //assert(_state_send == READY);
	    if (_sending)
		//we have a packet in flight
		_state_send = PAUSE_RECEIVED;
	    else
		_state_send = PAUSED;

	    //cout << timeAsMs(eventlist().now()) << " " << _name << " PAUSED "<<endl;	    
	}
	else {
	    //we are allowed to send!
	    _state_send = READY;
	    //cout << timeAsMs(eventlist().now()) << " " << _name << " GO "<<endl;

	    //start transmission if we have packets to send!
	    if(_enqueued.size()>0&&!_sending&&_circuit_up)
		beginService();
	}
	
	pkt.free();
	return;
    }

    /* normal packet, enqueue it */

    //remember the virtual queue that has sent us this packet; will notify the vq once the packet has left our buffer.

    pkt.flow().logTraffic(pkt, *this, TrafficLogger::PKT_ARRIVE);

    bool queueWasEmpty = _enqueued.empty();

    if (_nclasses > 1) {
	int cl = LosslessInputQueue::pkt_class(pkt);
	if (prev != NULL) {
	    cl++;
	    if (cl > _nclasses - 1) { cl = _nclasses - 1; LosslessInputQueue::stat_class_saturated++; }
	    LosslessInputQueue::set_pkt_class(pkt, cl);
	}
	cl = std::min(cl, _nclasses - 1);
	_class_q[cl].push_front(&pkt);
	_class_vq[cl].push_front(prev);
    } else
	_vq.push_front(prev);
    _enqueued.push_front(&pkt);


    _queuesize += pkt.size();
    if (prev == NULL) _local_bytes += pkt.size();

    if (_queuesize > stat_peak_bytes) stat_peak_bytes = _queuesize;
    if (_queuesize - _local_bytes > stat_peak_transit_bytes)
	stat_peak_transit_bytes = _queuesize - _local_bytes;

    if (_queuesize > _maxsize){
	if (prev == NULL) stat_overflow_local++; else stat_overflow_transit++;
	if (_queuesize - _local_bytes > _maxsize) stat_overflow_transit_only++;
	cout << " Queue " << _name << " LOSSLESS not working! I should have dropped this packet" << endl;
    }

    if (_logger) 
	_logger->logQueue(*this, QueueLogger::PKT_ENQUEUE, pkt);

    if (_nclasses > 1) {
	if (!_sending && _circuit_up && pick_class(false) >= 0)
	    beginService();
	return;
    }

    if (queueWasEmpty && _state_send == READY && _circuit_up) {
	/* schedule the dequeue event */
	assert(_enqueued.size()==1);
	beginService();
    }
}

int LosslessOutputQueue::pick_class(bool advance){
    for (int i = 0; i < _nclasses; i++) {
	const int c = (_rr + i) % _nclasses;
	if (!_class_q[c].empty() && _class_state[c] == READY) {
	    if (advance) _rr = (c + 1) % _nclasses;
	    return c;
	}
    }
    return -1;
}

void LosslessOutputQueue::beginService(){
    if (_nclasses > 1) {
	assert(!_sending);
	assert(_circuit_up);
	const int c = pick_class(true);
	assert(c >= 0);
	_serving_class = c;
	_sending = 1;
	eventlist().sourceIsPendingRel(*this, drainTime(_class_q[c].back()));
	return;
    }
    assert(_state_send==READY&&!_sending);
    assert(_circuit_up);
    Queue::beginService();
    _sending = 1;
}

void LosslessOutputQueue::completeService(){
    /* dequeue the packet */
    assert(!_enqueued.empty());
    Packet* pkt;
    VirtualQueue* q;
    if (_nclasses > 1) {
	const int c = _serving_class;
	assert(c >= 0 && !_class_q[c].empty());
	pkt = _class_q[c].back();
	q = _class_vq[c].back();
	_class_q[c].pop_back();
	_class_vq[c].pop_back();
	auto it = std::find(_enqueued.rbegin(), _enqueued.rend(), pkt);
	assert(it != _enqueued.rend());
	_enqueued.erase(std::next(it).base());
	_serving_class = -1;
    } else {
	pkt = _enqueued.back();
	q = _vq.back();
	_enqueued.pop_back();
	_vq.pop_back();
    }

    //mark on deque
    if (_ecn_enabled && _queuesize > _K)
	  pkt->set_flags(pkt->flags() | ECN_CE);    

    _queuesize -= pkt->size();
    if (q == NULL) _local_bytes -= pkt->size();

    pkt->flow().logTraffic(*pkt, *this, TrafficLogger::PKT_DEPART);
    if (_logger) _logger->logQueue(*this, QueueLogger::PKT_SERVICE, *pkt);

    //tell the virtual input queue this packet is done!
    if (q != NULL)
	q->completedService(*pkt);

    /* tell the packet to move on to the next pipe */
    pkt->sendOn();

    _sending = 0;

    notify_dyn_sch_drained();

    //if (_state_send!=READY){
    //cout << timeAsMs(eventlist().now()) << " queue " << _name << " not ready but sending pkt " << pkt->type() << endl;
    //}

    if (_nclasses > 1) {
	if (_circuit_up && pick_class(false) >= 0)
	    beginService();
	return;
    }

    if (_state_send == PAUSE_RECEIVED)
	_state_send = PAUSED;

    if (!_enqueued.empty()) {
	if (_state_send == READY && _circuit_up)
	    /* start packet transmission, schedule the next dequeue event */
	    beginService();
    }
}
