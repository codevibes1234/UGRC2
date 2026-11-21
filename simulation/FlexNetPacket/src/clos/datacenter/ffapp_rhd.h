// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
/*
 * ffapp_rhd.h
 * ===========
 * Minimal header for the isolated ZCube recursive-halving /
 * recursive-doubling (RHD) AllReduce microbenchmark.
 *
 * Use with:
 *   - ffapp_rhd.cpp          (stubs + FFZCubeRHDAllreduceTask impl)
 *   - main_tcp_zcube_rhd_bench.cpp
 *
 * Do NOT include the full ffapp.h when building the RHD bench, and
 * do NOT link the full ffapp.o (duplicate symbols).
 */

#ifndef FFAPP_RHD_H
#define FFAPP_RHD_H

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "eventlist.h"
#include "network.h"   // EventSource, simtime_picosec, …
#include "tcp.h"       // TcpRtxTimerScanner
#include "topology.h"

class FFTask;
class FFDevice;
class Topology;

// ============================================================================
// Enums
// ============================================================================

enum FFAllReduceStrategy {
    FF_DEFAULT_AR = 0,
    FF_RING_AR,
    FF_PS_AR,
    FF_DPS_AR,
    FF_ZCUBE_RHD_AR,   // recursive halving + recursive doubling on ZCube
    // (other strategies omitted — not used by this bench)
};

// ============================================================================
// FFApplication — minimal surface used by the RHD bench
// ============================================================================

class FFApplication {
public:
    static int total_apps;
    static int finished_apps;

    Topology*               topology;
    int                     ssthresh;
    EventList&              eventlist;
    std::ofstream*          fstream_out;
    TcpRtxTimerScanner&     tcpRtxScanner;
    FFAllReduceStrategy     allreduce_strategy;

    simtime_picosec         final_finish_time;
    simtime_picosec         first_iter_time;
    int                     n_finished_tasks;
    bool                    finished_once;
    bool                    fancy_ring;
    double                  bfb_rto_ms;

    int                     ngpupernode;
    int                     nnodes;
    int                     nswitches;
    int                     cwnd;
    double                  pull_rate;

    std::vector<int>                    gpus;
    std::map<uint64_t, FFTask*>         tasks;
    std::map<uint64_t, FFDevice*>       devices;

    // Constructors used by main_tcp_zcube_rhd_bench.cpp / ffapp_rhd.cpp
    FFApplication(Topology* top, int ss, std::ofstream* _fstream_out,
                  TcpRtxTimerScanner& rtx, EventList& evl,
                  FFAllReduceStrategy ars);

    FFApplication(Topology* top, int ss, std::ofstream* _fstream_out,
                  std::vector<int> gpus_,
                  TcpRtxTimerScanner& rtx, EventList& evl,
                  FFAllReduceStrategy ars);

    ~FFApplication();

    // Stubs required by the header contract (assert in .cpp for the bench)
    void load_taskgraph_json(std::string& taskgraph);
    void load_taskgraph_flatbuf(std::string& taskgraph);
    void start_init_tasks();
    void reset_and_restart();
    std::vector<int> choose_gpus(std::unordered_set<int>& candidates, int n);
};

// ============================================================================
// FFDevice — minimal stub
// ============================================================================

class FFDevice {
public:
    enum DeviceType {
        DEVICE_GPU = 0,
        DEVICE_CPU,
        DEVICE_NETWORK
    };
    enum DeviceState {
        DEVICE_IDLE = 0,
        DEVICE_BUSY
    };

    FFApplication* ffapp;
    int            node_id;
    int            gpu_id;
    float          bandwidth;
    DeviceType     type;
    DeviceState    state;
    int            from_gpu, to_gpu;
    int            from_node, to_node;
    simtime_picosec busy_up_to;

    // String-style constructor (legacy)
    FFDevice(FFApplication* app, std::string type_str, float bw,
             int nid, int gid, int fn, int tn, int fg, int tg);

    // FlatBuf-style constructor
    FFDevice(FFApplication* app, /* FlatBufTaskGraph::DeviceType */ int /*dtype*/,
             uint64_t nid, uint64_t prop, uint64_t bw);
};

// ============================================================================
// FFTask — minimal base used by FFZCubeRHDAllreduceTask
// ============================================================================

class FFTask : public EventSource {
public:
    enum FFTaskType {
        TASK_COMM = 0,
        TASK_COMPUTE,
        TASK_ALLREDUCE,
        TASK_OTHER
    };
    enum FFTaskState {
        TASK_NOT_READY = 0,
        TASK_READY,
        TASK_RUNNING,
        TASK_FINISHED
    };

    FFApplication*   ffapp;
    FFTaskType       type;
    FFTaskState      state;
    FFDevice*        device;

    unsigned int     counter;
    uint64_t         xfersize;
    int              src_node;
    int              dst_node;

    simtime_picosec  ready_time;
    simtime_picosec  run_time;
    simtime_picosec  start_time;
    simtime_picosec  finish_time;

    std::vector<uint64_t> next_tasks;

    // Primary constructor used by RHD
    FFTask(FFApplication* app, FFTaskType t);

    // Extra constructors required by linker / full header contract
    FFTask(FFApplication* app, std::string type_str,
           FFDevice* dev, uint64_t sz, float rt);
    FFTask(FFApplication* app, /* FlatBufTaskGraph::SimTaskType */ int /*tt*/,
           FFDevice* dev, uint64_t sz, float rt);

    virtual ~FFTask() {}

    virtual void doNextEvent();
    virtual void cleanup();

    // Stubs
    void taskstart();
    void start_flow();
    void execute_compute();
    void add_nextask(FFTask* t);
};

// Callback used by some flows
void taskfinish(void* task);

// ============================================================================
// FFZCubeRHDAllreduceTask
// Recursive-halving (RS) + recursive-doubling (AG) AllReduce.
// Uses only Topology::get_paths — no ZCube-specific downcast needed.
// ============================================================================

class FFZCubeRHDAllreduceTask : public FFTask {
public:
    Topology*              _topology;
    std::vector<uint64_t>  node_group;
    uint64_t               operator_size;

    int curr_step;
    int finished_curr_step;
    int flows_this_step;
    int num_steps;   // log2(N); total steps = 2 * num_steps

    FFZCubeRHDAllreduceTask(FFApplication*        ffapp,
                            Topology*             top,
                            std::vector<uint64_t> ng,
                            uint64_t              sz,
                            double                local_runtime);

    void doNextEvent() override;
    void start_all_flows(int step);
    void start_one_flow(int src, int dst, uint64_t bytes, simtime_picosec t0);
    void advance();
};

void ar_finish_rhd_zcube(void* ar_ptr);

// ============================================================================
// ============================================================================
class FFGroupRingAllreduceTask : public FFTask {
public:
    Topology*              _topology;
    std::vector<uint64_t>  node_group;
    uint64_t               operator_size;

    int curr_step;
    int finished_curr_step;
    int flows_this_step;
    int num_steps;

    FFGroupRingAllreduceTask(FFApplication*        ffapp,
                             Topology*             top,
                             std::vector<uint64_t> ng,
                             uint64_t              sz);

    void doNextEvent() override;
    void start_all_flows();
    void advance();
};

void ar_finish_group_ring(void* ar_ptr);

#endif // FFAPP_RHD_H
