// graph/graph.hpp
// clang-format off
#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "edge.hpp"
#include "shortest_path_tree.hpp"

// remplacer edge par link ?

class Graph
{
public:
    static constexpr uint32_t INVALID_EDGE = 0xFFFFFFFF;
    
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
        const auto& out = out_edges_[u];

        auto it = std::lower_bound(out.begin(), out.end(), v, 
            [this](uint32_t edge_id, uint16_t target_to_find) {
                return all_edges_[edge_id].target_node < target_to_find;
            }
        );

        if (it != out.end() && all_edges_[*it].target_node == v)
            return *it;

        return INVALID_EDGE;
    }

    const Edge &edge(uint16_t edge_id) const { return all_edges_[edge_id]; }

    const std::vector<uint16_t> &incoming_edges_ids(uint16_t node_id) const { return in_edges_[node_id]; } 
    const std::vector<uint16_t> &outgoing_ids(uint16_t node_id) const { return out_edges_[node_id]; } 

    const std::string& node_name(uint16_t id) const { return node_names_[id]; }

    uint16_t in_degree(uint16_t node_id) const { return in_edges_[node_id].size(); }
    uint16_t out_degree(uint16_t node_id) const { return out_edges_[node_id].size(); }
    uint16_t degree(uint16_t node_id) const { return in_degree(node_id) + out_degree(node_id); }

    double node_ingress_cap(uint16_t node_id) const;
    double node_egress_cap(uint16_t node_id) const;

    double node_pressure_index(uint16_t node_id) const;

    double total_cap() const;

    uint16_t edges_count() const { return static_cast<uint16_t>(all_edges_.size()); }
    uint16_t nodes_count() const { return node_count_; }

    ShortestPathTree shortest_path_tree(uint16_t source_node) const;

private:
    const uint16_t node_count_;

    std::vector<Edge> all_edges_;

    std::vector<std::vector<uint16_t>> in_edges_;
    std::vector<std::vector<uint16_t>> out_edges_;

    std::vector<std::string> node_names_;
};
