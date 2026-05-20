#include "graph/core/graph.hpp"

Graph Graph::compacted(const boost::dynamic_bitset<> &keep_node,
                       const boost::dynamic_bitset<> &keep_edge,
                       std::vector<uint16_t> &node_map,
                       std::vector<uint16_t> &edge_map,
                       const std::vector<double> &edge_new_capacity) const
{
    assert(frozen_ && "compacted() called on unfrozen graph");
    constexpr uint16_t REMOVED = std::numeric_limits<uint16_t>::max();
    const uint16_t num_nodes = nodes_count();
    const uint16_t num_edges = edges_count();

    assert(keep_node.size() == num_nodes && "keep_node size mismatch");
    assert(keep_edge.size() == num_edges && "keep_edge size mismatch");

    // 1. Remap nœuds (O(n))
    node_map.assign(num_nodes, REMOVED);
    uint16_t next_node_id = 0;
    for (size_t v = keep_node.find_first();
         v != boost::dynamic_bitset<>::npos;
         v = keep_node.find_next(v))
    {
        node_map[v] = next_node_id++;
    }

    // 2. Préallocation (popcount sur le bitset, O(m/64))
    const uint16_t kept_edges_count = static_cast<uint16_t>(keep_edge.count());

    // 3. Construction
    Graph compacted_graph(next_node_id);
    compacted_graph.all_edges_.reserve(kept_edges_count);

    // Copie des noms (uniquement pour les nœuds gardés)
    for (size_t v = keep_node.find_first();
         v != boost::dynamic_bitset<>::npos;
         v = keep_node.find_next(v))
    {
        compacted_graph.node_names_[node_map[v]] = node_names_[v];
    }

    // 4. Remap arcs (itère uniquement sur les arcs gardés)
    edge_map.assign(num_edges, REMOVED);
    for (size_t e = keep_edge.find_first();
         e != boost::dynamic_bitset<>::npos;
         e = keep_edge.find_next(e))
    {
        const Edge &edge = all_edges_[e];

        if (node_map[edge.source] == REMOVED || node_map[edge.target] == REMOVED)
            throw std::runtime_error(
                "compacted: arc " + std::to_string(e) +
                " gardé avec extrémité supprimée (src=" +
                std::to_string(edge.source) + ", tgt=" +
                std::to_string(edge.target) + ")");

        const double new_capacity = edge_new_capacity.empty()
                                        ? edge.capacity
                                        : edge_new_capacity[e];

        const uint16_t new_id = compacted_graph.add_edge(
            node_map[edge.source],
            node_map[edge.target],
            edge.weight,
            new_capacity);

        edge_map[e] = new_id;
    }

    // 5. Topology timeline (bitwise AND + iteration sur les survivants)
    compacted_graph.num_time_slots_ = num_time_slots_;
    compacted_graph.topology_timeline_.assign(
        num_time_slots_,
        boost::dynamic_bitset<>(kept_edges_count));

    for (uint8_t t = 0; t < num_time_slots_; ++t)
    {
        const auto survivors = topology_timeline_[t] & keep_edge;
        auto &new_tl = compacted_graph.topology_timeline_[t];

        for (size_t e = survivors.find_first();
             e != boost::dynamic_bitset<>::npos;
             e = survivors.find_next(e))
        {
            new_tl.set(edge_map[e]);
        }
    }

    compacted_graph.freeze();
    return compacted_graph;
}