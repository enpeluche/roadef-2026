// traffic/traffic_io.cpp
#include <nlohmann/json.hpp>
#include "traffic_data.hpp"
#include <fstream>

TrafficData TrafficData::load(const std::string &dataset, const std::string &instance_id)
{
    TrafficData td;
    std::string filepath = "instances/" + dataset + "/" + dataset + "-" + instance_id + "-tm.json";
    std::ifstream f(filepath);

    if (!f.is_open())
        throw std::runtime_error("Erreur ouverture : " + filepath);

    auto data = nlohmann::json::parse(f);
    td.num_time_slots_ = data.at("num_time_slots").get<uint16_t>();

    const auto &json_demands = data.at("demands");

    auto n_demands = json_demands.size();
    td.info_.reserve(n_demands);
    td.all_volumes_.resize(n_demands * td.num_time_slots_);

    for (uint16_t i = 0; i < n_demands; ++i)
    {
        const auto &item = json_demands[i];
        DemandBase d;
        d.id = i;
        d.source = item.at("s").get<uint16_t>();
        d.target = item.at("t").get<uint16_t>();

        const auto &v = item.at("v");

        double *dest = td.all_volumes_.data() + i * td.num_time_slots_;

        for (uint16_t t = 0; t < td.num_time_slots_; ++t)
            dest[t] = v[t].get<double>();

        td.info_.push_back(d);
    }

    td.compute_all_norms_();
    return td;
}