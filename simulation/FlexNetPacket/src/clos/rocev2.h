// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#ifndef ROCEV2_H
#define ROCEV2_H


#include <list>
#include <map>
#include <cstdint>
#include "config.h"
#include "network.h"
#include "eventlist.h"
#include "loggers.h"
#include "route.h"

class RoCEv2Sink;
class RoCEv2Packet;

class RoCEv2Src : public PacketSink, public EventSource, public PacketFlow {
    friend class RoCEv2Sink;

public:
    RoCEv2Src(TrafficLogger* pktlogger,
              EventList& eventlist,
              int flow_src,
              int flow_dst,
              void (*acf)(void*) = nullptr,
              void* acd = nullptr);
    ~RoCEv2Src();

    virtual void receivePacket(Packet& pkt);
    virtual const string& nodename() { return _nodename; }
    virtual void doNextEvent();

    void connect(const Route& routeout,
                 const Route& routeback,
                 RoCEv2Sink& sink,
                 simtime_picosec startTime);

    void set_flowsize(uint64_t flow_size_in_bytes) { _flow_size = flow_size_in_bytes; }
    void set_initial_credits(uint32_t credits) { _available_credits = credits; }
    static uint32_t default_initial_credits;
    uint32_t get_available_credits() const { return _available_credits; }
    uint64_t get_bytes_sent() const { return _bytes_sent; }
    uint64_t get_bytes_acked() const { return _bytes_acked; }

    inline uint64_t get_flowsize() { return _flow_size; }
    inline int get_flow_src() { return _flow_src; }
    inline int get_flow_dst() { return _flow_dst; }
    inline void set_start_time(simtime_picosec startTime) { _start_time = startTime; }
    inline simtime_picosec get_start_time() { return _start_time; }
    inline bool is_finished() { return _finished; }
    inline simtime_picosec get_finish_time() { return _finish_time; }

    static void pause_all_flows();
    static void resume_all_flows();
    static bool flows_paused() { return _flows_paused; }

    void resume_flow();

    void update_route(const Route* routeout, const Route* routein);

    static std::list<RoCEv2Src*>& live_flows() { return _live_flows; }

    RoCEv2Sink* get_sink() const { return _sink; }

private:
    static bool _flows_paused;
    static std::list<RoCEv2Src*> _live_flows;
    bool _paused_send_pending;

    void send_packets();
    void handle_credit_update(uint32_t new_credits);
    void finish_flow();

    RoCEv2Sink* _sink;
    string _nodename;

    int _flow_src;
    int _flow_dst;
    uint32_t _flow_id;
    static uint32_t _next_flow_id;

    uint64_t _flow_size;         
    uint64_t _bytes_sent;            
    uint64_t _bytes_acked;            
    bool _finished;
    bool _send_pending;
    simtime_picosec _finish_time;

    uint32_t _available_credits;      
    uint32_t _packet_size;          
    uint32_t _initial_credits;        

    const Route* _routeout;
    const Route* _routeback;

    simtime_picosec _start_time;
    simtime_picosec _last_send_time;

    TrafficLogger* _pktlogger;
    void (*_application_callback)(void*);
    void* _application_callback_data;
};

class RoCEv2Sink : public PacketSink, public EventSource, public PacketFlow {
    friend class RoCEv2Src;

public:
    RoCEv2Sink(TrafficLogger* pktlogger,
               EventList& eventlist);
    ~RoCEv2Sink();

    virtual void receivePacket(Packet& pkt);
    virtual const string& nodename() { return _nodename; }
    virtual void doNextEvent();

    uint64_t get_bytes_received() const { return _bytes_received; }
    void set_src(RoCEv2Src* src) { _src = src; }

private:

    void send_credit_update();

    RoCEv2Src* _src;
    string _nodename;

    uint64_t _bytes_received;
    uint64_t _cumulative_ack;

    uint32_t _bytes_per_credit_unit;

    TrafficLogger* _pktlogger;
};

class RoCEv2Packet : public Packet {
public:
    typedef enum { ROCEV2_DATA, ROCEV2_CREDIT } rocev2_type;

    RoCEv2Packet() : Packet() {
        _type = IP;
        _rocev2_type = ROCEV2_DATA;
        _psn = 0;
        _credits = 0;
        _ackno = 0;
    }

    virtual ~RoCEv2Packet() {}
    virtual void free() { Packet::free(); }

    rocev2_type get_rocev2_type() const { return _rocev2_type; }
    void set_rocev2_type(rocev2_type t) { _rocev2_type = t; _type = (t == ROCEV2_CREDIT) ? ROCECREDIT : IP; }

    uint32_t get_psn() const { return _psn; }
    void set_psn(uint32_t psn) { _psn = psn; }

    uint32_t get_credits() const { return _credits; }
    void set_credits(uint32_t cred) { _credits = cred; }

    uint64_t get_ackno() const { return _ackno; }
    void set_ackno(uint64_t ackno) { _ackno = ackno; }

private:
    rocev2_type _rocev2_type;  
    uint32_t _psn;             
    uint32_t _credits;         
    uint64_t _ackno;
};

#endif
