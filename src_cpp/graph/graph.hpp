// graph/graph.hpp

#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "edge.hpp"
#include "shortest_path_tree.hpp"
#include <boost/dynamic_bitset.hpp>

// remplacer les mots edge par les mots link ?
// virer les vector de vector

class Graph
{
public:
    static constexpr uint32_t INVALID_EDGE = 0xFFFFFFFF;

    Graph() = default;

    Graph(uint16_t nodes_count) : node_count_(nodes_count), in_edges_(nodes_count), out_edges_(nodes_count), node_names_(nodes_count) {}

    // io
    static Graph from_json(const std::string &dataset, const std::string &instance_id);
    void to_json(const std::string &dataset, const std::string &instance_id) const;

    uint16_t add_edge(uint16_t source_node, uint16_t target_node, double weight, double capacity)
    {
        uint16_t id = static_cast<uint16_t>(all_edges_.size());

        all_edges_.push_back({id, source_node, target_node, weight, capacity});

        in_edges_[target_node].push_back(id);
        out_edges_[source_node].push_back(id);

        return id;
    }

    uint32_t edge_id(uint16_t u, uint16_t v) const
    {
        const auto &out = out_edges_[u];

        auto it = std::lower_bound(out.begin(), out.end(), v,
                                   [this](uint32_t edge_id, uint16_t target_to_find)
                                   {
                                       return all_edges_[edge_id].target_node < target_to_find;
                                   });

        if (it != out.end() && all_edges_[*it].target_node == v)
            return *it;

        return INVALID_EDGE;
    }

    const Edge &edge(uint16_t edge_id) const { return all_edges_[edge_id]; }

    const std::vector<uint16_t> &incoming_edges_ids(uint16_t node_id) const { return in_edges_[node_id]; }
    const std::vector<uint16_t> &outgoing_ids(uint16_t node_id) const { return out_edges_[node_id]; }

    const std::string &node_name(uint16_t id) const { return node_names_[id]; }

    uint16_t in_degree(uint16_t node_id) const { return in_edges_[node_id].size(); }
    uint16_t out_degree(uint16_t node_id) const { return out_edges_[node_id].size(); }
    uint16_t degree(uint16_t node_id) const { return in_degree(node_id) + out_degree(node_id); }

    double node_ingress_cap(uint16_t node_id) const;
    double node_egress_cap(uint16_t node_id) const;

    double node_pressure_index(uint16_t node_id) const;

    double total_cap() const;

    uint16_t edges_count() const { return static_cast<uint16_t>(all_edges_.size()); }
    uint16_t nodes_count() const { return node_count_; }
    uint8_t num_time_slots() const { return num_time_slots_; }

    ShortestPathTree shortest_path_tree(uint16_t source_node, uint8_t t) const;

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph);

    /**
     * @brief Calcule la densité du graphe.
     * Formule : E / (V * (V - 1)) pour un graphe orienté.
     */
    double density() const
    {
        uint16_t n = nodes_count();
        if (n <= 1)
            return 0.0;

        double max_edges = static_cast<double>(n) * (n - 1);
        return static_cast<double>(edges_count()) / max_edges;
    }

    boost::dynamic_bitset<> get_timeline(uint8_t t) const { return topology_timeline_[t]; }

private:
    const uint16_t node_count_;                              ///< Le nombre de noeud du graphe.
    std::vector<Edge> all_edges_;                            ///< Le vecteur des Edge du graphe.
    std::vector<std::vector<uint16_t>> in_edges_;            ///< Un vecteur qui contient pour chaque index le vecteur des indices des Edge entrant.
    std::vector<std::vector<uint16_t>> out_edges_;           ///< Un vecteur qui contient pour chaque index le vecteur des indices des Edge sortant.
    std::vector<std::string> node_names_;                    ///< Le vecteur du nom des noeuds.
    std::vector<boost::dynamic_bitset<>> topology_timeline_; ///< Mer de bit représentant la disponibilité d'un arc à un timestep.
    uint8_t num_time_slots_;                                 ///< Nombre de timestep
};
