#ifndef EXPT_OPTIONS_H
#define EXPT_OPTIONS_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "config.h"
#include "tcp.h"
#include "rocev2.h"
#include "queue_lossless_output.h"
#include "queue_lossless_input.h"
#include "ecmp_hash.h"

struct ExptOptions {
    double min_rto_us = 0;
    int dcqcn = 0;
    long roce_credits = 0;
    double dcqcn_k_pkts = 10;
    int dcqcn_window = 0;
};

inline ExptOptions& expt_options()
{
    static ExptOptions o;
    return o;
}

inline bool expt_parse_option(int& i, int argc, char** argv)
{
    if (i + 1 >= argc) return false;
    ExptOptions& o = expt_options();
    if (!strcmp(argv[i], "-minrtous")) {
        o.min_rto_us = atof(argv[++i]);
        TcpSrc::min_rto = timeFromUs(o.min_rto_us);
        return true;
    }
    if (!strcmp(argv[i], "-ecmpseed")) { ecmp_salt = strtoull(argv[++i], nullptr, 10); return true; }
    if (!strcmp(argv[i], "-ecmpflow")) { ecmp_per_flow = atoi(argv[++i]) != 0; return true; }
    if (!strcmp(argv[i], "-dcqcn")) { o.dcqcn = atoi(argv[++i]); return true; }
    if (!strcmp(argv[i], "-rocecredits")) { o.roce_credits = atol(argv[++i]); return true; }
    if (!strcmp(argv[i], "-dcqcnk")) { o.dcqcn_k_pkts = atof(argv[++i]); return true; }
    if (!strcmp(argv[i], "-dcqcnwin")) { o.dcqcn_window = atoi(argv[++i]); return true; }
    if (!strcmp(argv[i], "-quiet")) {
        bool q = atoi(argv[++i]) != 0;
        LosslessOutputQueue::print_overflow = !q;
        LosslessInputQueue::print_overflow = !q;
        return true;
    }
    if (!strcmp(argv[i], "-creditprio")) { LosslessOutputQueue::credit_priority = atoi(argv[++i]) != 0; return true; }
    return false;
}

inline simtime_picosec expt_scan_period(simtime_picosec base)
{
    const ExptOptions& o = expt_options();
    if (o.min_rto_us > 0) {
        simtime_picosec s = timeFromUs(o.min_rto_us / 4.0);
        if (s < base) return s;
    }
    return base;
}

template <typename T>
inline void expt_apply_roce(T& credits, uint32_t speed_mbps)
{
#ifndef USE_DCTCP
    const ExptOptions& o = expt_options();
    if (o.dcqcn) {
        RoCEv2Src::dcqcn_enable((double)speed_mbps * 1e6);
        LosslessOutputQueue::default_ecn_K = memFromPkt(o.dcqcn_k_pkts);
        if (!o.dcqcn_window) credits = 1000000;
        std::atexit([] { std::cerr << "DCQCN_cnps " << RoCEv2Src::dcqcn_cnps << "\n"; });
    }
    if (o.roce_credits > 0) credits = (T)o.roce_credits;
    if (o.dcqcn || o.roce_credits > 0)
        std::cerr << "  RoCE window  : " << credits << (o.dcqcn ? " packets, DCQCN on" : " packets") << "\n";
#else
    (void)credits;
    (void)speed_mbps;
#endif
}

#endif
