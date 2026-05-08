// traffic/traffic_data.hpp

#pragma once

#include "demand_base.hpp"

#include <vector>
#include <string>

class TrafficData
{
public:
    uint16_t demands_count() const { return static_cast<uint16_t>(info_.size()); }
    uint16_t slots_count() const { return num_time_slots_; }

    double volume(uint16_t demand_id, uint16_t slot) const { return all_volumes_[demand_id * num_time_slots_ + slot]; }

    const DemandBase &get_info(uint16_t demand_id) const { return info_[demand_id]; }

    static TrafficData load(const std::string &dataset, const std::string &instance_id);

private:
    std::vector<DemandBase> info_;
    std::vector<double> all_volumes_;
    uint16_t num_time_slots_;

    void compute_all_norms_();
};