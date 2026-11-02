#include "zcube_partial_topology.h"
#include "queue.h"
#include "prioqueue.h"
#include "compositequeue.h"
#include "ecnqueue.h"
#include "queue_lossless.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
#include <cassert>
#include <iostream>
#include <sstream>

extern uint32_t RTT_rack;
extern uint32_t RTT_net;
extern uint32_t SPEED;

string ntoa(double n);

ZCubePartialTopology::ZCubePartialTopology(int no_of_nodes, mem_b queuesize, Logfile* log,
                                           EventList* ev, FirstFit* f, queue_type q)
    : Topology()
{
    logfile = log;
    eventlist = ev;
    ff = f;
    qt = q;
    _queuesize = queuesize;

    NPOD = 2;
    n = 1;
    while (NPOD * n * n < no_of_nodes) n++;
    NSRV = NPOD * n * n;
    NL0 = NPOD * n;
    NL1 = NPOD * n;
    NC = n;
    _no_of_nodes = NSRV;
    _linkspeed = (uint64_t)SPEED / 2;

    std::cout << "Building ZCube(" << n << ",2)-Partial with " << NSRV << " GPUs in "
              << NPOD << " pods (" << NL0 << " L0 + " << NL1 << " L1 + " << NC
              << " core switches), links at " << _linkspeed << " Mbps" << std::endl;

    init_network();
}

Queue* ZCubePartialTopology::alloc_src_queue(QueueLogger* ql, uint64_t speed)
{
    return new PriorityQueue(speedFromMbps(speed), memFromPkt(FEEDER_BUFFER), *eventlist, ql);
}

Queue* ZCubePartialTopology::alloc_queue(QueueLogger* ql, uint64_t speed)
{
    if (qt == ECN)
        return new ECNQueue(speedFromMbps(speed), _queuesize, *eventlist, ql, memFromPkt(50));
    else if (qt == LOSSLESS_INPUT)
        return new LosslessOutputQueue(speedFromMbps(speed), memFromPkt(200), *eventlist, ql);
    else if (qt == COMPOSITE)
        return new CompositeQueue(speedFromMbps(speed), _queuesize, *eventlist, ql);
    std::cerr << "ZCubePartialTopology: unsupported queue type (use ECN or LOSSLESS_INPUT)\n";
    assert(0);
    return NULL;
}

void ZCubePartialTopology::add_in(Route* rt, Queue* q)
{
    if (qt == LOSSLESS_INPUT)
        rt->push_back(q->getRemoteEndpoint());
}

void ZCubePartialTopology::init_network()
{
    QueueLogger* ql = NULL;

    q_gpu_l0.assign(NSRV, NULL); q_l0_gpu.assign(NSRV, NULL);
    q_gpu_l1.assign(NSRV, NULL); q_l1_gpu.assign(NSRV, NULL);
    p_gpu_l0.assign(NSRV, NULL); p_l0_gpu.assign(NSRV, NULL);
    p_gpu_l1.assign(NSRV, NULL); p_l1_gpu.assign(NSRV, NULL);

    q_l0_l1.assign(NL0, std::vector<Queue*>(NL1, NULL));
    q_l1_l0.assign(NL1, std::vector<Queue*>(NL0, NULL));
    p_l0_l1.assign(NL0, std::vector<Pipe*>(NL1, NULL));
    p_l1_l0.assign(NL1, std::vector<Pipe*>(NL0, NULL));

    q_l1_c.assign(NL1, std::vector<Queue*>(NC, NULL));
    q_c_l1.assign(NC, std::vector<Queue*>(NL1, NULL));
    p_l1_c.assign(NL1, std::vector<Pipe*>(NC, NULL));
    p_c_l1.assign(NC, std::vector<Pipe*>(NL1, NULL));

    for (int g = 0; g < NSRV; g++) {
        int l0 = gpu_l0(g), l1 = gpu_l1(g);

        q_gpu_l0[g] = alloc_src_queue(ql, _linkspeed);
        q_gpu_l0[g]->setName("GPU_" + ntoa(g) + "->L0_" + ntoa(l0));
        p_gpu_l0[g] = new Pipe(timeFromNs(RTT_rack), *eventlist);

        q_l0_gpu[g] = alloc_queue(ql, _linkspeed);
        q_l0_gpu[g]->setName("L0_" + ntoa(l0) + "->GPU_" + ntoa(g));
        p_l0_gpu[g] = new Pipe(timeFromNs(RTT_rack), *eventlist);

        q_gpu_l1[g] = alloc_src_queue(ql, _linkspeed);
        q_gpu_l1[g]->setName("GPU_" + ntoa(g) + "->L1_" + ntoa(l1));
        p_gpu_l1[g] = new Pipe(timeFromNs(RTT_net), *eventlist);

        q_l1_gpu[g] = alloc_queue(ql, _linkspeed);
        q_l1_gpu[g]->setName("L1_" + ntoa(l1) + "->GPU_" + ntoa(g));
        p_l1_gpu[g] = new Pipe(timeFromNs(RTT_net), *eventlist);

        if (qt == LOSSLESS_INPUT) {
            new LosslessInputQueue(*eventlist, q_gpu_l0[g]);
            new LosslessInputQueue(*eventlist, q_gpu_l1[g]);
        }
    }

    for (int pod = 0; pod < NPOD; pod++)
        for (int a = 0; a < n; a++)
            for (int b = 0; b < n; b++) {
                int l0 = pod * n + a, l1 = pod * n + b;

                q_l0_l1[l0][l1] = alloc_queue(ql, _linkspeed);
                q_l0_l1[l0][l1]->setName("L0_" + ntoa(l0) + "->L1_" + ntoa(l1));
                p_l0_l1[l0][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);

                q_l1_l0[l1][l0] = alloc_queue(ql, _linkspeed);
                q_l1_l0[l1][l0]->setName("L1_" + ntoa(l1) + "->L0_" + ntoa(l0));
                p_l1_l0[l1][l0] = new Pipe(timeFromNs(RTT_net), *eventlist);

                if (qt == LOSSLESS_INPUT) {
                    new LosslessInputQueue(*eventlist, q_l0_l1[l0][l1]);
                    new LosslessInputQueue(*eventlist, q_l1_l0[l1][l0]);
                }
            }

    for (int l1 = 0; l1 < NL1; l1++)
        for (int c = 0; c < NC; c++) {
            q_l1_c[l1][c] = alloc_queue(ql, _linkspeed);
            q_l1_c[l1][c]->setName("L1_" + ntoa(l1) + "->C_" + ntoa(c));
            p_l1_c[l1][c] = new Pipe(timeFromNs(RTT_net), *eventlist);

            q_c_l1[c][l1] = alloc_queue(ql, _linkspeed);
            q_c_l1[c][l1]->setName("C_" + ntoa(c) + "->L1_" + ntoa(l1));
            p_c_l1[c][l1] = new Pipe(timeFromNs(RTT_net), *eventlist);

            if (qt == LOSSLESS_INPUT) {
                new LosslessInputQueue(*eventlist, q_l1_c[l1][c]);
                new LosslessInputQueue(*eventlist, q_c_l1[c][l1]);
            }
        }
}

