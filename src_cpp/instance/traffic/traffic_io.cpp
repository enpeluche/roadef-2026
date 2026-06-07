/**
 * @file traffic_io.cpp
 */
#include <fstream>
#include <nlohmann/json.hpp>
#include "instance/traffic/traffic_data.hpp"

TrafficData TrafficData::load(const std::string &dataset, const std::string &instance_id)
{
    TrafficData td;

    const std::string filepath =
        "instances/" + dataset + "/" + dataset + "-" + instance_id + "-tm.json";

    std::ifstream f(filepath);
    if (!f.is_open())
        throw std::runtime_error("Failed to open traffic file: " + filepath);

    auto data = nlohmann::json::parse(f);

    td.num_time_slots_ = data.at("num_time_slots").get<TickCount>();

    const auto &json_demands = data.at("demands");
    const DemandCount n_demands = static_cast<DemandCount>(json_demands.size());

    // Both containers sized up front and filled by index -> the invariant
    // all_volumes_.size() == info_.size() * num_time_slots_ holds by construction.
    td.info_.resize(n_demands);
    td.all_volumes_.resize(static_cast<size_t>(n_demands) * td.num_time_slots_);

    for (DemandId i = 0; i < n_demands; ++i)
    {
        const auto &item = json_demands[i];

        DemandBase &d = td.info_[i];
        d.id = i;
        d.source = item.at("s").get<NodeId>();
        d.target = item.at("t").get<NodeId>();
        // d.n2 is filled by compute_all_norms_() below.

        const auto &v = item.at("v");
        const size_t offset = static_cast<size_t>(i) * td.num_time_slots_;
        for (Tick t = 0; t < td.num_time_slots_; ++t)
            td.all_volumes_[offset + t] = v[t].get<Capacity>();
    }

    td.compute_all_norms_();
    return td;
}