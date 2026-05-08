#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <queue>

/**
 * @brief
 */
struct ShortestPathTree
{
    uint16_t source_node;
    std::vector<double> distances;

    std::vector<uint16_t> predecessor_edge_offsets;

    std::vector<uint16_t> predecessor_edge_ids;

    inline uint16_t get_predecessor_count(uint16_t node_id) const
    {
        return predecessor_edge_offsets[node_id + 1] - predecessor_edge_offsets[node_id];
    }

    inline const uint16_t *get_predecessors_begin(uint16_t node_id) const
    {
        return &predecessor_edge_ids[predecessor_edge_offsets[node_id]];
    }

    inline const uint16_t *get_predecessors_end(uint16_t node_id) const
    {
        return &predecessor_edge_ids[predecessor_edge_offsets[node_id + 1]];
    }
};
