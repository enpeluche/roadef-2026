// graph/core/edge.hpp

#pragma once

#include <iostream>
#include <cmath>
#include <cstdint>

namespace EdgeConsts
{
    /**
     * Facteur de conversion double→uint64 pour les poids.
     * Choisi pour conserver 8 chiffres significatifs.
     */
    constexpr uint64_t WEIGHT_SCALE = 100'000'000ULL; // 1e8

    inline uint64_t to_int(double w)
    {
        return static_cast<uint64_t>(std::round(w * WEIGHT_SCALE));
    }

    inline double to_double(uint64_t w)
    {
        return static_cast<double>(w) / static_cast<double>(WEIGHT_SCALE);
    }
}

/**
 * @brief Une structure représentant un arc pour un réseau télécom.
 *
 * @note La structure fait 24 octets.
 */
struct Edge
{
    uint16_t id;     ///< Identifiant unique de l'arc.
    uint16_t source; ///< Identifiant du noeud source.
    uint16_t target; ///< Identifiant du noeud destination.
    uint64_t weight; ///< Poids de l'arc, utilisé pour les PCC.
    double capacity; ///< Capacité de l'arc.

    /**
     * @brief Surcharge l'opérateur d'affichage pour la structure Edge.
     */
    friend std::ostream &operator<<(std::ostream &os, const Edge &edge)
    {
        os << "Edge(" << edge.id << ", "
           << edge.source << "->" << edge.target
           << ", [w=" << edge.weight << ", c=" << edge.capacity << "])";

        return os;
    }
};