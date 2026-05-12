// traffic/traffic_data.cpp

// clang-format off

#include "traffic_data.hpp"

#include <cmath>
#include <omp.h>

TrafficData::TrafficData(uint16_t num_slots,
                         const std::vector<DemandBase> &demands,
                         const std::vector<double> &volumes)
    : info_(demands),
      all_volumes_(volumes),
      num_time_slots_(num_slots)
{
    compute_all_norms_();
}

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