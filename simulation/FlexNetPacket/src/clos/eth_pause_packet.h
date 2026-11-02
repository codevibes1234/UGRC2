// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#ifndef ETHPACKET_H
#define ETHPACKET_H

#include <list>
#include <bitset>
#include "network.h"

// ETHPAUSE is a subclass of Packet
// They incorporate a packet database, to reuse packet objects that are no longer needed.
// Note: you never construct a new EthPause packet directly; 
// rather you use the static method newpkt() which knows to reuse old packets from the database.

#define PAUSESIZE 64

class EthPausePacket : public Packet {
 public:

    inline static EthPausePacket* newpkt(unsigned int sleep, unsigned int pfc_class = 0){
	EthPausePacket* p = _packetdb.allocPacket();
	p->_type = ETH_PAUSE;
	p->_sleepTime = sleep;
	p->_pfcClass = pfc_class;
	p->_size = PAUSESIZE;
	return p;
    }
  
    void free() {_packetdb.freePacket(this);}
    virtual ~EthPausePacket(){}

    inline unsigned int sleepTime() const {return _sleepTime;}
    inline unsigned int pfcClass() const {return _pfcClass;}

 protected:
    unsigned int _sleepTime;
    unsigned int _pfcClass;
    static PacketDB<EthPausePacket> _packetdb;
};

#endif
