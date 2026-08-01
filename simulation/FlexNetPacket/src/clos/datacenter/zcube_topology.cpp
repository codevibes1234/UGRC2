// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#include "zcube_topology.h"
#include <vector>
#include "string.h"
#include <sstream>
#include <strstream>
#include <iostream>
#include <fstream>
#include "main.h"
#include "queue.h"
#include "switch.h"
#include "compositequeue.h"
#include "prioqueue.h"
#include "queue_lossless.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
#include "ecnqueue.h"

extern uint32_t RTT_rack;
extern uint32_t RTT_net;
extern uint32_t SPEED;
extern std::ofstream fct_util_out;

string ntoa(double n);
string itoa(uint64_t n);

ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
                             EventList* ev, FirstFit* f, queue_type q)
    : Topology()
{
    logfile = log;
    eventlist = ev;
    ff = f;
    qt = q;
    failed_links = 0;
    link_speed = 0;
    _queuesize = queuesize;
    set_params(no_of_nodes);
    init_network();
    load_fc_schedule();
}

ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
                             EventList* ev, FirstFit* f, queue_type q, int fail)
    : Topology()
{
    logfile = log;
    eventlist = ev;
    ff = f;
    qt = q;
    failed_links = fail;
    link_speed = 0;
    _queuesize = queuesize;
    set_params(no_of_nodes);
    init_network();
    load_fc_schedule();
}

ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
                             EventList* ev, FirstFit* f, queue_type q, uint32_t speed)
    : Topology()
{
    logfile = log;
    eventlist = ev;
    ff = f;
    qt = q;
    failed_links = 0;
    link_speed = speed;
    _queuesize = queuesize;
    set_params(no_of_nodes);
    init_network();
    load_fc_schedule();
}

void ZCubeTopology::set_params(int no_of_nodes)
{
    n = 1;
    while(n*n < no_of_nodes) n++;
    NSRV = n*n;
    NL0 = n;
    NL1 = n;
    _no_of_nodes = NSRV;

    if (NSRV > no_of_nodes) {
        std::cerr << "Warning: ZCube(" << n << ",2) has " << NSRV
                  << " GPUs. Requested " << no_of_nodes << " (using next power-of-n)\n";
    }

    std::cout << "Building ZCube(n=" << n << ", k=2) with " << NSRV
              << " GPUs (" << NL0 << " L0 + " << NL1 << " L1 switches)\n";

    // GPU <-> L0
    queues_ns_l0.resize(NSRV, std::vector<Queue*>(NL0, nullptr));
    queues_l0_ns.resize(NL0,  std::vector<Queue*>(NSRV, nullptr));
    pipes_ns_l0.resize(NSRV,  std::vector<Pipe*>(NL0, nullptr));
    pipes_l0_ns.resize(NL0,   std::vector<Pipe*>(NSRV, nullptr));

    // GPU <-> L1
    queues_ns_l1.resize(NSRV, std::vector<Queue*>(NL1, nullptr));
    queues_l1_ns.resize(NL1,  std::vector<Queue*>(NSRV, nullptr));
    pipes_ns_l1.resize(NSRV,  std::vector<Pipe*>(NL1, nullptr));
    pipes_l1_ns.resize(NL1,   std::vector<Pipe*>(NSRV, nullptr));

    // L0 <-> L1
    queues_l0_l1.resize(NL0, std::vector<Queue*>(NL1, nullptr));
    queues_l1_l0.resize(NL1, std::vector<Queue*>(NL0, nullptr));
    pipes_l0_l1.resize(NL0,  std::vector<Pipe*>(NL1, nullptr));
    pipes_l1_l0.resize(NL1,  std::vector<Pipe*>(NL0, nullptr));

    switches_l0.resize(NL0, nullptr);
    switches_l1.resize(NL1, nullptr);
}

