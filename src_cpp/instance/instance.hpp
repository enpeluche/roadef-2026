// instance.hpp

#pragma once

#include "graph/core/graph.hpp"
#include "traffic/traffic_data.hpp"

#include <chrono>
#include <cstdint>
#include <string>

#include <iostream>

#include <nlohmann/json.hpp>
#include <fstream>

// renommer graph en graph, sortir de graph le timeslot, et créer un objet interventions, mettre le budget dans interventions
//  trier les includes et garder que le necessaire

struct Instance
{
    std::string name;    ///< Nom de l'instance.
    Graph graph;         ///< Graphe dirigé de l'instance.
    TrafficData traffic; ///< Données de traffic de l'instance.
    std::vector<uint8_t> budget;

    uint8_t num_time_slots_ = 0;                             ///< Nombre total de pas de temps (topologie dynamique).
    std::vector<boost::dynamic_bitset<>> topology_timeline_; ///< Masques d'activation des arcs pour chaque pas de temps.

    static Instance
    load(const std::string &dataset, const std::string &id)
    {
        std::string filepath = "instances/" + dataset + "/" + dataset + "-" + id + "-scenario.json";
        std::ifstream f(filepath);

        if (!f.is_open())
            throw std::runtime_error("Erreur ouverture : " + filepath);

        auto data = nlohmann::json::parse(f);

        // 1. On charge d'abord le graphe et le trafic pour connaître la dimension temporelle
        Graph loaded_G = Graph::from_json(dataset, id);
        TrafficData loaded_traffic = TrafficData::load(dataset, id);

        // On récupère le nombre exact de slots de l'instance
        uint8_t num_slots = loaded_G.time_slots_count();

        // 2. On initialise le budget à la BONNE taille (rempli de 0 par défaut)
        std::vector<uint8_t> tbudget(num_slots, 0);

        // 3. On lit le budget si la clé existe dans le JSON du scénario
        if (data.contains("budget"))
        {
            for (const auto &item : data["budget"])
            {
                int t = item["t"].get<int>();
                uint8_t val = item["value"].get<uint8_t>();

                // Sécurité : on vérifie qu'on ne déborde pas
                if (t >= 0 && t < num_slots)
                {
                    tbudget[t] = val;
                }
                else
                {
                    throw std::runtime_error("Erreur : le timestep du budget (" + std::to_string(t) + ") dépasse le max de l'instance (" + std::to_string(num_slots) + ")");
                }
            }
        }

        std::string instance_name = dataset + id;

        return {
            std::move(instance_name),
            std::move(loaded_G),
            std::move(loaded_traffic),
            std::move(tbudget)};
    }

    friend std::ostream &operator<<(std::ostream &os, const Instance &inst)
    {
        os << "Instance : " << inst.name << "\n\n";
        os << inst.graph << "\n\n";
        os << inst.traffic << "\n\n";

        return os;
    }

    // Topology timeline

    /**
     * @brief Récupère le masque d'activation des arcs pour un pas de temps donné.
     *
     * @param t Le pas de temps (time slot).
     * @return const boost::dynamic_bitset<>& Référence constante vers le masque binaire.
     */
    const boost::dynamic_bitset<> &timeline(uint8_t t) const { return topology_timeline_[t]; }

    /**
     * @brief Initialise la structure temporelle de la topologie du graphe.
     *
     * @param n_slots Le nombre total de pas de temps à allouer.
     */
    void init_topology(uint8_t n_slots)
    {
        num_time_slots_ = n_slots;
        topology_timeline_.assign(n_slots, boost::dynamic_bitset<>(all_edges_.size()));
    }

    /**
     * @brief Définit l'état des arcs pour un pas de temps spécifique.
     *
     * @param t Le pas de temps cible.
     * @param bits Le masque binaire d'activation des arcs (transféré par déplacement).
     */
    void set_timeline_slot(uint8_t t, boost::dynamic_bitset<> bits)
    {
        topology_timeline_[t] = std::move(bits);
    }

    uint8_t time_slots_count() const { return num_time_slots_; }

    // 5. Topology timeline (bitwise AND + iteration sur les survivants)
    // compacted_graph.num_time_slots_ = num_time_slots_;
    // compacted_graph.topology_timeline_.assign(
    //     num_time_slots_,
    //    boost::dynamic_bitset<>(kept_edges_count));

    // for (uint8_t t = 0; t < num_time_slots_; ++t)
    //  {
    //  const auto survivors = topology_timeline_[t] & keep_edge;
    //  auto &new_tl = compacted_graph.topology_timeline_[t];

    //  for (size_t e = survivors.find_first();
    //          e != boost::dynamic_bitset<>::npos;
    //              e = survivors.find_next(e))
    //  {
    //     new_tl.set(edge_map[e]);
    // }
    //  }

    // Mer de bits
    // g.num_time_slots_ = tm_data["num_time_slots"];
    // g.topology_timeline_.assign(g.num_time_slots_, boost::dynamic_bitset<>(g.all_edges_.size()));
    // for (int t = 0; t < g.num_time_slots_; ++t)
    //    g.topology_timeline_[t].set();

    // if (scenario_data.contains("interventions"))
    //     for (const auto &inter : scenario_data["interventions"])
    //    {
    //        int t = inter["t"].get<int>();
    //        for (int link_id : inter["links"])
    //            g.topology_timeline_[t].reset(link_id);
    //    }
};