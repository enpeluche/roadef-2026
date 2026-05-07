// graph.cpp

#include "graph.hpp"

int Graph::add_edge(int from, int to, double weight, double capacity)
{
    int id = static_cast<int>(all_edges_.size());

    all_edges_.push_back({from, to, id, weight, capacity});

    in_edges_[to].push_back(id);
    out_edges_[from].push_back(id);

    return id;
}

const std::vector<int> &Graph::get_in_edges_ids(int node_id) const
{
    return in_edges_[node_id];
}

const std::vector<int> &Graph::get_out_edges_ids(int node_id) const
{
    return out_edges_[node_id];
}

const Edge &Graph::get_edge(int edge_id) const
{
    return all_edges_[edge_id];
}

int Graph::get_in_degree(int node_id) const
{
    return in_edges_[node_id].size();
}

int Graph::get_out_degree(int node_id) const
{
    return out_edges_[node_id].size();
}

int Graph::get_degree(int node_id) const
{
    return get_in_degree(node_id) + get_out_degree(node_id);
}

double Graph::get_in_capacity(int node_id) const
{
    const std::vector<int> &in = get_in_edges_ids(node_id);

    double sum = 0;

    for (int id : in)
        sum += get_edge(id).capacity;

    return sum;
}

double Graph::get_out_capacity(int node_id) const
{
    const std::vector<int> &out = get_out_edges_ids(node_id);

    double sum = 0;

    for (int id : out)
        sum += get_edge(id).capacity;

    return sum;
}

double Graph::get_total_capacity() const
{
    double total_capacity = 0;

    for (const auto &edge : all_edges_)
        total_capacity += edge.capacity;

    return total_capacity;
}

double Graph::get_capacity_ratio(int node_id) const
{
    double in = get_in_capacity(node_id);
    double out = get_out_capacity(node_id);

    if (out == 0)
        return in;
    return in / out;
}

Graph::Graph(int num_nodes)
{
    in_edges_.assign(num_nodes, std::vector<int>());
    out_edges_.assign(num_nodes, std::vector<int>());
}