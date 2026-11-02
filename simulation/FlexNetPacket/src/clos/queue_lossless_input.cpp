// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-        
#include "queue_lossless_input.h"
#include <math.h>
#include <iostream>
#include <sstream>
#include "switch.h"
#include <map>

class PauseDelivery : public EventSource {
public:
    PauseDelivery(EventList &ev) : EventSource(ev, "PauseDelivery") {}
    void schedule(simtime_picosec when, Queue *target, unsigned wait, int pfc_class = 0) {
        _pending.insert(std::make_pair(when, std::make_pair(target, std::make_pair(wait, pfc_class))));
        eventlist().sourceIsPending(*this, when);
    }
    void doNextEvent() {
        assert(!_pending.empty());
        auto it = _pending.begin();
        Queue *target = it->second.first;
        unsigned wait = it->second.second.first;
        int pfc_class = it->second.second.second;
        _pending.erase(it);
        EthPausePacket *pkt = EthPausePacket::newpkt(wait, pfc_class);
        target->receivePacket(*pkt);
    }
private:
    std::multimap<simtime_picosec, std::pair<Queue*, std::pair<unsigned, int>>> _pending;
};

simtime_picosec LosslessInputQueue::cfg_pause_delay = 0;

double   LosslessInputQueue::cfg_buffer_pkts = 20;
double   LosslessInputQueue::cfg_xoff_pkts   = 15;
double   LosslessInputQueue::cfg_xon_pkts    = 12;
mem_b    LosslessInputQueue::stat_peak_bytes = 0;
uint64_t LosslessInputQueue::stat_overflow_pkts = 0;
uint64_t LosslessInputQueue::stat_arrivals = 0;
uint64_t LosslessInputQueue::stat_pause_sent = 0;
uint64_t LosslessInputQueue::stat_resume_sent = 0;
mem_b    LosslessInputQueue::stat_peak_overshoot_bytes = 0;
uint64_t LosslessInputQueue::stat_peak_overshoot_link_bps = 0;
uint64_t LosslessInputQueue::stat_overflow_scaled_pkts = 0;
int      LosslessInputQueue::cfg_classes = 1;
uint64_t LosslessInputQueue::stat_class_saturated = 0;

void LosslessInputQueue::configure(double xoff_pkts, double xon_pkts, double buffer_pkts)
{
    assert(xoff_pkts > 0 && xon_pkts > 0);
    assert(xoff_pkts > xon_pkts);
    assert(buffer_pkts >= xoff_pkts);
    cfg_xoff_pkts = xoff_pkts;
    cfg_xon_pkts = xon_pkts;
    cfg_buffer_pkts = buffer_pkts;
}

void LosslessInputQueue::print_stats(std::ostream &os)
{
    const double p = (double)Packet::data_packet_size();
    os << "PFC_pause_delay_ps " << cfg_pause_delay << "\n"
       << "PFC_xoff_pkts " << cfg_xoff_pkts << "\n"
       << "PFC_xon_pkts " << cfg_xon_pkts << "\n"
       << "PFC_buffer_pkts " << cfg_buffer_pkts << "\n"
       << "PFC_arrivals " << stat_arrivals << "\n"
       << "PFC_pause_sent " << stat_pause_sent << "\n"
       << "PFC_resume_sent " << stat_resume_sent << "\n"
       << "PFC_peak_occupancy_pkts " << stat_peak_bytes / p << "\n"
       << "PFC_peak_overshoot_pkts " << stat_peak_overshoot_bytes / p << "\n"
       << "PFC_peak_overshoot_link_gbps " << stat_peak_overshoot_link_bps / 1e9 << "\n"
       << "PFC_overflow_speed_scaled_headroom_pkts " << stat_overflow_scaled_pkts << "\n"
       << "PFC_overflow_pkts " << stat_overflow_pkts << "\n";
    if (cfg_classes > 1)
	os << "PFC_classes " << cfg_classes << "\n"
	   << "PFC_class_saturated " << stat_class_saturated << "\n";
}

LosslessInputQueue::LosslessInputQueue(EventList& eventlist)
    : Queue(0,(mem_b)(Packet::data_packet_size()*cfg_buffer_pkts),eventlist,NULL),
      VirtualQueue(),
      _state_recv(READY)
{
    _high_threshold = (int)(Packet::data_packet_size()*cfg_xoff_pkts);
    _low_threshold = (int)(Packet::data_packet_size()*cfg_xon_pkts);

    assert(_high_threshold>0);
    assert(_high_threshold > _low_threshold);
    if (cfg_classes > 1) {
	_class_bytes.assign(cfg_classes, 0);
	_class_state.assign(cfg_classes, READY);
    }
}

LosslessInputQueue::LosslessInputQueue(EventList& eventlist,Queue* peer)
    : Queue(0,(mem_b)(Packet::data_packet_size()*cfg_buffer_pkts),eventlist,NULL),
      VirtualQueue(),
      _state_recv(READY)
{
    _high_threshold = (int)(Packet::data_packet_size()*cfg_xoff_pkts);
    _low_threshold = (int)(Packet::data_packet_size()*cfg_xon_pkts);

    assert(_high_threshold>0);
    assert(_high_threshold > _low_threshold);

    if (cfg_classes > 1) {
	_class_bytes.assign(cfg_classes, 0);
	_class_state.assign(cfg_classes, READY);
    }

    stringstream ss;
    ss << "VirtualQueue("<< peer->_name<< ")";
    _nodename = ss.str();
    _remoteEndpoint = peer;

    peer->setRemoteEndpoint(this);
}

