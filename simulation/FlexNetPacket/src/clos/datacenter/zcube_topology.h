#ifndef ZCUBE_TOPOLOGY
#define ZCUBE_TOPOLOGY
#include "main.h"
#include "randomqueue.h"
#include "pipe.h"
#include "config.h"
#include "loggers.h"
#include "network.h"
#include "firstfit.h"
#include "topology.h"
#include "logfile.h"
#include "eventlist.h"
#include "switch.h"
#include <ostream>

//#define N K*K*K/4

// #define HOST_POD_SWITCH(src) (2*src/K)
//#define HOST_POD_ID(src) src%NSRV
// #define HOST_POD(src) (src/NC)

#define GPU_L0(gpu) (gpu / n)
#define GPU_L1(gpu) (gpu % n)

// #define MIN_POD_ID(pod_id) (pod_id*K/2)
// #define MAX_POD_ID(pod_id) ((pod_id+1)*K/2-1)

#ifndef QT
#define QT
typedef enum {RANDOM, ECN, COMPOSITE, CTRL_PRIO, LOSSLESS, LOSSLESS_INPUT, LOSSLESS_INPUT_ECN} queue_type;
#endif

#include "fc_schedule.h"

class ZCubeTopology: public Topology, public FCScheduleSource, public ExplicitRouteTopology{
 public:

  vector <Switch*> switches_l0; // lower pod
  vector <Switch*> switches_l1; // upper pod

  vector< vector<Pipe*> > pipes_l0_l1;
  vector< vector<Pipe*> > pipes_l1_ns;
  vector< vector<Pipe*> > pipes_l0_ns;
  vector< vector<Queue*> > queues_l0_l1;
  vector< vector<Queue*> > queues_l1_ns;
  vector< vector<Queue*> > queues_l0_ns;

  Pipe* get_downlink(int l0, int gpu) {return pipes_l0_ns[l0][gpu];} // for Util monitoring

  vector< vector<Pipe*> > pipes_l1_l0;
  vector< vector<Pipe*> > pipes_ns_l1;
  vector< vector<Pipe*> > pipes_ns_l0;
  vector< vector<Queue*> > queues_l1_l0;
  vector< vector<Queue*> > queues_ns_l1;
  vector< vector<Queue*> > queues_ns_l0;
  
  FirstFit* ff;
  Logfile* logfile;
  EventList* eventlist;
  int failed_links;
  queue_type qt;

  ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log, EventList* ev, FirstFit* f, queue_type q);
  ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log, EventList* ev, FirstFit* f, queue_type q, int fail);
  ZCubeTopology(int no_of_nodes, mem_b queuesize, Logfile* log, EventList* ev, FirstFit* f, queue_type q, uint32_t speed);

  void init_network();
  virtual vector<const Route*>* get_paths(int src, int dest);
  Route* route_for_switch_path(int src, int dst, const std::vector<int> &sw) override;

  Queue* alloc_src_queue(QueueLogger* q);
  Queue* alloc_src_queue(QueueLogger* q, uint64_t speed); 
  Queue* alloc_queue(QueueLogger* q, mem_b queuesize);
  Queue* alloc_queue(QueueLogger* q, uint64_t speed, mem_b queuesize);

  void count_queue(Queue*);
  void print_path(std::ofstream& paths,int src,const Route* route);
  vector<int>* get_neighbours(int src) { return NULL;};
  int no_of_nodes() const {return _no_of_nodes;}
  // int get_k() const {return K;}
  int get_n() const {return n;}
  // ForestColl schedule — added by apply_zcube_patches.py
  void load_fc_schedule();
  const std::vector<ZCubeFCTransmission>& fc_rs_step(int step) const;
  const std::vector<ZCubeFCTransmission>& fc_ag_step(int step) const;
  int fc_num_steps()        const;
  int fc_rs_total_chunks()  const;
  int fc_ag_total_chunks()  const;
 private:
  map<Queue*,int> _link_usage;
  /*
  int find_lp_switch(Queue* queue);
  int find_up_switch(Queue* queue);
  int find_core_switch(Queue* queue);
  */
  int find_l0_switch(Queue* queue);
  int find_l1_switch(Queue* queue);

  int find_destination(Queue* queue);
  void set_params(int no_of_nodes);
  // int K, NK, NC, NSRV;
  int n, k, NL0, NL1, NSRV; 
  uint32_t link_speed=0;
  int _no_of_nodes;
  // ForestColl schedule data — added by apply_zcube_patches.py
  std::vector<std::vector<ZCubeFCTransmission>> _fc_rs_steps;
  std::vector<std::vector<ZCubeFCTransmission>> _fc_ag_steps;
  int _fc_num_steps       = 0;
  int _fc_rs_total_chunks = 0;
  int _fc_ag_total_chunks = 0;
  mem_b _queuesize;
};

// class UtilMonitor : public EventSource {
//  public:

//     UtilMonitor(FatTreeTopology* top, EventList &eventlist);

//     void start(simtime_picosec period);
//     void doNextEvent();
//     void printAggUtil();

//     FatTreeTopology* _top;
//     simtime_picosec _period; // picoseconds between utilization reports
//     int64_t _max_agg_Bps; // delivered to endhosts, across the whole network
//     int64_t _max_B_in_period;
//     int _H; // number of hosts
//     int _N; // number of racks
//     int _hpr; // number of hosts / rack
// };

#endif