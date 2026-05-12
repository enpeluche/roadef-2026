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

std::ostream &operator<<(std::ostream &os, const TrafficData &td)
{
    os << "========== TRAFFIC DATA SUMMARY ==========\n";
    os << " Demands     : " << td.demands_count() << "\n";
    os << " Time slots  : " << td.slots_count() << "\n\n";

    // En-tête du tableau Markdown
    os << "| DEMAND_ID | SOURCE | DESTINATION | VOLUMES\n";
    os << "|-----------|--------|-------------|--------\n";

    // On limite l'affichage aux 10 premières demandes
    uint16_t demands_limit = std::min<uint16_t>(10, td.demands_count());

    // On limite l'affichage des volumes pour ne pas casser l'écran
    uint16_t slots_limit = std::min<uint16_t>(7, td.slots_count());

    for (uint16_t i = 0; i < demands_limit; ++i)
    {
        const auto &demand = td.get_info(i);

        os << "| " << std::setw(9) << demand.id
           << " | " << std::setw(6) << demand.source
           << " | " << std::setw(11) << demand.target
           << " | [";

        // Affichage des volumes
        for (uint16_t t = 0; t < slots_limit; ++t)
            os << td.volume(i, t) << (t == slots_limit - 1 ? "" : ", ");

        // Si on a tronqué les time slots, on l'indique
        if (td.slots_count() > slots_limit)
            os << ", ... (" << td.slots_count() - slots_limit << " more)";

        os << "]\n";
    }

    // Si on a tronqué les demandes, on l'indique
    if (td.demands_count() > demands_limit)
        os << "| ...       | ...    | ...         | ...\n";

    return os;
}