Queue *ZCubeTopology::alloc_src_queue(QueueLogger *queueLogger)
{
    if(link_speed == 0) {
        return new PriorityQueue(speedFromMbps((uint64_t)SPEED), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
    }
    else {
        return new PriorityQueue(speedFromMbps((uint64_t)link_speed), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
    }
}

Queue *ZCubeTopology::alloc_src_queue(QueueLogger *queueLogger, uint64_t speed)
{
    return new PriorityQueue(speedFromMbps(speed), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
}

Queue *ZCubeTopology::alloc_queue(QueueLogger *queueLogger, mem_b queuesize)
{
    if(link_speed == 0) {
        return alloc_queue(queueLogger, SPEED, queuesize);
    }
    else {
        return alloc_queue(queueLogger, link_speed, queuesize);
    }
}

Queue *ZCubeTopology::alloc_queue(QueueLogger *queueLogger, uint64_t speed, mem_b queuesize)
{
    if (qt == RANDOM)
        return new RandomQueue(speedFromMbps(speed), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
    else if (qt == COMPOSITE)
        return new CompositeQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
    else if (qt == CTRL_PRIO)
        return new CtrlPrioQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
    else if (qt == ECN)
        return new ECNQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger, memFromPkt(50));
    else if (qt == LOSSLESS)
        return new LosslessQueue(speedFromMbps(speed), memFromPkt(50), *eventlist, queueLogger, NULL);
    else if (qt == LOSSLESS_INPUT)
        return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(200), *eventlist, queueLogger);
    else if (qt == LOSSLESS_INPUT_ECN)
        return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(10000), *eventlist, queueLogger, 1, memFromPkt(16));
    assert(0);
}


void ZCubeTopology::init_network()
{
    QueueLoggerSampling *queueLogger = nullptr;

    for (int g = 0; g < NSRV; g++){
        for (int l0 = 0; l0 < NL0; l0++)
        {
            queues_ns_l0[g][l0] = NULL;
            pipes_ns_l0[g][l0] = NULL;
            queues_l0_ns[l0][g] = NULL;
            pipes_l0_ns[l0][g] = NULL;
        }

        for(int l1 = 0; l1 < NL1; l1++)
        {
            queues_ns_l1[g][l1] = NULL;
            pipes_ns_l1[g][l1] = NULL;
            queues_l1_ns[l1][g] = NULL;
            pipes_l1_ns[l1][g] = NULL;
        }
    }

    for (int l0 = 0; l0 < NL0; l0++) {
        for (int l1 = 0; l1 < NL1; l1++)
        {
            queues_l0_l1[l0][l1] = NULL;
            pipes_l0_l1[l0][l1] = NULL;
            queues_l1_l0[l1][l0] = NULL;
            pipes_l1_l0[l1][l0] = NULL;
        }
    }

    // create switches if we have lossless operation
    if (qt == LOSSLESS)
        for (int j = 0; j < NL0; j++)
        {
            switches_l0[j] = new Switch("Switch_L0_" + ntoa(j));
            switches_l1[j] = new Switch("Switch_L1_" + ntoa(j));
        }

    // -------------------------------------------------------------------------
    // Link 1: GPU <-> L0 (NIC0)
    // GPU g connects to L0[g/n] via NIC0.
    // Each GPU has 2 NIC ports so each port runs at SPEED/2.
    // This matches the paper: a 400G NIC split into 2x200G ports.
    // -------------------------------------------------------------------------
    for (int g = 0; g < NSRV; g++)
    {
        int l0 = g / n;

        // Downlink: L0 -> GPU  (SPEED/2 = one NIC port)
        queues_l0_ns[l0][g] = alloc_queue(queueLogger, SPEED/2, _queuesize);
        queues_l0_ns[l0][g]->setName("L0_"+ntoa(l0)+"->GPU"+ntoa(g));

        pipes_l0_ns[l0][g] = new Pipe(timeFromNs(RTT_rack), *eventlist);
        pipes_l0_ns[l0][g]->setName("Pipe-L0_"+ntoa(l0)+"->GPU"+ntoa(g));
        pipes_l0_ns[l0][g]->set_pipe_downlink();

        // Uplink: GPU -> L0  (SPEED/2 = one NIC port)
        queues_ns_l0[g][l0] = alloc_src_queue(queueLogger, SPEED/2);
        queues_ns_l0[g][l0]->setName("GPU"+ntoa(g)+"->L0_"+ntoa(l0));

        pipes_ns_l0[g][l0] = new Pipe(timeFromNs(RTT_rack), *eventlist);
        pipes_ns_l0[g][l0]->setName("Pipe-GPU"+ntoa(g)+"->L0_"+ntoa(l0));

        if(qt == LOSSLESS) {
            switches_l0[l0]->addPort(queues_l0_ns[l0][g]);
            ((LosslessQueue*)queues_l0_ns[l0][g])->setRemoteEndpoint(queues_ns_l0[g][l0]);
        } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
            new LosslessInputQueue(*eventlist, queues_ns_l0[g][l0]);
        }

        if(ff) {
            ff->add_queue(queues_l0_ns[l0][g]);
            ff->add_queue(queues_ns_l0[g][l0]);
        }
    }

    // -------------------------------------------------------------------------
    // Link 2: GPU <-> L1 (NIC1)
    // GPU g connects to L1[g%n] via NIC1.
    // Same reasoning: SPEED/2 per NIC port.
    // -------------------------------------------------------------------------
    for (int g = 0; g < NSRV; g++)
    {
        int l1 = g % n;

        // Downlink: L1 -> GPU  (SPEED/2)
        queues_l1_ns[l1][g] = alloc_queue(queueLogger, SPEED/2, _queuesize);
        queues_l1_ns[l1][g]->setName("L1_"+ntoa(l1)+"->GPU"+ntoa(g));

        pipes_l1_ns[l1][g] = new Pipe(timeFromNs(RTT_net), *eventlist);
        pipes_l1_ns[l1][g]->setName("Pipe-L1_"+ntoa(l1)+"->GPU"+ntoa(g));
        pipes_l1_ns[l1][g]->set_pipe_downlink();

        // Uplink: GPU -> L1  (SPEED/2)
        queues_ns_l1[g][l1] = alloc_src_queue(queueLogger, SPEED/2);
        queues_ns_l1[g][l1]->setName("GPU"+ntoa(g)+"->L1_"+ntoa(l1));

        pipes_ns_l1[g][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);
        pipes_ns_l1[g][l1]->setName("Pipe-GPU"+ntoa(g)+"->L1_"+ntoa(l1));

        if(qt == LOSSLESS) {
            switches_l1[l1]->addPort(queues_l1_ns[l1][g]);
            ((LosslessQueue*)queues_l1_ns[l1][g])->setRemoteEndpoint(queues_ns_l1[g][l1]);
        } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
            new LosslessInputQueue(*eventlist, queues_ns_l1[g][l1]);
        }

        if(ff) {
            ff->add_queue(queues_l1_ns[l1][g]);
            ff->add_queue(queues_ns_l1[g][l1]);
        }
    }

    // -------------------------------------------------------------------------
    // Link 3: L0 <-> L1 (inter-switch backbone)
    // Switch ports are independent and each runs at full SPEED.
    // The paper (Fig 24, Table 6) shows ALL links at the same 200G rate —
    // there is no halving at the switch level. The SPEED/2 only applies to
    // GPU NIC ports because each GPU splits one logical NIC into two ports.
    // Switches have no such constraint: every port runs at full line rate.
    // -------------------------------------------------------------------------
    for (int l0 = 0; l0 < NL0; l0++)
    {
        for(int l1 = 0; l1 < NL1; l1++)
        {
            // L0 -> L1  (full SPEED — switch port, not a NIC)
            queues_l0_l1[l0][l1] = alloc_queue(queueLogger, SPEED, _queuesize);
            queues_l0_l1[l0][l1]->setName("L0_"+ntoa(l0)+"->L1_"+ntoa(l1));

            pipes_l0_l1[l0][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);
            pipes_l0_l1[l0][l1]->setName("Pipe-L0_"+ntoa(l0)+"->L1_"+ntoa(l1));

            // L1 -> L0  (full SPEED — switch port, not a NIC)
            queues_l1_l0[l1][l0] = alloc_queue(queueLogger, SPEED, _queuesize);
            queues_l1_l0[l1][l0]->setName("L1_"+ntoa(l1)+"->L0_"+ntoa(l0));

            pipes_l1_l0[l1][l0] = new Pipe(timeFromNs(RTT_net), *eventlist);
            pipes_l1_l0[l1][l0]->setName("Pipe-L1_"+ntoa(l1)+"->L0_"+ntoa(l0));

            if(qt == LOSSLESS) {
                switches_l0[l0]->addPort(queues_l0_l1[l0][l1]);
                switches_l1[l1]->addPort(queues_l1_l0[l1][l0]);
                ((LosslessQueue*)queues_l0_l1[l0][l1])->setRemoteEndpoint(queues_l1_l0[l1][l0]);
                ((LosslessQueue*)queues_l1_l0[l1][l0])->setRemoteEndpoint(queues_l0_l1[l0][l1]);
            } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
                new LosslessInputQueue(*eventlist, queues_l0_l1[l0][l1]);
                new LosslessInputQueue(*eventlist, queues_l1_l0[l1][l0]);
            }

            if(ff) {
                ff->add_queue(queues_l0_l1[l0][l1]);
                ff->add_queue(queues_l1_l0[l1][l0]);
            }
        }
    }

    if(qt == LOSSLESS)
    {
        for(int j = 0; j < NL0; j++)
        {
            switches_l0[j]->configureLossless();
            switches_l1[j]->configureLossless();
        }
    }
}

void check_non_null(Route *rt)
{
    int fail = 0;
    for (unsigned int i = 1; i < rt->size() - 1; i += 2)
        if (rt->at(i) == NULL)
        {
            fail = 1;
            break;
        }

    if (fail)
    {
        for (unsigned int i = 1; i < rt->size() - 1; i += 2)
            printf("%p ", rt->at(i));
        cout << endl;
        assert(0);
    }
}


