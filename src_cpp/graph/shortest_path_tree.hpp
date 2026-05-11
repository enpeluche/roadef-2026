// graph/shortest_path_tree.hpp

#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <queue>
#include <iostream>
/**
 * @brief
 */
struct ShortestPathTree
{
    uint16_t source_node;          ///< Identifiant du noeud source.
    std::vector<double> distances; ///< Vecteur des distances minimales de chaque noeud au noeud source.

    std::vector<uint16_t> predecessor_edge_offsets; ///< Indices des prédecesseurs dans predecessors_edge_ids.

    std::vector<uint16_t> predecessor_edge_ids; ///<  Indices des prédecesseurs.

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

    friend std::ostream &operator<<(std::ostream &os, const ShortestPathTree &spt);
};
