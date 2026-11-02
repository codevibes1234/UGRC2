// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

#include "rocev2.h"
#include "tcp.h"
#include "config.h"
#include <algorithm>
#include <sstream>

// ============================================================================
// ============================================================================

uint32_t RoCEv2Src::_next_flow_id = 0;
bool RoCEv2Src::_flows_paused = false;
std::list<RoCEv2Src*> RoCEv2Src::_live_flows;
uint32_t RoCEv2Src::default_initial_credits = 34;

// ============================================================================
// ============================================================================

RoCEv2Src::RoCEv2Src(TrafficLogger* pktlogger,
                     EventList& eventlist,
                     int flow_src,
                     int flow_dst,
                     void (*app_callback)(void*),
                     void* app_callback_data)
    : PacketSink(),
      EventSource(eventlist, "RoCEv2Src"),
            PacketFlow(pktlogger),
      _sink(nullptr),
      _flow_src(flow_src),
      _flow_dst(flow_dst),
      _flow_id(_next_flow_id++),
      _flow_size(0),
      _bytes_sent(0),
      _bytes_acked(0),
      _finished(false),
    _send_pending(false),
    _paused_send_pending(false),
      _finish_time(0),
    _available_credits(default_initial_credits),
      _packet_size(9000),
    _initial_credits(default_initial_credits),
      _routeout(nullptr),
      _routeback(nullptr),
      _start_time(0),
      _last_send_time(0),
      _pktlogger(pktlogger),
      _application_callback(app_callback),
      _application_callback_data(app_callback_data)
{
    _live_flows.push_back(this);

    std::stringstream ss;
    ss << "RoCEv2Src(" << flow_src << "->" << flow_dst << ")";
    _nodename = ss.str();
}

RoCEv2Src::~RoCEv2Src()
{
    _live_flows.remove(this);
}


void RoCEv2Src::pause_all_flows()
{
    _flows_paused = true;
}

void RoCEv2Src::resume_all_flows()
{
    _flows_paused = false;
}

void RoCEv2Src::resume_flow()
{
    if (_finished || !_paused_send_pending)
        return;
    _paused_send_pending = false;

    if (!_send_pending) {
        _send_pending = true;
        eventlist().sourceIsPending(*this, eventlist().now());
    }
}

void RoCEv2Src::update_route(const Route* routeout, const Route* routein)
{
    delete _routeout;
    delete _routeback;
    _routeout = routeout;
    _routeback = routein;
}

// ============================================================================
// ============================================================================

void RoCEv2Src::connect(const Route& routeout,
                        const Route& routeback,
                        RoCEv2Sink& sink,
                        simtime_picosec startTime)
{
    _routeout = &routeout;
    _routeback = &routeback;
    _sink = &sink;
    _start_time = startTime;
    _last_send_time = startTime;

    sink.set_src(this);

    _send_pending = true;
    eventlist().sourceIsPending(*this, startTime);
}

// ============================================================================
// ============================================================================

void RoCEv2Src::doNextEvent()
{
    _send_pending = false;

    if (_finished) {
        return;
    }

    send_packets();
}

// ============================================================================
// ============================================================================

void RoCEv2Src::send_packets()
{
    if (_flow_size == 0) {
        finish_flow();
        return;
    }

    if (_flows_paused) {
        _paused_send_pending = true;
        return;
    }

    while (_available_credits > 0 && _bytes_sent < _flow_size) {
        uint64_t remaining = _flow_size - _bytes_sent;
        uint32_t bytes_this_pkt = std::min((uint64_t)_packet_size, remaining);

        RoCEv2Packet* pkt = new RoCEv2Packet();
        pkt->set_rocev2_type(RoCEv2Packet::ROCEV2_DATA);
        pkt->set_psn(_bytes_sent / _packet_size);
        pkt->set_size(bytes_this_pkt + 40);

        pkt->set_route(*this, *_routeout, bytes_this_pkt + 40, pkt->get_psn());

        if (_pktlogger) {
            _pktlogger->logTraffic(*pkt, static_cast<PacketFlow&>(*this), TrafficLogger::PKT_CREATE);
        }

        pkt->sendOn();

        _bytes_sent += bytes_this_pkt;
        _available_credits--;
        _last_send_time = eventlist().now();
    }

}

