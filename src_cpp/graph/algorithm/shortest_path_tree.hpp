// #include "graph/algorithm/shortest_path_tree.hpp"

#pragma once

#include <boost/dynamic_bitset.hpp>
#include <cstdint>
#include <ostream>
#include <span>
#include <vector>

// TODO : rajouter les feuilles, couper dès qu'on a toutes les cibles, ne pas calculer tout le SPT

/**
 * @brief Représente un arbre des plus courts chemins partant d'un noeud source.
 */
struct ShortestPathTree
{
    uint16_t source;                                ///< Identifiant unique du noeud source.
    std::vector<uint64_t> distances;                ///< Vecteur des distances minimales de chaque noeud au noeud source.
    std::vector<uint16_t> predecessor_edge_offsets; ///< Indices des prédecesseurs dans predecessors_edge_ids.
    std::vector<uint16_t> predecessor_edge_ids;     ///< Indices des prédecesseurs.
    boost::dynamic_bitset<> edge_membership;        ///< Indique si un arc est utilisé ou pas

    /**
     * @brief Retourne une vue sécurisée sur les arcs prédecesseurs d'un noeud.
     */
    inline std::span<const uint16_t> predecessors(uint16_t node_id) const
    {
        uint16_t start = predecessor_edge_offsets[node_id];
        uint16_t count = predecessor_edge_offsets[node_id + 1] - start;

        return std::span<const uint16_t>(predecessor_edge_ids.data() + start, count);
    }

    /**
     * @brief Surcharge de l'opérateur d'affichage.
     */
    friend std::ostream &operator<<(std::ostream &os, const ShortestPathTree &spt);

    /**
     * @brief Renvoie la distance à un noeud.
     */
    inline uint64_t distance_to(uint16_t target) const
    {
        return distances[target];
    }
};
