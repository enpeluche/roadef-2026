/**
 * @file shortest_path_tree.hpp
 * Usage: #include "graph/algorithm/shortest_path_tree.hpp"
 *
 * @todo add leaf, early cut
 */
#pragma once

#include "common/types.hpp"

#include <boost/dynamic_bitset.hpp>
#include <ostream>
#include <span>
#include <vector>

/**
 * @brief Représente un arbre des plus courts chemins partant d'un noeud source.
 */
struct ShortestPathTree
{
    NodeId source;                                ///< Identifiant unique du noeud source.
    std::vector<Weight> distances;                ///< Vecteur des distances minimales de chaque noeud au noeud source.
    std::vector<EdgeId> predecessor_edge_offsets; ///< Indices des prédecesseurs dans predecessors_edge_ids.
    std::vector<EdgeId> predecessor_edge_ids;     ///< Indices des prédecesseurs.
    boost::dynamic_bitset<> edge_membership;      ///< Indique si un arc est utilisé ou pas

    /**
     * @brief Retourne une vue sécurisée sur les arcs prédecesseurs d'un noeud.
     */
    inline std::span<const EdgeId> predecessors(NodeId id) const
    {
        EdgeId start = predecessor_edge_offsets[id];
        EdgeId count = predecessor_edge_offsets[id + 1] - start;

        return std::span<const EdgeId>(predecessor_edge_ids.data() + start, count);
    }

    /**
     * @brief Renvoie la distance à un noeud.
     */
    inline Weight distance_to(NodeId target) const
    {
        return distances[target];
    }
};
