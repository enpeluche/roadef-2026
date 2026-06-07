/**
 * @file edge.hpp
 * Usage: #include "instance/graph/core/edge.hpp"
 *
 * @todo weight -> metric
 */

#pragma once

#include "common/types.hpp"

#include <cmath>

namespace EdgeConsts
{
    /**
     * Facteur de conversion double→Weight pour les poids.
     * Choisi pour conserver 8 chiffres significatifs.
     */
    constexpr Weight WEIGHT_SCALE = 100'000'000ULL; // 1e8

    inline Weight to_int(double w)
    {
        return static_cast<Weight>(std::round(w * WEIGHT_SCALE));
    }

    inline double to_double(Weight w)
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
    EdgeId id;         ///< Identifiant unique de l'arc.
    NodeId source;     ///< Identifiant du noeud source.
    NodeId target;     ///< Identifiant du noeud destination.
    Weight weight;     ///< Poids de l'arc, utilisé pour les PCC.
    Capacity capacity; ///< Capacité de l'arc.
};