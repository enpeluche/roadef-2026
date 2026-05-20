// traffic/demand_base.hpp

#pragma once

#include <cstdint>

/**
 * @brief Données statiques d'une demande.
 * Taille : 3 * 2 (uint16) + 2 (padding) + 1 * 4 (float) = 12 octets.
 */
struct DemandBase
{
    uint16_t id;     ///< Id unique d'une demande.
    uint16_t source; ///< id du noeud source
    uint16_t target; ///< id du noeud destination
    float n2;        ///< Norme euclidienne d'une demande.
};