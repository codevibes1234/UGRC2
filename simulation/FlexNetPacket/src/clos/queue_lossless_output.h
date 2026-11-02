// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-        
#ifndef _LOSSLESS_OUTPUT_QUEUE_H
#define _LOSSLESS_OUTPUT_QUEUE_H
/*
 * A FIFO queue that supports PAUSE frames and lossless operation
 */

#include <list>
#include <vector>
#include "queue.h"
#include "config.h"
#include "eventlist.h"
#include "network.h"
#include "loggertypes.h"
#include "eth_pause_packet.h"
#include "ecn.h"

class LosslessOutputQueue : public Queue {
 public:
    LosslessOutputQueue(linkspeed_bps bitrate, mem_b maxsize, EventList &eventlist, QueueLogger* logger, int ECN=0, int K=0);

    void receivePacket(Packet& pkt);
    void receivePacket(Packet& pkt,VirtualQueue* q);

    void beginService();
    void completeService();

    virtual bool pfc_ready() {
	if (_nclasses > 1) return !_sending && pick_class(false) >= 0;
	return _state_send == READY && !_sending;
    }
    virtual bool honours_circuit_gate() { return true; }

    enum {PAUSED,READY,PAUSE_RECEIVED};

    static mem_b    stat_peak_bytes;
    static mem_b    stat_peak_transit_bytes;
    static uint64_t stat_overflow_local;
    static uint64_t stat_overflow_transit;
    static uint64_t stat_overflow_transit_only;
    static void print_stats(std::ostream &os);

 private:
    int _nclasses = 1;
    std::vector<std::list<Packet*>> _class_q;
    std::vector<std::list<VirtualQueue*>> _class_vq;
    std::vector<int> _class_state;
    int _serving_class = -1;
    int _rr = 0;
    int pick_class(bool advance);
    mem_b _local_bytes = 0;
    list<VirtualQueue*> _vq;

    int _state_send;
    int _sending;

    int _ecn_enabled;
    int _K;
};

#endif
