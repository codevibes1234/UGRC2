// main_tcp_circulant128_tg.cpp
// ============================
// htsim main for Circulant C(128,{1,3,7,17}) BFB AllReduce — TopoOpt baseline.

#include <iostream>
#include <string>
#include "../main.h"
#include "../eventlist.h"
#include "../logfile.h"
#include "../tcp.h"
#include "../dctcp.h"
#include "circulant128_topology.h"
#include "ffapp_bfb_circulant128_tg.h"
#include "../ffapp.h"

int main(int argc, char* argv[])
{
    linkspeed_bps speed    = 10000000000ULL;
    mem_b         queuesize = 100 * Packet::data_packet_size();
    int           ssthresh  = 10000;
    simtime_picosec endtime = timeFromSec(3600.1);
    string        flowfile, ofile = "output.txt";
    string        schedfile = "circulant128_schedule.json";
    double        rto_ms    = 100.0;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-speed"))    speed    = atoll(argv[++i]) * 1000000ULL;
        else if (!strcmp(argv[i], "-q"))        queuesize = atoi(argv[++i]) * Packet::data_packet_size();
        else if (!strcmp(argv[i], "-ssthresh")) ssthresh = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-simtime"))  endtime  = timeFromSec(atof(argv[++i]));
        else if (!strcmp(argv[i], "-flowfile")) flowfile = argv[++i];
        else if (!strcmp(argv[i], "-ofile"))    ofile    = argv[++i];
        else if (!strcmp(argv[i], "-sched"))    schedfile = argv[++i];
        else if (!strcmp(argv[i], "-rto"))      rto_ms   = atof(argv[++i]);
    }

    std::cerr << "[Circulant128-TG] flowfile=" << flowfile << " speed=" << speed/1000000 << "Mbps\n";

    EventList eventlist;
    eventlist.setEndtime(endtime);
    ofstream* fstream = new ofstream(ofile + ".flows");
    TcpRtxTimerScanner tcpRtxScanner(timeFromMs(rto_ms), eventlist);

    Circulant128Topology* topology = new Circulant128Topology();
    topology->load_schedule(schedfile);
    topology->init_network(speed, queuesize, eventlist, nullptr);

    FFApplication* app = new FFApplication(
        topology, ssthresh, fstream, tcpRtxScanner, eventlist,
        FFApplication::FF_BFB_CIRCULANT128_AR);
    app->bfb_rto_ms = rto_ms;
    app->load_taskgraph_flatbuf(flowfile);
    app->start_init_tasks();

    eventlist.run();
    std::cerr << "FinalFinish " << app->final_finish_time << "\n";
    return 0;
}
