/**
 * @file traffic_data.cpp
 */

#include "common/types.hpp"
#include "instance/traffic/traffic_data.hpp"

#include <cmath>
#include <omp.h>

TrafficData::TrafficData(TickCount num_slots,
                         const std::vector<DemandBase> &demands,
                         const std::vector<Capacity> &volumes)
    : info_(demands),
      all_volumes_(volumes),
      num_time_slots_(num_slots)
{
    compute_all_norms_();
}

void TrafficData::compute_all_norms_()
{
    DemandCount n = info_.size();
#pragma omp parallel for schedule(static)

    for (DemandId i = 0; i < n; ++i)
    {
        double sum_sq = 0;
        size_t offset = i * num_time_slots_;

        for (Tick t = 0; t < num_time_slots_; ++t)
        {
            double v = all_volumes_[offset + t];
            sum_sq += v * v;
        }

        info_[i].n2 = std::sqrt(sum_sq);
    }
}