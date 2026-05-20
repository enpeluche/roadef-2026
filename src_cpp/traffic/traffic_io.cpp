#include <fstream>
#include <nlohmann/json.hpp>
#include "traffic/traffic_data.hpp"

TrafficData TrafficData::load(const std::string &dataset, const std::string &instance_id)
{
    TrafficData td;
    std::string filepath = "instances/" + dataset + "/" + dataset + "-" + instance_id + "-tm.json";
    std::ifstream f(filepath);

    if (!f.is_open())
        throw std::runtime_error("Erreur ouverture : " + filepath);

    auto data = nlohmann::json::parse(f);
    td.num_time_slots_ = data.at("num_time_slots").get<TickCount>();

    const auto &json_demands = data.at("demands");

    auto n_demands = json_demands.size();
    td.info_.reserve(n_demands);
    td.all_volumes_.resize(n_demands * td.num_time_slots_);

    for (uint16_t i = 0; i < n_demands; ++i)
    {
        const auto &item = json_demands[i];
        DemandBase d;
        d.id = i;
        d.source = item.at("s").get<NodeCount>();
        d.target = item.at("t").get<NodeCount>();

        const auto &v = item.at("v");

        double *dest = td.all_volumes_.data() + i * td.num_time_slots_;

        for (Tick t = 0; t < td.num_time_slots_; ++t)
            dest[t] = v[t].get<double>();

        td.info_.push_back(d);
    }

    td.compute_all_norms_();
    return td;
}

std::ostream &operator<<(std::ostream &os, const TrafficData &td)
{
    constexpr uint16_t DEFAULT_LIMIT = 5;

    const uint16_t n_demands = td.demands_count();
    const TickCount n_slots = td.slots_count();
    const uint16_t limit = std::min(DEFAULT_LIMIT, n_demands);

    os << "Traffic: " << n_demands << " demands, " << n_slots << " slots\n\n";

    os << "| ID | SRC | TGT | VOLS\n";
    os << "|----|-----|-----|-----\n";

    for (uint16_t i = 0; i < limit; ++i)
    {
        const auto &d = td.get_info(i);
        os << "| " << d.id << " | " << d.source << " | " << d.target << " | [";
        for (Tick t = 0; t < n_slots; ++t)
            os << td.volume(i, t) << (t + 1 < n_slots ? ", " : "");
        os << "]\n";
    }

    if (n_demands > limit)
        os << "| ... (" << (n_demands - limit) << " more)\n";

    return os;
}