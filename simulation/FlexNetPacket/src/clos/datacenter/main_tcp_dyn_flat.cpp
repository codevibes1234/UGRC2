// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
#include "config.h"
#include <sstream>
#include <strstream>
#include <fstream> // need to read flows
#include <iostream>
#include <string.h>
#include <math.h>
#include "network.h"
#include "randomqueue.h"
//#include "shortflows.h"
#include "pipe.h"
#include "eventlist.h"
#include "logfile.h"
#include "loggers.h"
#include "clock.h"
#include "tcp.h"
#include "mtcp.h"
#include "compositequeue.h"
#include "firstfit.h"
#include "topology.h"
#include "dyn_net_sch.h"
#include "topoopt_builder.h"
#include "queue_lossless_input.h"
#include "queue_lossless_output.h"
//#include "connection_matrix.h"

// Choose the topology here:
// #include "test_topology.h"
#include "flat_topology.h"
#include "ffapp.h"

#include <list>

// Simulation params

#define PRINT_PATHS 0

#define PERIODIC 0
#include "main.h"

uint32_t RTT = 1000; // ns
int DEFAULT_NODES = 16;

uint32_t SPEED;
std::ofstream fct_util_out;

FirstFit *ff = NULL;
//unsigned int subflow_count = 8; // probably not necessary ???

#define DEFAULT_PACKET_SIZE 9000 // full packet (including header), Bytes
#define DEFAULT_HEADER_SIZE 64   // header size, Bytes
#define DEFAULT_QUEUE_SIZE 100
#define DEFAULT_DEGREE 4
#define DEFAULT_RECONF_DELAY 100

string ntoa(double n);
string itoa(uint64_t n);

EventList eventlist;
// Logfile* lg;

void exit_error(char *progr, char *param)
{
  cerr << "Bad parameter: " << param << endl;
  cerr << "Usage " << progr << " [UNCOUPLED(DEFAULT)|COUPLED_INC|FULLY_COUPLED|COUPLED_EPSILON] [epsilon][COUPLED_SCALABLE_TCP]" << endl;
  exit(1);
}

void print_path(std::ofstream &paths, const Route *rt)
{
  for (unsigned int i = 1; i < rt->size() - 1; i += 2)
  {
    RandomQueue *q = (RandomQueue *)rt->at(i);
    if (q != NULL)
      paths << q->str() << " ";
    else
      paths << "NULL ";
  }

  paths << endl;
}

