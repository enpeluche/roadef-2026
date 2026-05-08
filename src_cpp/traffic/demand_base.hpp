// traffic/demand_base.hpp

#pragma once
#include <cstdint>

/**
 * @brief Données statiques d'une demande.
 * Taille : 3 * 8 (doubles) + 3 * 2 (uint16) + 2 (padding) = 32 octets.
 */
struct DemandBase
{
    uint16_t id; ///< Id unique d'une demande.

    uint16_t source; ///< id du noeud source
    uint16_t target; ///< id du noeud destination

    double n1;   ///< Norme 1 d'une demande : somme de ses éléménts.
    double n2;   ///< Norme euclidienne d'une demande.
    double ninf; ///< Norme infini d'une demande : max de ses éléménts.
};