// ============================================================================
// ============================================================================

void RoCEv2Src::receivePacket(Packet& pkt)
{
    RoCEv2Packet* roce_pkt = dynamic_cast<RoCEv2Packet*>(&pkt);
    if (!roce_pkt) {
        return;
    }

    if (roce_pkt->get_rocev2_type() == RoCEv2Packet::ROCEV2_CREDIT) {
        uint32_t new_credits = roce_pkt->get_credits();
        if (roce_pkt->get_ackno() > _bytes_acked) {
            _bytes_acked = roce_pkt->get_ackno();
        }
        handle_credit_update(new_credits);

        if (_bytes_acked >= _flow_size) {
            finish_flow();
        }

        if (_pktlogger) {
            _pktlogger->logTraffic(*roce_pkt, static_cast<PacketFlow&>(*this), TrafficLogger::PKT_CREATESEND);
        }

        roce_pkt->free();

    }
}

// ============================================================================
// ============================================================================

void RoCEv2Src::handle_credit_update(uint32_t new_credits)
{
    _available_credits += new_credits;

    if (_available_credits > 0 && !_finished) {
        if (!_send_pending) {
            _send_pending = true;
            eventlist().sourceIsPending(*this, eventlist().now());
        }
    }
}

// ============================================================================
// ============================================================================

void RoCEv2Src::finish_flow()
{
    if (_finished) {
        return;
    }

    _finished = true;
    _finish_time = eventlist().now();

    if (_application_callback) {
        _application_callback(_application_callback_data);
    }
}

// ============================================================================
// ============================================================================

RoCEv2Sink::RoCEv2Sink(TrafficLogger* pktlogger,
                       EventList& eventlist)
    : PacketSink(),
      EventSource(eventlist, "RoCEv2Sink"),
            PacketFlow(pktlogger),
      _src(nullptr),
      _bytes_received(0),
      _cumulative_ack(0),
      _bytes_per_credit_unit(9000),
      _pktlogger(pktlogger)
{
    _nodename = "RoCEv2Sink";
}

RoCEv2Sink::~RoCEv2Sink()
{
}

// ============================================================================
// ============================================================================

void RoCEv2Sink::receivePacket(Packet& pkt)
{
    RoCEv2Packet* roce_pkt = dynamic_cast<RoCEv2Packet*>(&pkt);
    if (!roce_pkt) {
        return;
    }

    if (roce_pkt->get_rocev2_type() == RoCEv2Packet::ROCEV2_DATA) {
        _bytes_received += roce_pkt->size() - 40;
        _cumulative_ack = _bytes_received;

        if (_pktlogger) {
            _pktlogger->logTraffic(*roce_pkt, static_cast<PacketFlow&>(*this), TrafficLogger::PKT_ARRIVE);
        }

        send_credit_update();

        roce_pkt->free();
    }
}

// ============================================================================
// ============================================================================

void RoCEv2Sink::send_credit_update()
{
    if (!_src) {
        return;
    }

    RoCEv2Packet* credit_pkt = new RoCEv2Packet();
    credit_pkt->set_rocev2_type(RoCEv2Packet::ROCEV2_CREDIT);
    credit_pkt->set_credits(1);
    credit_pkt->set_ackno(_cumulative_ack);
    credit_pkt->set_size(40);

    credit_pkt->set_route(*this, *_src->_routeback, 40, _cumulative_ack);

    if (_pktlogger) {
        _pktlogger->logTraffic(*credit_pkt, static_cast<PacketFlow&>(*this), TrafficLogger::PKT_CREATE);
    }

    credit_pkt->sendOn();
}

// ============================================================================
// ============================================================================

void RoCEv2Sink::doNextEvent()
{
}