vector<const Route*>* ZCubeTopology::get_paths(int src, int dest)
{
    vector<const Route*>* paths = new vector<const Route*>();
    route_t *routeout, *routeback;

    int src_l0 = src / n;  // src's L0 switch
    int dst_l0 = dest / n; // dst's L0 switch
    int src_l1 = src % n;  // src's L1 switch
    int dst_l1 = dest % n; // dst's L1 switch

    // -------------------------------------------------------------------------
    // Case 1: same L0 group
    // src and dst share the same L0 switch.
    // Path: src (NIC0) -> L0 -> dst  (2 hops)
    // -------------------------------------------------------------------------
    if (src_l0 == dst_l0)
    {
        routeout = new Route();
        routeout->push_back(queues_ns_l0[src][src_l0]);
        routeout->push_back(pipes_ns_l0[src][src_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_ns_l0[src][src_l0]->getRemoteEndpoint());
        routeout->push_back(queues_l0_ns[dst_l0][dest]);
        routeout->push_back(pipes_l0_ns[dst_l0][dest]);

        routeback = new Route();
        routeback->push_back(queues_ns_l0[dest][dst_l0]);
        routeback->push_back(pipes_ns_l0[dest][dst_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_ns_l0[dest][dst_l0]->getRemoteEndpoint());
        routeback->push_back(queues_l0_ns[src_l0][src]);
        routeback->push_back(pipes_l0_ns[src_l0][src]);

        routeout->set_reverse(routeback);
        routeback->set_reverse(routeout);
        paths->push_back(routeout);
        check_non_null(routeout);
        return paths;
    }

    // -------------------------------------------------------------------------
    // Case 2: same L1 index
    // src and dst share the same L1 switch.
    // Path: src (NIC1) -> L1 -> dst  (2 hops)
    // -------------------------------------------------------------------------
    else if (src_l1 == dst_l1)
    {
        routeout = new Route();
        routeout->push_back(queues_ns_l1[src][src_l1]);
        routeout->push_back(pipes_ns_l1[src][src_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_ns_l1[src][src_l1]->getRemoteEndpoint());
        routeout->push_back(queues_l1_ns[dst_l1][dest]);
        routeout->push_back(pipes_l1_ns[dst_l1][dest]);

        routeback = new Route();
        routeback->push_back(queues_ns_l1[dest][dst_l1]);
        routeback->push_back(pipes_ns_l1[dest][dst_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_ns_l1[dest][dst_l1]->getRemoteEndpoint());
        routeback->push_back(queues_l1_ns[src_l1][src]);
        routeback->push_back(pipes_l1_ns[src_l1][src]);

        routeout->set_reverse(routeback);
        routeback->set_reverse(routeout);
        paths->push_back(routeout);
        check_non_null(routeout);
        return paths;
    }

    // -------------------------------------------------------------------------
    // Case 3: different L0 group AND different L1 index
    // Two ECMP paths available:
    //   Path A: src -> NIC0 -> L0[src] -> L1[dst] -> dst
    //   Path B: src -> NIC1 -> L1[src] -> L0[dst] -> dst
    // -------------------------------------------------------------------------
    else
    {
        // Path A: via src's L0 then dst's L1
        routeout = new Route();
        routeout->push_back(queues_ns_l0[src][src_l0]);
        routeout->push_back(pipes_ns_l0[src][src_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_ns_l0[src][src_l0]->getRemoteEndpoint());
        routeout->push_back(queues_l0_l1[src_l0][dst_l1]);
        routeout->push_back(pipes_l0_l1[src_l0][dst_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_l0_l1[src_l0][dst_l1]->getRemoteEndpoint());
        routeout->push_back(queues_l1_ns[dst_l1][dest]);
        routeout->push_back(pipes_l1_ns[dst_l1][dest]);

        routeback = new Route();
        routeback->push_back(queues_ns_l1[dest][dst_l1]);
        routeback->push_back(pipes_ns_l1[dest][dst_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_ns_l1[dest][dst_l1]->getRemoteEndpoint());
        routeback->push_back(queues_l1_l0[dst_l1][src_l0]);
        routeback->push_back(pipes_l1_l0[dst_l1][src_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_l1_l0[dst_l1][src_l0]->getRemoteEndpoint());
        routeback->push_back(queues_l0_ns[src_l0][src]);
        routeback->push_back(pipes_l0_ns[src_l0][src]);

        routeout->set_reverse(routeback);
        routeback->set_reverse(routeout);
        paths->push_back(routeout);
        check_non_null(routeout);

        // Path B: via src's L1 then dst's L0
        routeout = new Route();
        routeout->push_back(queues_ns_l1[src][src_l1]);
        routeout->push_back(pipes_ns_l1[src][src_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_ns_l1[src][src_l1]->getRemoteEndpoint());
        routeout->push_back(queues_l1_l0[src_l1][dst_l0]);
        routeout->push_back(pipes_l1_l0[src_l1][dst_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeout->push_back(queues_l1_l0[src_l1][dst_l0]->getRemoteEndpoint());
        routeout->push_back(queues_l0_ns[dst_l0][dest]);
        routeout->push_back(pipes_l0_ns[dst_l0][dest]);

        routeback = new Route();
        routeback->push_back(queues_ns_l0[dest][dst_l0]);
        routeback->push_back(pipes_ns_l0[dest][dst_l0]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_ns_l0[dest][dst_l0]->getRemoteEndpoint());
        routeback->push_back(queues_l0_l1[dst_l0][src_l1]);
        routeback->push_back(pipes_l0_l1[dst_l0][src_l1]);
        if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
            routeback->push_back(queues_l0_l1[dst_l0][src_l1]->getRemoteEndpoint());
        routeback->push_back(queues_l1_ns[src_l1][src]);
        routeback->push_back(pipes_l1_ns[src_l1][src]);

        routeout->set_reverse(routeback);
        routeback->set_reverse(routeout);
        paths->push_back(routeout);
        check_non_null(routeout);

        return paths;
    }
}

void ZCubeTopology::count_queue(Queue* queue)
{
    if (_link_usage.find(queue) == _link_usage.end())
        _link_usage[queue] = 0;
    _link_usage[queue]++;
}

int ZCubeTopology::find_l0_switch(Queue* queue)
{
    for (int l0 = 0; l0 < NL0; l0++)
        for (int g = 0; g < NSRV; g++)
            if (queues_l0_ns[l0][g] == queue) return l0;
    return -1;
}

int ZCubeTopology::find_l1_switch(Queue* queue)
{
    for (int l1 = 0; l1 < NL1; l1++)
        for (int g = 0; g < NSRV; g++)
            if (queues_l1_ns[l1][g] == queue) return l1;
    return -1;
}

int ZCubeTopology::find_destination(Queue* queue)
{
    for (int g = 0; g < NSRV; g++) {
        for (int l0 = 0; l0 < NL0; l0++)
            if (queues_l0_ns[l0][g] == queue) return g;
        for (int l1 = 0; l1 < NL1; l1++)
            if (queues_l1_ns[l1][g] == queue) return g;
    }
    return -1;
}

void ZCubeTopology::print_path(std::ofstream& paths, int src, const Route* route)
{
    paths << "SRC_" << src << " ";
    paths << "ZCube(" << n << ",2) path (" << (route->size()/2) << " hops) ";

    int l0 = find_l0_switch((Queue*)route->at(1));
    int l1 = find_l1_switch((Queue*)route->at(1));
    if (l0 >= 0) paths << "L0_" << l0 << " ";
    if (l1 >= 0) paths << "L1_" << l1 << " ";

    paths << "DST_" << find_destination((Queue*)route->at(route->size() - 1)) << " ";
    paths << endl;
}
// // -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// #include "zcube_topology.h"
// #include <vector>
// #include "string.h"
// #include <sstream>
// #include <strstream>
// #include <iostream>
// #include <fstream>
// #include "main.h"
// #include "queue.h"
// #include "switch.h"
// #include "compositequeue.h"
// #include "prioqueue.h"
// #include "queue_lossless.h"
// #include "queue_lossless_input.h"
// #include "queue_lossless_output.h"
// #include "ecnqueue.h"

// extern uint32_t RTT_rack;
// extern uint32_t RTT_net;
// extern uint32_t SPEED;
// extern std::ofstream fct_util_out;

// string ntoa(double n);
// string itoa(uint64_t n);

// //extern int N;

// /*
// FatTreeTopology::FatTreeTopology(int no_of_nodes, mem_b queuesize, Logfile *lg,
//                                  EventList *ev, FirstFit *fit, queue_type q)
// {
//   _queuesize = queuesize;
//   logfile = lg;
//   eventlist = ev;
//   ff = fit;
//   qt = q;
//   failed_links = 0;

//   set_params(no_of_nodes);

//   init_network();
// }

// FatTreeTopology::FatTreeTopology(int no_of_nodes, mem_b queuesize, Logfile *lg,
//                                  EventList *ev, FirstFit *fit, queue_type q, uint32_t speed)
// {
//   _queuesize = queuesize;
//   logfile = lg;
//   eventlist = ev;
//   ff = fit;
//   qt = q;
//   failed_links = 0;
//   link_speed = speed;

//   set_params(no_of_nodes);

//   init_network();
// }

// FatTreeTopology::FatTreeTopology(int no_of_nodes, mem_b queuesize, Logfile *lg,
//                                  EventList *ev, FirstFit *fit, queue_type q, int fail)
// {
//   _queuesize = queuesize;
//   logfile = lg;
//   qt = q;

//   eventlist = ev;
//   ff = fit;

//   failed_links = fail;

//   set_params(no_of_nodes);

//   init_network();
// }
// */

// ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
//                              EventList* ev, FirstFit* f, queue_type q)
//     : Topology()  // base class
// {
//     logfile = log;
//     eventlist = ev;
//     ff = f;
//     qt = q;
//     failed_links = 0;
//     link_speed = 0;
//     _queuesize = queuesize;
//     set_params(no_of_nodes);
//     init_network();
// }

// ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
//                              EventList* ev, FirstFit* f, queue_type q, int fail)
//     : Topology()
// {
//     logfile = log;
//     eventlist = ev;
//     ff = f;
//     qt = q;
//     failed_links = fail;
//     link_speed = 0;
//     _queuesize = queuesize;
//     set_params(no_of_nodes);
//     init_network();
// }

// ZCubeTopology::ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
//                              EventList* ev, FirstFit* f, queue_type q, uint32_t speed)
//     : Topology()
// {
//     logfile = log;
//     eventlist = ev;
//     ff = f;
//     qt = q;
//     failed_links = 0;
//     link_speed = speed;
//     _queuesize = queuesize;
//     set_params(no_of_nodes);
//     init_network();
// }

// void ZCubeTopology::set_params(int no_of_nodes)
// {
//   n = 1;
//   while(n*n < no_of_nodes) n++;
//   NSRV = n*n;
//   NL0 = n;
//   NL1 = n;
//   _no_of_nodes = NSRV;

//   if (NSRV > no_of_nodes) {
//         std::cerr << "Warning: ZCube(" << n << ",2) has " << NSRV 
//                   << " GPUs. Requested " << no_of_nodes << " (using next power-of-n)\n";
//     }

//     std::cout << "Building ZCube(n=" << n << ", k=2) with " << NSRV 
//               << " GPUs (" << NL0 << " L0 + " << NL1 << " L1 switches)\n";

//     // GPU <-> L0
//     queues_ns_l0.resize(NSRV, std::vector<Queue*>(NL0, nullptr));
//     queues_l0_ns.resize(NL0,  std::vector<Queue*>(NSRV, nullptr));
//     pipes_ns_l0.resize(NSRV,  std::vector<Pipe*>(NL0, nullptr));
//     pipes_l0_ns.resize(NL0,   std::vector<Pipe*>(NSRV, nullptr));

//     // GPU <-> L1
//     queues_ns_l1.resize(NSRV, std::vector<Queue*>(NL1, nullptr));
//     queues_l1_ns.resize(NL1,  std::vector<Queue*>(NSRV, nullptr));
//     pipes_ns_l1.resize(NSRV,  std::vector<Pipe*>(NL1, nullptr));
//     pipes_l1_ns.resize(NL1,   std::vector<Pipe*>(NSRV, nullptr));

//     // L0 <-> L1
//     queues_l0_l1.resize(NL0, std::vector<Queue*>(NL1, nullptr));
//     queues_l1_l0.resize(NL1, std::vector<Queue*>(NL0, nullptr));
//     pipes_l0_l1.resize(NL0,  std::vector<Pipe*>(NL1, nullptr));
//     pipes_l1_l0.resize(NL1,  std::vector<Pipe*>(NL0, nullptr));

//     switches_l0.resize(NL0, nullptr);
//     switches_l1.resize(NL1, nullptr);
// }

// /*
// Queue *FatTreeTopology::alloc_src_queue(QueueLogger *queueLogger)
// {
//   if(link_speed == 0) {
//     return new PriorityQueue(speedFromMbps((uint64_t)SPEED), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
//   }
//   else {
//     return new PriorityQueue(speedFromMbps((uint64_t)link_speed), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
//   }
// }

// Queue *FatTreeTopology::alloc_queue(QueueLogger *queueLogger, mem_b queuesize)
// {
//   if(link_speed == 0) {
//     return alloc_queue(queueLogger, SPEED, queuesize);
//   }
//   else {
//     return alloc_queue(queueLogger, link_speed, queuesize);
//   }
// }

// Queue *FatTreeTopology::alloc_queue(QueueLogger *queueLogger, uint64_t speed, mem_b queuesize)
// {
//   if (qt == RANDOM)
//     return new RandomQueue(speedFromMbps(speed), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
//   else if (qt == COMPOSITE)
//     return new CompositeQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
//   else if (qt == CTRL_PRIO)
//     return new CtrlPrioQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
//   else if (qt == ECN)
//     return new ECNQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger, memFromPkt(50));
//   else if (qt == LOSSLESS)
//     return new LosslessQueue(speedFromMbps(speed), memFromPkt(50), *eventlist, queueLogger, NULL);
//   else if (qt == LOSSLESS_INPUT)
//     return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(200), *eventlist, queueLogger);
//   else if (qt == LOSSLESS_INPUT_ECN)
//     return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(10000), *eventlist, queueLogger, 1, memFromPkt(16));
//   assert(0);
// }
// */

// Queue *ZCubeTopology::alloc_src_queue(QueueLogger *queueLogger)
// {
//     if(link_speed == 0) {
//         return new PriorityQueue(speedFromMbps((uint64_t)SPEED), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
//     }
//     else {
//         return new PriorityQueue(speedFromMbps((uint64_t)link_speed), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
//     }
// }
// Queue *ZCubeTopology::alloc_src_queue(QueueLogger *queueLogger, uint64_t speed)
// {
//     return new PriorityQueue(speedFromMbps(speed), memFromPkt(FEEDER_BUFFER), *eventlist, queueLogger);
// }

// Queue *ZCubeTopology::alloc_queue(QueueLogger *queueLogger, mem_b queuesize)
// {
//     if(link_speed == 0) {
//         return alloc_queue(queueLogger, SPEED, queuesize);
//     }
//     else {
//         return alloc_queue(queueLogger, link_speed, queuesize);
//     }
// }

// Queue *ZCubeTopology::alloc_queue(QueueLogger *queueLogger, uint64_t speed, mem_b queuesize)
// {
//     if (qt == RANDOM)
//         return new RandomQueue(speedFromMbps(speed), memFromPkt(SWITCH_BUFFER + RANDOM_BUFFER), *eventlist, queueLogger, memFromPkt(RANDOM_BUFFER));
//     else if (qt == COMPOSITE)
//         return new CompositeQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
//     else if (qt == CTRL_PRIO)
//         return new CtrlPrioQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger);
//     else if (qt == ECN)
//         return new ECNQueue(speedFromMbps(speed), queuesize, *eventlist, queueLogger, memFromPkt(50));
//     else if (qt == LOSSLESS)
//         return new LosslessQueue(speedFromMbps(speed), memFromPkt(50), *eventlist, queueLogger, NULL);
//     else if (qt == LOSSLESS_INPUT)
//         return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(200), *eventlist, queueLogger);
//     else if (qt == LOSSLESS_INPUT_ECN)
//         return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(10000), *eventlist, queueLogger, 1, memFromPkt(16));
//     assert(0);
// }


// void ZCubeTopology::init_network()
// {
//   QueueLoggerSampling *queueLogger = nullptr;

//   for (int g = 0; g < NSRV; g++){
//     for (int l0 = 0; l0 < NL0; l0++)
//     {
//       queues_ns_l0[g][l0] = NULL;
//       pipes_ns_l0[g][l0] = NULL;
//       queues_l0_ns[l0][g] = NULL;
//       pipes_l0_ns[l0][g] = NULL;
//     }

//     for(int l1 = 0; l1 < NL1; l1++)
//     {
//       queues_ns_l1[g][l1] = NULL;
//       pipes_ns_l1[g][l1] = NULL;
//       queues_l1_ns[l1][g] = NULL;
//       pipes_l1_ns[l1][g] = NULL;
//     }
//   }

//   for (int l0 = 0; l0 < NL0; l0++) {
//     for (int l1 = 0; l1 < NL1; l1++)
//     {
//       queues_l0_l1[l0][l1] = NULL;
//       pipes_l0_l1[l0][l1] = NULL;
//       queues_l1_l0[l1][l0] = NULL;
//       pipes_l1_l0[l1][l0] = NULL;
//     }
//   } 


//   //create switches if we have lossless operation
//   if (qt == LOSSLESS)
//     for (int j = 0; j < NL0; j++)
//     {
//       switches_l0[j] = new Switch("Switch_L0_" + ntoa(j));
//       switches_l1[j] = new Switch("Switch_L1_" + ntoa(j));
//     }

//   // Link 1: GPU <-> L0 (NIC0)
//   // GPU g connects to L0[g/n] via NIC0
//   for (int g = 0; g < NSRV; g++)
//   {
//     int l0 = g / n;

//     queues_l0_ns[l0][g] = alloc_queue(queueLogger, SPEED/2, _queuesize);
//     queues_l0_ns[l0][g]->setName("L0_"+ntoa(l0)+"->GPU"+ntoa(g));

//     pipes_l0_ns[l0][g] = new Pipe(timeFromNs(RTT_rack), *eventlist);
//     pipes_l0_ns[l0][g]->setName("Pipe-L0_"+ntoa(l0) + "->GPU" + ntoa(g));
//     pipes_l0_ns[l0][g]->set_pipe_downlink();

//     // Uplink from GPU -> L0 (NIC0)
//     queues_ns_l0[g][l0] = alloc_src_queue(queueLogger, SPEED/2);
//     queues_ns_l0[g][l0]->setName("GPU" + ntoa(g) + "->L0_" + ntoa(l0));

//     pipes_ns_l0[g][l0] = new Pipe(timeFromNs(RTT_rack), *eventlist);
//     pipes_ns_l0[g][l0]->setName("Pipe-GPU" + ntoa(g) + "->L0_" + ntoa(l0));

//     if(qt == LOSSLESS) {
//       switches_l0[l0]->addPort(queues_l0_ns[l0][g]);
//       ((LosslessQueue*)queues_l0_ns[l0][g])->setRemoteEndpoint(queues_ns_l0[g][l0]);
//     } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
//       new LosslessInputQueue(*eventlist, queues_ns_l0[g][l0]);
//     }

//     if(ff) 
//     {
//       ff->add_queue(queues_l0_ns[l0][g]);
//       ff->add_queue(queues_ns_l0[g][l0]);
//     }
//   }

//   // Link 2: GPU <-> L1 (NIC1)
//   // GPU g connects to L1[g % n] via NIC1
//   for (int g = 0; g < NSRV; g++)
//   {
//     int l1 = g % n;

//     // Donwlink: l1 -> GPU
//     queues_l1_ns[l1][g] = alloc_queue(queueLogger, SPEED/2, _queuesize);
//     queues_l1_ns[l1][g]->setName("L1_"+ntoa(l1)+"->GPU"+ntoa(g));

//     pipes_l1_ns[l1][g] = new Pipe(timeFromNs(RTT_net), *eventlist);
//     pipes_l1_ns[l1][g]->setName("Pipe-L1_"+ntoa(l1) + "->GPU" + ntoa(g));
//     pipes_l1_ns[l1][g]->set_pipe_downlink();

//     // Uplink from GPU -> L0 (NIC0)
//     queues_ns_l1[g][l1] = alloc_src_queue(queueLogger, SPEED/2);
//     queues_ns_l1[g][l1]->setName("GPU" + ntoa(g) + "->L1_" + ntoa(l1));

//     pipes_ns_l1[g][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);
//     pipes_ns_l1[g][l1]->setName("Pipe-GPU" + ntoa(g) + "->L1_" + ntoa(l1));

//     if(qt == LOSSLESS) {
//       switches_l1[l1]->addPort(queues_l1_ns[l1][g]);
//       ((LosslessQueue*)queues_l1_ns[l1][g])->setRemoteEndpoint(queues_ns_l1[g][l1]);
//     } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
//       new LosslessInputQueue(*eventlist, queues_ns_l1[g][l1]);
//     }

//     if(ff) 
//     {
//       ff->add_queue(queues_l1_ns[l1][g]);
//       ff->add_queue(queues_ns_l1[g][l1]);
//     }
//   }

//   // Links 3: L0 <-> L1 
//   // Every L0 switch connects to every L1 switch
//   for (int l0 = 0; l0 < NL0; l0++)
//   {
//     for(int l1 = 0; l1 < NL1; l1++){

//     // L0 -> L1
//     queues_l0_l1[l0][l1] = alloc_queue(queueLogger, SPEED/2,_queuesize);
//     queues_l0_l1[l0][l1]->setName("L0_"+ntoa(l0)+"->L1_"+ntoa(l1));

//     pipes_l0_l1[l0][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);
//     pipes_l0_l1[l0][l1]->setName("Pipe-L0_"+ntoa(l0) + "->L1_" + ntoa(l1));

//     // L1 -> L0
//     queues_l1_l0[l1][l0] = alloc_queue(queueLogger, SPEED/2, _queuesize);
//     queues_l1_l0[l1][l0]->setName("L1_" + ntoa(l1) + "->L0_" + ntoa(l0));

//     pipes_l1_l0[l1][l0] = new Pipe(timeFromNs(RTT_net), *eventlist);
//     pipes_l1_l0[l1][l0]->setName("Pipe-L1_" + ntoa(l1) + "->L0_" + ntoa(l0));

//     if(qt == LOSSLESS) {
//       switches_l0[l0]->addPort(queues_l0_l1[l0][l1]);
//       switches_l1[l1]->addPort(queues_l1_l0[l1][l0]);
//       ((LosslessQueue*)queues_l0_l1[l0][l1])->setRemoteEndpoint(queues_l1_l0[l1][l0]);
//       ((LosslessQueue*)queues_l1_l0[l1][l0])->setRemoteEndpoint(queues_l0_l1[l0][l1]);
//     } else if(qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN) {
//       new LosslessInputQueue(*eventlist, queues_l0_l1[l0][l1]);
//       new LosslessInputQueue(*eventlist, queues_l1_l0[l1][l0]);
//     }

//     if(ff) 
//     {
//       ff->add_queue(queues_l0_l1[l0][l1]);
//       ff->add_queue(queues_l1_l0[l1][l0]);
//     }
//     }
//   }

//   if(qt == LOSSLESS)
//   {
//     for(int j = 0; j < NL0; j++)
//     {
//       switches_l0[j]->configureLossless();
//       switches_l1[j]->configureLossless();
//     }
//   }
// }

// void check_non_null(Route *rt)
// {
//   int fail = 0;
//   for (unsigned int i = 1; i < rt->size() - 1; i += 2)
//     if (rt->at(i) == NULL)
//     {
//       fail = 1;
//       break;
//     }

//   if (fail)
//   {
//     //    cout <<"Null queue in route"<<endl;
//     for (unsigned int i = 1; i < rt->size() - 1; i += 2)
//       printf("%p ", rt->at(i));

//     cout << endl;
//     assert(0);
//   }
// }


// vector<const Route*>* ZCubeTopology::get_paths(int src, int dest)
// {
//     vector<const Route*>* paths = new vector<const Route*>();
//     route_t *routeout, *routeback;

//     int src_l0 = src / n;  // src's L0 switch
//     int dst_l0 = dest / n; // dst's L0 switch
//     int src_l1 = src % n;  // src's L1 switch
//     int dst_l1 = dest % n; // dst's L1 switch

//     // Case 1: same L0 group
//     // src and dst share the same L0 switch
//     // Path: src (NIC0) -> L0 -> dst (2 hops)
//     if (src_l0 == dst_l0)
//     {
//         routeout = new Route();
//         routeout->push_back(queues_ns_l0[src][src_l0]);
//         routeout->push_back(pipes_ns_l0[src][src_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_ns_l0[src][src_l0]->getRemoteEndpoint());
//         routeout->push_back(queues_l0_ns[dst_l0][dest]);
//         routeout->push_back(pipes_l0_ns[dst_l0][dest]);

//         routeback = new Route();
//         routeback->push_back(queues_ns_l0[dest][dst_l0]);
//         routeback->push_back(pipes_ns_l0[dest][dst_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_ns_l0[dest][dst_l0]->getRemoteEndpoint());
//         routeback->push_back(queues_l0_ns[src_l0][src]);
//         routeback->push_back(pipes_l0_ns[src_l0][src]);

//         routeout->set_reverse(routeback);
//         routeback->set_reverse(routeout);
//         paths->push_back(routeout);
//         check_non_null(routeout);
//         return paths;
//     }

//     // -- Case 2: same L1 index
//     // src and dst share the same L1 switch
//     // Path: src (NIC1) -> L1 -> dst (2 hops)
//     else if (src_l1 == dst_l1)
//     {
//         routeout = new Route();
//         routeout->push_back(queues_ns_l1[src][src_l1]);
//         routeout->push_back(pipes_ns_l1[src][src_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_ns_l1[src][src_l1]->getRemoteEndpoint());
//         routeout->push_back(queues_l1_ns[dst_l1][dest]);
//         routeout->push_back(pipes_l1_ns[dst_l1][dest]);

//         routeback = new Route();
//         routeback->push_back(queues_ns_l1[dest][dst_l1]);
//         routeback->push_back(pipes_ns_l1[dest][dst_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_ns_l1[dest][dst_l1]->getRemoteEndpoint());
//         routeback->push_back(queues_l1_ns[src_l1][src]);
//         routeback->push_back(pipes_l1_ns[src_l1][src]);

//         routeout->set_reverse(routeback);
//         routeback->set_reverse(routeout);
//         paths->push_back(routeout);
//         check_non_null(routeout);
//         return paths;
//     }

//     // --- Case 3: different L0 group AND different L1 index ------
//     // Two paths available for ECMP:
//     // Path A: src → NIC0 → L0[src] → L1[dst] → dst
//     // Path B: src → NIC1 → L1[src] → L0[dst] → dst
//     else
//     {
//         // Path A: via src's L0 then dst's L1
//         routeout = new Route();
//         routeout->push_back(queues_ns_l0[src][src_l0]);
//         routeout->push_back(pipes_ns_l0[src][src_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_ns_l0[src][src_l0]->getRemoteEndpoint());
//         routeout->push_back(queues_l0_l1[src_l0][dst_l1]);
//         routeout->push_back(pipes_l0_l1[src_l0][dst_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_l0_l1[src_l0][dst_l1]->getRemoteEndpoint());
//         routeout->push_back(queues_l1_ns[dst_l1][dest]);
//         routeout->push_back(pipes_l1_ns[dst_l1][dest]);

//         routeback = new Route();
//         routeback->push_back(queues_ns_l1[dest][dst_l1]);
//         routeback->push_back(pipes_ns_l1[dest][dst_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_ns_l1[dest][dst_l1]->getRemoteEndpoint());
//         routeback->push_back(queues_l1_l0[dst_l1][src_l0]);
//         routeback->push_back(pipes_l1_l0[dst_l1][src_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_l1_l0[dst_l1][src_l0]->getRemoteEndpoint());
//         routeback->push_back(queues_l0_ns[src_l0][src]);
//         routeback->push_back(pipes_l0_ns[src_l0][src]);

//         routeout->set_reverse(routeback);
//         routeback->set_reverse(routeout);
//         paths->push_back(routeout);
//         check_non_null(routeout);

//         // Path B: via src's L1 then dst's L0
//         routeout = new Route();
//         routeout->push_back(queues_ns_l1[src][src_l1]);
//         routeout->push_back(pipes_ns_l1[src][src_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_ns_l1[src][src_l1]->getRemoteEndpoint());
//         routeout->push_back(queues_l1_l0[src_l1][dst_l0]);
//         routeout->push_back(pipes_l1_l0[src_l1][dst_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeout->push_back(queues_l1_l0[src_l1][dst_l0]->getRemoteEndpoint());
//         routeout->push_back(queues_l0_ns[dst_l0][dest]);
//         routeout->push_back(pipes_l0_ns[dst_l0][dest]);

//         routeback = new Route();
//         routeback->push_back(queues_ns_l0[dest][dst_l0]);
//         routeback->push_back(pipes_ns_l0[dest][dst_l0]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_ns_l0[dest][dst_l0]->getRemoteEndpoint());
//         routeback->push_back(queues_l0_l1[dst_l0][src_l1]);
//         routeback->push_back(pipes_l0_l1[dst_l0][src_l1]);
//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//             routeback->push_back(queues_l0_l1[dst_l0][src_l1]->getRemoteEndpoint());
//         routeback->push_back(queues_l1_ns[src_l1][src]);
//         routeback->push_back(pipes_l1_ns[src_l1][src]);

//         routeout->set_reverse(routeback);
//         routeback->set_reverse(routeout);
//         paths->push_back(routeout);
//         check_non_null(routeout);

//         return paths;
//     }
// }

// /*
// vector<const Route *> *FatTreeTopology::get_paths(int src, int dest)
// {
//   vector<const Route *> *paths = new vector<const Route *>();

//   route_t *routeout, *routeback;
//   //QueueLoggerSimple *simplequeuelogger = new QueueLoggerSimple();
//   //QueueLoggerSimple *simplequeuelogger = 0;
//   //logfile->addLogger(*simplequeuelogger);
//   //Queue* pqueue = new Queue(speedFromMbps((uint64_t)SPEED), memFromPkt(FEEDER_BUFFER), *eventlist, simplequeuelogger);
//   //pqueue->setName("PQueue_" + ntoa(src) + "_" + ntoa(dest));
//   //logfile->writeName(*pqueue);
//   if (HOST_POD_SWITCH(src) == HOST_POD_SWITCH(dest))
//   {

//     // forward path
//     routeout = new Route();
//     //routeout->push_back(pqueue);
//     routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]);
//     routeout->push_back(pipes_ns_nlp[src][HOST_POD_SWITCH(src)]);

//     if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//       routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]->getRemoteEndpoint());

//     routeout->push_back(queues_nlp_ns[HOST_POD_SWITCH(dest)][dest]);
//     routeout->push_back(pipes_nlp_ns[HOST_POD_SWITCH(dest)][dest]);

//     // reverse path for RTS packets
//     routeback = new Route();
//     routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]);
//     routeback->push_back(pipes_ns_nlp[dest][HOST_POD_SWITCH(dest)]);

//     if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//       routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]->getRemoteEndpoint());

//     routeback->push_back(queues_nlp_ns[HOST_POD_SWITCH(src)][src]);
//     routeback->push_back(pipes_nlp_ns[HOST_POD_SWITCH(src)][src]);

//     routeout->set_reverse(routeback);
//     routeback->set_reverse(routeout);

//     //print_route(*routeout);
//     paths->push_back(routeout);

//     check_non_null(routeout);
//     return paths;
//   }
//   else if (HOST_POD(src) == HOST_POD(dest))
//   {
//     //don't go up the hierarchy, stay in the pod only.

//     int pod = HOST_POD(src);
//     //there are K/2 paths between the source and the destination
//     for (int upper = MIN_POD_ID(pod); upper <= MAX_POD_ID(pod); upper++)
//     {
//       //upper is nup

//       routeout = new Route();
//       //routeout->push_back(pqueue);

//       routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]);
//       routeout->push_back(pipes_ns_nlp[src][HOST_POD_SWITCH(src)]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]->getRemoteEndpoint());

//       routeout->push_back(queues_nlp_nup[HOST_POD_SWITCH(src)][upper]);
//       routeout->push_back(pipes_nlp_nup[HOST_POD_SWITCH(src)][upper]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeout->push_back(queues_nlp_nup[HOST_POD_SWITCH(src)][upper]->getRemoteEndpoint());

//       routeout->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(dest)]);
//       routeout->push_back(pipes_nup_nlp[upper][HOST_POD_SWITCH(dest)]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeout->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(dest)]->getRemoteEndpoint());

//       routeout->push_back(queues_nlp_ns[HOST_POD_SWITCH(dest)][dest]);
//       routeout->push_back(pipes_nlp_ns[HOST_POD_SWITCH(dest)][dest]);

//       // reverse path for RTS packets
//       routeback = new Route();

//       routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]);
//       routeback->push_back(pipes_ns_nlp[dest][HOST_POD_SWITCH(dest)]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]->getRemoteEndpoint());

//       routeback->push_back(queues_nlp_nup[HOST_POD_SWITCH(dest)][upper]);
//       routeback->push_back(pipes_nlp_nup[HOST_POD_SWITCH(dest)][upper]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeback->push_back(queues_nlp_nup[HOST_POD_SWITCH(dest)][upper]->getRemoteEndpoint());

//       routeback->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(src)]);
//       routeback->push_back(pipes_nup_nlp[upper][HOST_POD_SWITCH(src)]);

//       if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//         routeback->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(src)]->getRemoteEndpoint());

//       routeback->push_back(queues_nlp_ns[HOST_POD_SWITCH(src)][src]);
//       routeback->push_back(pipes_nlp_ns[HOST_POD_SWITCH(src)][src]);

//       routeout->set_reverse(routeback);
//       routeback->set_reverse(routeout);

//       //print_route(*routeout);
//       paths->push_back(routeout);
//       check_non_null(routeout);
//     }
//     return paths;
//   }
//   else
//   {
//     int pod = HOST_POD(src);

//     for (int upper = MIN_POD_ID(pod); upper <= MAX_POD_ID(pod); upper++)
//       for (int core = (upper % (K / 2)) * K / 2; core < ((upper % (K / 2)) + 1) * K / 2; core++)
//       {
//         //upper is nup

//         routeout = new Route();
//         //routeout->push_back(pqueue);

//         routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]);
//         routeout->push_back(pipes_ns_nlp[src][HOST_POD_SWITCH(src)]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeout->push_back(queues_ns_nlp[src][HOST_POD_SWITCH(src)]->getRemoteEndpoint());

//         routeout->push_back(queues_nlp_nup[HOST_POD_SWITCH(src)][upper]);
//         routeout->push_back(pipes_nlp_nup[HOST_POD_SWITCH(src)][upper]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeout->push_back(queues_nlp_nup[HOST_POD_SWITCH(src)][upper]->getRemoteEndpoint());

//         routeout->push_back(queues_nup_nc[upper][core]);
//         routeout->push_back(pipes_nup_nc[upper][core]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeout->push_back(queues_nup_nc[upper][core]->getRemoteEndpoint());

//         //now take the only link down to the destination server!

//         int upper2 = HOST_POD(dest) * K / 2 + 2 * core / K;
//         //printf("K %d HOST_POD(%d) %d core %d upper2 %d\n",K,dest,HOST_POD(dest),core, upper2);

//         routeout->push_back(queues_nc_nup[core][upper2]);
//         routeout->push_back(pipes_nc_nup[core][upper2]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeout->push_back(queues_nc_nup[core][upper2]->getRemoteEndpoint());

//         routeout->push_back(queues_nup_nlp[upper2][HOST_POD_SWITCH(dest)]);
//         routeout->push_back(pipes_nup_nlp[upper2][HOST_POD_SWITCH(dest)]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeout->push_back(queues_nup_nlp[upper2][HOST_POD_SWITCH(dest)]->getRemoteEndpoint());

//         routeout->push_back(queues_nlp_ns[HOST_POD_SWITCH(dest)][dest]);
//         routeout->push_back(pipes_nlp_ns[HOST_POD_SWITCH(dest)][dest]);

//         // reverse path for RTS packets
//         routeback = new Route();

//         routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]);
//         routeback->push_back(pipes_ns_nlp[dest][HOST_POD_SWITCH(dest)]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeback->push_back(queues_ns_nlp[dest][HOST_POD_SWITCH(dest)]->getRemoteEndpoint());
//         routeback->push_back(queues_nlp_nup[HOST_POD_SWITCH(dest)][upper2]);
//         routeback->push_back(pipes_nlp_nup[HOST_POD_SWITCH(dest)][upper2]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeback->push_back(queues_nlp_nup[HOST_POD_SWITCH(dest)][upper2]->getRemoteEndpoint());

//         routeback->push_back(queues_nup_nc[upper2][core]);
//         routeback->push_back(pipes_nup_nc[upper2][core]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeback->push_back(queues_nup_nc[upper2][core]->getRemoteEndpoint());

//         //now take the only link back down to the src server!

//         routeback->push_back(queues_nc_nup[core][upper]);
//         routeback->push_back(pipes_nc_nup[core][upper]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeback->push_back(queues_nc_nup[core][upper]->getRemoteEndpoint());

//         routeback->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(src)]);
//         routeback->push_back(pipes_nup_nlp[upper][HOST_POD_SWITCH(src)]);

//         if (qt == LOSSLESS_INPUT || qt == LOSSLESS_INPUT_ECN)
//           routeback->push_back(queues_nup_nlp[upper][HOST_POD_SWITCH(src)]->getRemoteEndpoint());

//         routeback->push_back(queues_nlp_ns[HOST_POD_SWITCH(src)][src]);
//         routeback->push_back(pipes_nlp_ns[HOST_POD_SWITCH(src)][src]);

//         routeout->set_reverse(routeback);
//         routeback->set_reverse(routeout);

//         //print_route(*routeout);
//         paths->push_back(routeout);
//         check_non_null(routeout);
//       }
//     return paths;
//   }
// }
// */

// /*
// void FatTreeTopology::count_queue(Queue *queue)
// {
//   if (_link_usage.find(queue) == _link_usage.end())
//   {
//     _link_usage[queue] = 0;
//   }

//   _link_usage[queue] = _link_usage[queue] + 1;
// }
// */

// void ZCubeTopology::count_queue(Queue* queue)
// {
//     if (_link_usage.find(queue) == _link_usage.end())
//         _link_usage[queue] = 0;
//     _link_usage[queue]++;
// }

// /*
// int FatTreeTopology::find_lp_switch(Queue *queue)
// {
//   //first check ns_nlp
//   for (int i = 0; i < NSRV; i++)
//     for (int j = 0; j < NK; j++)
//       if (queues_ns_nlp[i][j] == queue)
//         return j;

//   //only count nup to nlp
//   count_queue(queue);

//   for (int i = 0; i < NK; i++)
//     for (int j = 0; j < NK; j++)
//       if (queues_nup_nlp[i][j] == queue)
//         return j;

//   return -1;
// }

// int FatTreeTopology::find_up_switch(Queue *queue)
// {
//   count_queue(queue);
//   //first check nc_nup
//   for (int i = 0; i < NC; i++)
//     for (int j = 0; j < NK; j++)
//       if (queues_nc_nup[i][j] == queue)
//         return j;

//   //check nlp_nup
//   for (int i = 0; i < NK; i++)
//     for (int j = 0; j < NK; j++)
//       if (queues_nlp_nup[i][j] == queue)
//         return j;

//   return -1;
// }

// int FatTreeTopology::find_core_switch(Queue *queue)
// {
//   count_queue(queue);
//   //first check nup_nc
//   for (int i = 0; i < NK; i++)
//     for (int j = 0; j < NC; j++)
//       if (queues_nup_nc[i][j] == queue)
//         return j;

//   return -1;
// }

// int FatTreeTopology::find_destination(Queue *queue)
// {
//   //first check nlp_ns
//   for (int i = 0; i < NK; i++)
//     for (int j = 0; j < NSRV; j++)
//       if (queues_nlp_ns[i][j] == queue)
//         return j;

//   return -1;
// }
// */

// int ZCubeTopology::find_l0_switch(Queue* queue)
// {
//     for (int l0 = 0; l0 < NL0; l0++)
//         for (int g = 0; g < NSRV; g++)
//             if (queues_l0_ns[l0][g] == queue) return l0;
//     return -1;
// }

// int ZCubeTopology::find_l1_switch(Queue* queue)
// {
//     for (int l1 = 0; l1 < NL1; l1++)
//         for (int g = 0; g < NSRV; g++)
//             if (queues_l1_ns[l1][g] == queue) return l1;
//     return -1;
// }

// int ZCubeTopology::find_destination(Queue* queue)
// {
//     // Find which GPU this downlink queue points to
//     for (int g = 0; g < NSRV; g++) {
//         for (int l0 = 0; l0 < NL0; l0++)
//             if (queues_l0_ns[l0][g] == queue) return g;
//         for (int l1 = 0; l1 < NL1; l1++)
//             if (queues_l1_ns[l1][g] == queue) return g;
//     }
//     return -1;
// }


// /*
// void FatTreeTopology::print_path(std::ofstream &paths, int src, const Route *route)
// {
//   paths << "SRC_" << src << " ";

//   if (route->size() / 2 == 2)
//   {
//     paths << "LS_" << find_lp_switch((Queue *)route->at(1)) << " ";
//     paths << "DST_" << find_destination((Queue *)route->at(3)) << " ";
//   }
//   else if (route->size() / 2 == 4)
//   {
//     paths << "LS_" << find_lp_switch((Queue *)route->at(1)) << " ";
//     paths << "US_" << find_up_switch((Queue *)route->at(3)) << " ";
//     paths << "LS_" << find_lp_switch((Queue *)route->at(5)) << " ";
//     paths << "DST_" << find_destination((Queue *)route->at(7)) << " ";
//   }
//   else if (route->size() / 2 == 6)
//   {
//     paths << "LS_" << find_lp_switch((Queue *)route->at(1)) << " ";
//     paths << "US_" << find_up_switch((Queue *)route->at(3)) << " ";
//     paths << "CS_" << find_core_switch((Queue *)route->at(5)) << " ";
//     paths << "US_" << find_up_switch((Queue *)route->at(7)) << " ";
//     paths << "LS_" << find_lp_switch((Queue *)route->at(9)) << " ";
//     paths << "DST_" << find_destination((Queue *)route->at(11)) << " ";
//   }
//   else
//   {
//     paths << "Wrong hop count " << ntoa(route->size() / 2);
//   }

//   paths << endl;
// }
// */

// void ZCubeTopology::print_path(std::ofstream& paths, int src, const Route* route)
// {
//     paths << "SRC_" << src << " ";
//     paths << "ZCube(" << n << ",2) path (" << (route->size()/2) << " hops) ";

//     // Optional: show L0/L1 switches for debugging
//     int l0 = find_l0_switch((Queue*)route->at(1));  // first queue after src
//     int l1 = find_l1_switch((Queue*)route->at(1));
//     if (l0 >= 0) paths << "L0_" << l0 << " ";
//     if (l1 >= 0) paths << "L1_" << l1 << " ";

//     paths << "DST_" << find_destination((Queue*)route->at(route->size() - 1)) << " ";
//     paths << endl;
// } 

// //////////////////////////////////////////////
// //      Aggregate utilization monitor       //
// //////////////////////////////////////////////

// // UtilMonitor::UtilMonitor(FatTreeTopology* top, EventList &eventlist)
// //   : EventSource(eventlist,"utilmonitor"), _top(top)
// // {
// //     _H = _top->no_of_nodes(); // number of hosts
// //     _N = _H / (_top->get_k() / 2); // racks
// //     _hpr = _top->get_k() / 2; // hosts per rack
// //     uint64_t rate = 10000000000 / 8; // bytes / second
// //     rate = rate * _H;

// //     _max_agg_Bps = rate;

// //     // debug:
// //     //cout << "max packets per second = " << rate << endl;

// // }

// // void UtilMonitor::start(simtime_picosec period) {
// //     _period = period;
// //     _max_B_in_period = _max_agg_Bps * timeAsSec(_period);

// //     // debug:
// //     //cout << "_max_pkts_in_period = " << _max_pkts_in_period << endl;

// //     eventlist().sourceIsPending(*this, _period);
// // }

// // void UtilMonitor::doNextEvent() {
// //     printAggUtil();
// // }

// // void UtilMonitor::printAggUtil() {

// //     uint64_t B_sum = 0;

// //     int host = 0;
// //     for (int tor = 0; tor < _N; tor++) {
// //         for (int downlink = 0; downlink < _hpr; downlink++) {
// //             Pipe* pipe = _top->get_downlink(tor, host);
// //             B_sum = B_sum + pipe->reportBytes();
// //             host++;
// //         }
// //     }

// //     // debug:
// //     //cout << "Bsum = " << B_sum << endl;
// //     //cout << "_max_B_in_period = " << _max_B_in_period << endl;

// //     double util = (double)B_sum / (double)_max_B_in_period;

// //     fct_util_out << "Util " << fixed << util << " " << timeAsMs(eventlist().now()) << endl;

// //     //if (eventlist().now() + _period < eventlist().getEndtime())
// //     eventlist().sourceIsPendingRel(*this, _period);

// // }

// ── ForestColl schedule — added by apply_zcube_patches.py ────────────────────
// ZCUBE_FC_CPP_PATCH

void ZCubeTopology::load_fc_schedule()
{
    // Pull in the generated static arrays using macros to avoid name collision.
    #define _fc_num_steps       _gen_fc_num_steps
    #define _fc_rs_total_chunks _gen_fc_rs_total_chunks
    #define _fc_ag_total_chunks _gen_fc_ag_total_chunks
    #define _fc_rs_steps        _gen_fc_rs_steps
    #define _fc_ag_steps        _gen_fc_ag_steps
    #include "zcube_fc_schedule_generated.cpp"
    #undef _fc_num_steps
    #undef _fc_rs_total_chunks
    #undef _fc_ag_total_chunks
    #undef _fc_rs_steps
    #undef _fc_ag_steps
    _fc_num_steps       = _gen_fc_num_steps;
    _fc_rs_total_chunks = _gen_fc_rs_total_chunks;
    _fc_ag_total_chunks = _gen_fc_ag_total_chunks;
    _fc_rs_steps        = _gen_fc_rs_steps;
    _fc_ag_steps        = _gen_fc_ag_steps;
}

const std::vector<ZCubeFCTransmission>&
ZCubeTopology::fc_rs_step(int step) const
{
    assert(step >= 0 && step < (int)_fc_rs_steps.size());
    return _fc_rs_steps[step];
}

const std::vector<ZCubeFCTransmission>&
ZCubeTopology::fc_ag_step(int step) const
{
    assert(step >= 0 && step < (int)_fc_ag_steps.size());
    return _fc_ag_steps[step];
}

int ZCubeTopology::fc_num_steps()       const { return _fc_num_steps; }
int ZCubeTopology::fc_rs_total_chunks() const { return _fc_rs_total_chunks; }
int ZCubeTopology::fc_ag_total_chunks() const { return _fc_ag_total_chunks; }
