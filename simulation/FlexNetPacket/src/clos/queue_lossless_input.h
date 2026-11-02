// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-        
#ifndef _LOSSLESS_INPUT_QUEUE_H
#define _LOSSLESS_INPUT_QUEUE_H
#include "queue.h"
/*
 * A FIFO queue that supports PAUSE frames and lossless operation
 */

#include <list>
#include <vector>
#include "config.h"
#include "eventlist.h"
#include "network.h"
#include "loggertypes.h"
#include "eth_pause_packet.h"

class Switch;

class LosslessInputQueue : public Queue, public VirtualQueue {
 public:
    LosslessInputQueue(EventList &eventlist);
    LosslessInputQueue(EventList &eventlist,Queue* peer);

    void receivePacket(Packet& pkt);

    void sendPause(unsigned int wait);
    void completedService(Packet& pkt);

    virtual void setName(const string& name) {
	Logged::setName(name); 
	_nodename += name;
    }
    virtual string& nodename() { return _nodename; }

    enum {PAUSED,READY,PAUSE_RECEIVED};

    static double cfg_buffer_pkts;
    static double cfg_xoff_pkts;
    static double cfg_xon_pkts;
    static void configure(double xoff_pkts, double xon_pkts, double buffer_pkts);

    static simtime_picosec cfg_pause_delay;

    static mem_b    stat_peak_bytes;
    static uint64_t stat_overflow_pkts;
    static uint64_t stat_arrivals;
    static uint64_t stat_pause_sent;
    static uint64_t stat_resume_sent;
    static mem_b    stat_peak_overshoot_bytes;
    static uint64_t stat_peak_overshoot_link_bps;
    static uint64_t stat_overflow_scaled_pkts;
    static void print_stats(std::ostream &os);

    static int cfg_classes;
    static uint64_t stat_class_saturated;
    static int pkt_class(const Packet &pkt) { return (int)((pkt.flags() >> 16) & 0xF); }
    static void set_pkt_class(Packet &pkt, int c) {
	pkt.set_flags((pkt.flags() & ~(0xFu << 16)) | ((uint32_t)c << 16));
    }
    static int pkt_in_class(const Packet &pkt) { return (int)((pkt.flags() >> 20) & 0xF); }
    static void set_pkt_in_class(Packet &pkt, int c) {
	pkt.set_flags((pkt.flags() & ~(0xFu << 20)) | ((uint32_t)c << 20));
    }

 private:
    std::vector<mem_b> _class_bytes;
    std::vector<int>   _class_state;
    void sendPause(unsigned int wait, int pfc_class);
    int _state_recv;

    int _low_threshold;
    int _high_threshold;
};

#endif
