// graph.hpp

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "edge.hpp"
#include "dijkstra_result.hpp"

class Graph
{
public:
    Graph(const std::string &dataset, const std::string &instance_id);
    Graph(int num_nodes);

    int add_edge(int from, int to, double weight, double capacity);

    const std::vector<int> &get_in_edges_ids(int node_id) const;
    const std::vector<int> &get_out_edges_ids(int node_id) const;

    const Edge &get_edge(int edge_id) const;

    int get_in_degree(int node_id) const;
    int get_out_degree(int node_id) const;
    int get_degree(int node_id) const;

    double get_in_capacity(int node_id) const;
    double get_out_capacity(int node_id) const;
    double get_total_capacity() const;

    int num_edges() const { return static_cast<int>(all_edges_.size()); }
    int num_nodes() const { return static_cast<int>(in_edges_.size()); }

    friend std::ostream &operator<<(std::ostream &os, const Graph &g)
    {
        for (Edge edge : g.all_edges_)
            os << edge << std::endl;

        return os;
    }

    int get_edge_id(int u, int v) const
    {
        for (int edge_id : out_edges_[u])
            if (all_edges_[edge_id].to == v)
                return edge_id;

        return -1;
    }

    double get_capacity_ratio(int node_id) const;

    DijkstraResult compute_dijkstra(int source_node) const;

private:
    std::vector<std::vector<int>> in_edges_;
    std::vector<std::vector<int>> out_edges_;
    std::vector<Edge> all_edges_;
};