void
LosslessInputQueue::receivePacket(Packet& pkt) 
{
    if (cfg_classes > 1) {
	int c = pkt_class(pkt);
	if (c > cfg_classes - 1) c = cfg_classes - 1;
	set_pkt_in_class(pkt, c);
	_queuesize += pkt.size();
	_class_bytes[c] += pkt.size();
	stat_arrivals++;
	if (_class_bytes[c] > stat_peak_bytes) stat_peak_bytes = _class_bytes[c];
	if (_class_state[c] == PAUSED && _class_bytes[c] - _high_threshold > stat_peak_overshoot_bytes) {
	    stat_peak_overshoot_bytes = _class_bytes[c] - _high_threshold;
	    stat_peak_overshoot_link_bps = getRemoteEndpoint() ? getRemoteEndpoint()->_bitrate : 0;
	}
	if (_class_bytes[c] > _high_threshold && _class_state[c] != PAUSED) {
	    _class_state[c] = PAUSED;
	    stat_pause_sent++;
	    sendPause(1000, c);
	}
	if (_class_bytes[c] > _maxsize) {
	    stat_overflow_pkts++;
	    cout << " Queue " << _name << " class " << c
		 << " LOSSLESS not working! I should have dropped this packet" << endl;
	}
	pkt.sendOn2(this);
	return;
    }

    /* normal packet, enqueue it */
    _queuesize += pkt.size();

    stat_arrivals++;
    if (_queuesize > stat_peak_bytes) stat_peak_bytes = _queuesize;
    if (_state_recv == PAUSED && _queuesize - _high_threshold > stat_peak_overshoot_bytes) {
	stat_peak_overshoot_bytes = _queuesize - _high_threshold;
	stat_peak_overshoot_link_bps = getRemoteEndpoint() ? getRemoteEndpoint()->_bitrate : 0;
    }
    {
	double scale = 1.0;
	if (getRemoteEndpoint() && getRemoteEndpoint()->_bitrate > speedFromMbps((uint64_t)100000))
	    scale = (double)getRemoteEndpoint()->_bitrate / (double)speedFromMbps((uint64_t)100000);
	double limit = _high_threshold + (_maxsize - _high_threshold) * scale;
	if ((double)_queuesize > limit) stat_overflow_scaled_pkts++;
    }

    //send PAUSE notifications if that is the case!
    if (_queuesize > _high_threshold && _state_recv!=PAUSED){
	_state_recv = PAUSED;
	stat_pause_sent++;
	sendPause(1000);
    }

    //if (_state_recv==PAUSED)
    //cout << timeAsMs(eventlist().now()) << " queue " << _name << " switch (" << _switch->_name << ") "<< " recv when paused pkt " << pkt.type() << " sz " << _queuesize << endl;	

    if (_queuesize > _maxsize){
	stat_overflow_pkts++;
	cout << " Queue " << _name << " LOSSLESS not working! I should have dropped this packet" << endl;
    }

    //tell the output queue we're here!
    pkt.sendOn2(this);
}

void LosslessInputQueue::completedService(Packet& pkt){
    if (cfg_classes > 1) {
	const int c = pkt_in_class(pkt);
	_queuesize -= pkt.size();
	_class_bytes[c] -= pkt.size();
	if (_class_bytes[c] < _low_threshold && _class_state[c] == PAUSED) {
	    stat_resume_sent++;
	    sendPause(0, c);
	    _class_state[c] = READY;
	}
	return;
    }
    _queuesize -= pkt.size();

    //unblock if that is the case
    if (_queuesize < _low_threshold && _state_recv == PAUSED) {
	stat_resume_sent++;
	sendPause(0);
	_state_recv = READY;
    }
}

void LosslessInputQueue::sendPause(unsigned int wait, int pfc_class){
    cout << "Ingress link " << getRemoteEndpoint()->_name << " PAUSE " << wait
	 << " class " << pfc_class << endl;
    if (cfg_pause_delay == 0) {
	EthPausePacket* pkt = EthPausePacket::newpkt(wait, pfc_class);
	getRemoteEndpoint()->receivePacket(*pkt);
	return;
    }
    static PauseDelivery *delivery = nullptr;
    if (delivery == nullptr) delivery = new PauseDelivery(eventlist());
    delivery->schedule(eventlist().now() + cfg_pause_delay, getRemoteEndpoint(), wait, pfc_class);
}

void LosslessInputQueue::sendPause(unsigned int wait){
    cout << "Ingress link " << getRemoteEndpoint()->_name << " PAUSE " << wait << endl;
    //cout << "Informing " << getRemoteEndpoint()->_name << endl;
    if (cfg_pause_delay == 0) {
	EthPausePacket* pkt = EthPausePacket::newpkt(wait);
	getRemoteEndpoint()->receivePacket(*pkt);
	return;
    }
    static PauseDelivery *delivery = nullptr;
    if (delivery == nullptr) delivery = new PauseDelivery(eventlist());
    delivery->schedule(eventlist().now() + cfg_pause_delay, getRemoteEndpoint(), wait);
};

