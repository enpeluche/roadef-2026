// traffic/demand_base.hpp

#pragma once

#include "common/types.hpp"

#include <cstdint>

/**
 * @brief Données statiques d'une demande.
 * Taille : 3 * 2 (uint16) + 2 (padding) + 1 * 4 (float) = 12 octets.
 */
struct DemandBase
{
    uint16_t id;   ///< Id unique d'une demande.
    NodeId source; ///< id du noeud source
    NodeId target; ///< id du noeud destination
    float n2;      ///< Norme euclidienne d'une demande.
};