// graph/core/edge.hpp

#pragma once

#include <cstdint>
#include <iostream>

/**
 * @brief Une structure représentant un arc pour un réseau télécom.
 *
 * @note La structure fait 24 octets.
 */
struct Edge
{
    uint16_t id;          ///< Identifiant unique de l'arc.
    uint16_t source_node; ///< Identifiant du noeud source.
    uint16_t target_node; ///< Identifiant du noeud destination.
    double weight;        ///< Poids de l'arc.
    double capacity;      ///< Capacité de l'arc.

    /**
     * @brief Surcharge l'opérateur d'affichage pour la structure Edge.
     */
    friend std::ostream &operator<<(std::ostream &os, const Edge &edge)
    {
        os << "Edge(" << edge.id << ", "
           << edge.source_node << "->" << edge.target_node
           << ", [w=" << edge.weight << ", c=" << edge.capacity << "])";

        return os;
    }
};