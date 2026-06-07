/**
 * @file traffic_data.hpp
 * Usage: #include "instance/traffic/traffic_data.hpp"
 */

#pragma once

#include "common/types.hpp"
#include "instance/traffic/demand_base.hpp"

#include <vector>
#include <string>

/**
 * @brief Read-only container for network traffic demands and their time-series volumes.
 * * Invariant: all_volumes_.size() == info_.size() * num_time_slots_
 */
class TrafficData
{
public:
    /**
     * @brief Initializes traffic data and precomputes L2 norms.
     * @pre volumes.size() must equal demands.size() * num_slots.
     */
    TrafficData(TickCount num_slots,
                const std::vector<DemandBase> &demands,
                const std::vector<double> &volumes);

    TrafficData() = default;

    uint16_t demands_count() const { return static_cast<uint16_t>(info_.size()); }

    TickCount slots_count() const { return num_time_slots_; }

    /**
     * @pre demand_id < demands_count()
     * @pre t < slots_count()
     */
    double volume(DemandId id, Tick t) const
    {
        return all_volumes_[id * num_time_slots_ + t];
    }

    /**
     * @pre demand_id < demands_count()
     */
    const DemandBase &get_info(DemandId id) const { return info_[id]; }

    /**
     * @brief Constructs TrafficData by reading a JSON instance file.
     * @param dataset Directory name inside the "instances/" relative path.
     * @param instance_id Identifier used to form the "-tm.json" filename.
     * @throws std::runtime_error if the file cannot be opened.
     * @throws nlohmann::json::exception on JSON parsing or schema validation errors.
     */
    static TrafficData load(const std::string &dataset, const std::string &instance_id);

    friend std::ostream &operator<<(std::ostream &os, const TrafficData &td);

private:
    std::vector<DemandBase> info_;
    std::vector<Capacity> all_volumes_; ///< Flattened 2D array [demand_id][time_slot].
    TickCount num_time_slots_;

    /**
     * @brief Computes and stores the L2 norm for each demand in info_.
     */
    void compute_all_norms_();
};