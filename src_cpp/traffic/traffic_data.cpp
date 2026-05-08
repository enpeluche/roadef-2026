// traffic/traffic_data.cpp

#include "traffic_data.hpp"

#include <cmath>
#include <omp.h>

void TrafficData::compute_all_norms_()
{
    uint16_t n = info_.size();
#pragma omp parallel for schedule(static)
    for (uint16_t i = 0; i < n; ++i)
    {
        double sum = 0, sum_sq = 0, max_v = 0;
        uint16_t offset = i * num_time_slots_;

        for (uint16_t t = 0; t < num_time_slots_; ++t)
        {
            double v = all_volumes_[offset + t];
            sum += v;
            sum_sq += v * v;
            if (v > max_v)
                max_v = v;
        }
        info_[i].n1 = sum;
        info_[i].n2 = std::sqrt(sum_sq);
        info_[i].ninf = max_v;
    }
}