int main(int argc, char **argv)
{

  TcpPacket::set_packet_size(DEFAULT_PACKET_SIZE - DEFAULT_HEADER_SIZE); // MTU
  mem_b queuesize = DEFAULT_QUEUE_SIZE * DEFAULT_PACKET_SIZE;

  int algo = UNCOUPLED;
  double epsilon = 1;
  int ssthresh = 15;
  int degree = DEFAULT_DEGREE;
  uint32_t reconf_delay = DEFAULT_RECONF_DELAY;
#ifdef USE_DCTCP
  string queue_kind = "ecn";
#else
  string queue_kind = "lossless";
#endif
  string opt_method = "sipring";
  unsigned dyn_seed = 1;
  bool allow_inert_pause = false;
  string routing_kind = "updown";
  string logical_family = "";
  double pfc_xoff = 15, pfc_xon = 12, pfc_buf = -1;
  long long pfc_delay_ns = 0;
  double rtx_scan_ms = 1000;
  int pfc_classes = 1;
  long long rdelay_ns = -1, rperiod_ns = 0;

  int no_of_nodes = DEFAULT_NODES;

  string flowfile;       // so we can read the flows from a specified file
  double simtime;        // seconds
  double utiltime = .01; // seconds

  // stringstream filename(ios_base::out);
  int i = 1;
  // filename << "logout.dat";

  while (i < argc)
  {
    //   if (!strcmp(argv[i],"-o")){
    //       filename.str(std::string());
    //       filename << argv[i+1];
    //       i++;
    //   } else
    if (!strcmp(argv[i], "-nodes"))
    {
      no_of_nodes = atoi(argv[i + 1]);
      cout << "no_of_nodes " << no_of_nodes << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-speed"))
    {
      SPEED = atoi(argv[i + 1]);
      cout << "speed " << SPEED << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-rtt"))
    {
      RTT = atoi(argv[i + 1]);
      cout << "RTT " << RTT << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-ofile"))
    {
      fct_util_out = std::ofstream(argv[i + 1]);
      cout << "ofile " << argv[i + 1] << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-ssthresh"))
    {
      ssthresh = atoi(argv[i + 1]);
      cout << "ssthresh " << ssthresh << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-q"))
    {
      queuesize = memFromPkt(atoi(argv[i + 1]));
      cout << "queuesize " << queuesize << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-rdelay"))
    {
      reconf_delay = atoi(argv[i + 1]);
      cout << "reconf_delay " << reconf_delay << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-deg"))
    {
      degree = atoi(argv[i + 1]);
      cout << "degree " << degree << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-queue"))
    {
      queue_kind = string(argv[i + 1]);
      if (queue_kind != "ecn" && queue_kind != "lossless") {
        std::cerr << "ERROR: -queue must be ecn or lossless, got '" << queue_kind << "'\n";
        exit(1);
      }
      cout << "queue " << queue_kind << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-rdelayns")) { rdelay_ns = atoll(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-rperiodns")) { rperiod_ns = atoll(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-pfcdelayns")) { pfc_delay_ns = atoll(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-ecnqexact")) { FlatTopology::ecn_queue_exact = true; }
    else if (!strcmp(argv[i], "-rtxscanms")) { rtx_scan_ms = atof(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-rocedemand")) { DemandRecorder::include_roce = atoi(argv[i + 1]) != 0; i++; }
    else if (!strcmp(argv[i], "-logicalseed")) { dyn_sch_set_logical_seed((unsigned)strtoul(argv[i + 1], nullptr, 10)); i++; }
    else if (!strcmp(argv[i], "-pfcxoff")) { pfc_xoff = atof(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-pfcxon"))  { pfc_xon  = atof(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-pfcbuf"))  { pfc_buf  = atof(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-pfcclasses")) { pfc_classes = atoi(argv[i + 1]); i++; }
    else if (!strcmp(argv[i], "-logical"))
    {
      logical_family = string(argv[i + 1]);
      if (logical_family != "jellyfish" && logical_family != "flatbutterfly" &&
          logical_family != "dragonfly" && logical_family != "megafly" &&
          logical_family != "topoopt") {
        std::cerr << "ERROR: -logical must be jellyfish, flatbutterfly, dragonfly"
                     " or megafly, got '" << logical_family << "'\n";
        exit(1);
      }
      cout << "logical " << logical_family << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-routing"))
    {
      routing_kind = string(argv[i + 1]);
      if (routing_kind != "updown" && routing_kind != "shortest") {
        std::cerr << "ERROR: -routing must be updown or shortest, got '"
                  << routing_kind << "'\n";
        exit(1);
      }
      cout << "routing " << routing_kind << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-seed"))
    {
      dyn_seed = (unsigned)strtoul(argv[i + 1], nullptr, 10);
      cout << "seed " << dyn_seed << endl;
      i++;
    }
    else if (!strcmp(argv[i], "-allow-inert-pause"))
    {
      allow_inert_pause = true;
    }
    else if (!strcmp(argv[i], "-omethod"))
    {
      opt_method = string(argv[i + 1]);
      cout << "opt_method " << opt_method << endl;
      i++;
    }
    else if (!strcmp(argv[i], "UNCOUPLED"))
      algo = UNCOUPLED;
    else if (!strcmp(argv[i], "COUPLED_INC"))
      algo = COUPLED_INC;
    else if (!strcmp(argv[i], "FULLY_COUPLED"))
      algo = FULLY_COUPLED;
    else if (!strcmp(argv[i], "COUPLED_TCP"))
      algo = COUPLED_TCP;
    else if (!strcmp(argv[i], "COUPLED_SCALABLE_TCP"))
      algo = COUPLED_SCALABLE_TCP;
    else if (!strcmp(argv[i], "COUPLED_EPSILON"))
    {
      algo = COUPLED_EPSILON;
      if (argc > i + 1)
      {
        epsilon = atof(argv[i + 1]);
        i++;
      }
      printf("Using epsilon %f\n", epsilon);
    }
    else if (!strcmp(argv[i], "-flowfile"))
    {
      flowfile = argv[i + 1];
      i++;
    }
    else if (!strcmp(argv[i], "-simtime"))
    {
      simtime = atof(argv[i + 1]);
      i++;
    }
    else if (!strcmp(argv[i], "-utiltime"))
    {
      utiltime = atof(argv[i + 1]);
      i++;
    }
    else
      exit_error(argv[0], argv[i]);
    i++;
  }
  srand(13);

  eventlist.setEndtime(timeFromSec(simtime));
  Clock c(timeFromSec(5 / 100.), eventlist);

  DynFlatScheduler::OptStrategy optstrategy;
  if (opt_method == "sipring")
    optstrategy = DynFlatScheduler::SIPML_RING;
  else if (opt_method == "sipocs")
    optstrategy = DynFlatScheduler::SIPML_OCS;
  else if (opt_method == "dheu")
    optstrategy = DynFlatScheduler::D_HEURISTIC;
  else 
    assert("unknown strategy!" && false);

#if PRINT_PATHS
  filename << ".paths";
  cout << "Logging path choices to " << filename.str() << endl;
  std::ofstream paths(filename.str().c_str());
  if (!paths)
  {
    cout << "Can't open for writing paths file!" << endl;
    exit(1);
  }
#endif

  TcpRtxTimerScanner tcpRtxScanner(timeFromMs(rtx_scan_ms), eventlist);
  std::cout << "rtx_scan_ms " << rtx_scan_ms << std::endl;
  DemandRecorder demandrecorder = DemandRecorder(degree, &tcpRtxScanner);


    {
        uint64_t rtt_ps = 12ULL * RTT * 1000ULL;
        uint64_t bdp_bytes = (uint64_t)SPEED * 1000000ULL / 8ULL * rtt_ps / 1000000000000ULL;
        uint32_t credits = (uint32_t)(2 * bdp_bytes / 9000);
        FFApplication::roce_initial_credits = credits < 8 ? 8 : credits;
    }

#ifdef USE_DCTCP
  std::cout << "transport dctcp" << std::endl;
#else
  std::cout << "transport rocev2" << std::endl;
#endif
  if (allow_inert_pause)
    std::cerr << "WARNING: -allow-inert-pause no longer does anything; both "
                 "transports support the reconfiguration pause.\n";

  dyn_sch_set_seed(dyn_seed);

  if (pfc_buf < 0) pfc_buf = pfc_xoff + 5;
  LosslessInputQueue::configure(pfc_xoff, pfc_xon, pfc_buf);
  if (pfc_classes < 1 || pfc_classes > 16) {
    std::cerr << "ERROR: -pfcclasses must be 1..16 (the class lives in 4 bits)\n";
    exit(1);
  }
  LosslessInputQueue::cfg_classes = pfc_classes;

  LosslessInputQueue::cfg_pause_delay = (simtime_picosec)pfc_delay_ns * 1000ULL;

  queue_type qt = (queue_kind == "lossless") ? LOSSLESS_INPUT : ECN;
  FlatTopology *top = new FlatTopology(no_of_nodes, queuesize, nullptr /* &logfile */, &eventlist, ff, qt);
  simtime_picosec delay_ps = (rdelay_ns >= 0) ? (simtime_picosec)rdelay_ns * 1000ULL
                                              : 10000000ULL * reconf_delay;
  simtime_picosec period_ps = (simtime_picosec)rperiod_ns * 1000ULL;
  std::cout << "reconf_delay_ps " << delay_ps << std::endl;
  std::cout << "reconf_period_ps " << (period_ps ? period_ps : 4 * delay_ps) << std::endl;
  TopoOptPlan topoopt;
  if (logical_family == "topoopt") {
    topoopt = topoopt_plan(flowfile, no_of_nodes, degree);
    ocs_external_conn() = topoopt.conn;
    std::string st;
    for (int p : topoopt.strides) st += (st.empty() ? "" : "/") + std::to_string(p);
    ocs_external_how() = "TopoOpt reconstruction: " + std::to_string(topoopt.d_ar) +
                         " AllReduce ring interface(s), strides " + st + "; " +
                         std::to_string(topoopt.d_mp) + " model-parallel matching interface(s)";
  }
  DynFlatScheduler sch = DynFlatScheduler(no_of_nodes, degree, top, optstrategy, &demandrecorder, delay_ps, eventlist, routing_kind == "shortest" ? DynFlatScheduler::ROUTING_SHORTEST : DynFlatScheduler::ROUTING_UPDOWN, logical_family, period_ps);
  // TcpSrc::demand_recorder = &sch.demandrecorder;

      // FFApplication app = FFApplication(top, ssthresh, sinkLogger, traffic_logger, tcpRtxScanner, eventlist);
  FFApplication app = FFApplication(top, ssthresh, &fct_util_out, tcpRtxScanner, eventlist);
  if (logical_family == "topoopt" && !topoopt.rings.empty()) {
    app.fancy_ring = true;
    for (auto &kv : topoopt.rings) app.selected_jumps[kv.first] = kv.second;
  }
  app.load_taskgraph_flatbuf(flowfile);
  app.start_init_tasks();

  // UtilMonitor* UM = new UtilMonitor(top, eventlist);
  // UM->start(timeFromSec(utiltime));

  // Record the setup
  int pktsize = Packet::data_packet_size();
  // logfile.write("# pktsize=" + ntoa(pktsize) + " bytes");
  //logfile.write("# subflows=" + ntoa(subflow_count));
  // logfile.write("# hostnicrate = " + ntoa(SPEED) + " pkt/sec");
  // logfile.write("# corelinkrate = " + ntoa(SPEED*CORE_TO_HOST) + " pkt/sec");
  //logfile.write("# buffer = " + ntoa((double) (queues_na_ni[0][1]->_maxsize) / ((double) pktsize)) + " pkt");
  //double rtt = timeAsSec(timeFromUs(RTT));
  //logfile.write("# rtt =" + ntoa(rtt));

  // GO!
  while (eventlist.doNextEvent())
  {
  }

  fct_util_out << "FinalFinish " << app.final_finish_time << std::endl;

  sch.print_stats(fct_util_out);
  sch.print_stats(std::cout);
  LosslessInputQueue::print_stats(fct_util_out);
  LosslessOutputQueue::print_stats(fct_util_out);
  LosslessInputQueue::print_stats(std::cout);
  LosslessOutputQueue::print_stats(std::cout);
}

string ntoa(double n)
{
  stringstream s;
  s << n;
  return s.str();
}

string itoa(uint64_t n)
{
  stringstream s;
  s << n;
  return s.str();
}