vector<const Route*>* ZCubePartialTopology::get_paths(int src, int dest)
{
    vector<const Route*>* paths = new vector<const Route*>();
    assert(src != dest);

    int spod = gpu_pod(src), dpod = gpu_pod(dest);
    int sl0 = gpu_l0(src), dl0 = gpu_l0(dest);
    int sl1 = gpu_l1(src), dl1 = gpu_l1(dest);

    if (spod == dpod) {
        if (sl0 == dl0) {
            Route* r = new Route();
            r->push_back(q_gpu_l0[src]); r->push_back(p_gpu_l0[src]); add_in(r, q_gpu_l0[src]);
            r->push_back(q_l0_gpu[dest]); r->push_back(p_l0_gpu[dest]);
            paths->push_back(r);
            return paths;
        }
        if (sl1 == dl1) {
            Route* r = new Route();
            r->push_back(q_gpu_l1[src]); r->push_back(p_gpu_l1[src]); add_in(r, q_gpu_l1[src]);
            r->push_back(q_l1_gpu[dest]); r->push_back(p_l1_gpu[dest]);
            paths->push_back(r);
            return paths;
        }
        Route* a = new Route();
        a->push_back(q_gpu_l0[src]); a->push_back(p_gpu_l0[src]); add_in(a, q_gpu_l0[src]);
        a->push_back(q_l0_l1[sl0][dl1]); a->push_back(p_l0_l1[sl0][dl1]); add_in(a, q_l0_l1[sl0][dl1]);
        a->push_back(q_l1_gpu[dest]); a->push_back(p_l1_gpu[dest]);
        paths->push_back(a);

        Route* b = new Route();
        b->push_back(q_gpu_l1[src]); b->push_back(p_gpu_l1[src]); add_in(b, q_gpu_l1[src]);
        b->push_back(q_l1_l0[sl1][dl0]); b->push_back(p_l1_l0[sl1][dl0]); add_in(b, q_l1_l0[sl1][dl0]);
        b->push_back(q_l0_gpu[dest]); b->push_back(p_l0_gpu[dest]);
        paths->push_back(b);
        return paths;
    }

    for (int c = 0; c < NC; c++) {
        Route* r = new Route();
        r->push_back(q_gpu_l1[src]);  r->push_back(p_gpu_l1[src]);  add_in(r, q_gpu_l1[src]);
        r->push_back(q_l1_c[sl1][c]); r->push_back(p_l1_c[sl1][c]); add_in(r, q_l1_c[sl1][c]);
        r->push_back(q_c_l1[c][dl1]); r->push_back(p_c_l1[c][dl1]); add_in(r, q_c_l1[c][dl1]);
        r->push_back(q_l1_gpu[dest]); r->push_back(p_l1_gpu[dest]);
        paths->push_back(r);
    }
    return paths;
}
