#ifndef FC_SCHEDULE_H
#define FC_SCHEDULE_H

#include <vector>

struct ZCubeFCTransmission { int sender, receiver, chunks; };

class FCScheduleSource {
 public:
    virtual ~FCScheduleSource() {}

    virtual const std::vector<ZCubeFCTransmission>& fc_rs_step(int step) const = 0;
    virtual const std::vector<ZCubeFCTransmission>& fc_ag_step(int step) const = 0;
    virtual int fc_num_steps()       const = 0;
    virtual int fc_rs_total_chunks() const = 0;
    virtual int fc_ag_total_chunks() const = 0;

    virtual bool fc_has_paths() const { return false; }
    virtual const std::vector<int>& fc_rs_path(int  , int  ) const
    { static const std::vector<int> none; return none; }
    virtual const std::vector<int>& fc_ag_path(int  , int  ) const
    { static const std::vector<int> none; return none; }
};

class Route;

class ExplicitRouteTopology {
 public:
    virtual ~ExplicitRouteTopology() {}
    virtual Route* route_for_switch_path(int src, int dst,
                                         const std::vector<int> &sw) = 0;
};

#endif
