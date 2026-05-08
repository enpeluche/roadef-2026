// graph.cpp

#include "graph.hpp"

double Graph::node_ingress_cap(uint16_t node_id) const
{
    const std::vector<uint16_t> &in = incoming_edges_ids(node_id);

    double sum = 0;

    for (uint16_t id : in)
        sum += edge(id).capacity;

    return sum;
}

double Graph::node_egress_cap(uint16_t node_id) const
{
    const std::vector<uint16_t> &out = outgoing_ids(node_id);

    double sum = 0;

    for (uint16_t id : out)
        sum += edge(id).capacity;

    return sum;
}

double Graph::total_cap() const
{
    double total_capacity = 0;

    for (const auto &edge : all_edges_)
        total_capacity += edge.capacity;

    return total_capacity;
}

double Graph::node_pressure_index(uint16_t node_id) const
{
    double in = node_ingress_cap(node_id);
    double out = node_egress_cap(node_id);

    if (out == 0)
        return in;
    return in / out;
}