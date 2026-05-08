// graph/edge.hpp

#pragma once

#include <cstdint>

/**
 * @brief Une structure représentant un arc pour un réseau télécom.
 *
 * @note La structure fait 24 octets.
 */
struct Edge
{
    uint32_t id;          ///< Identifiant unique de l'arc.
    uint16_t source_node; ///< Identifiant du noeud source.
    uint16_t target_node; ///< Identifiant du noeud destination.

    double weight;   ///< Poids de l'arc.
    double capacity; ///< Capacité de l'arc.
};