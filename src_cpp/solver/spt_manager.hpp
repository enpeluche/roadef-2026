// solver/spt_manager.hpp

#pragma once

#include "graph/graph.hpp"
#include <omp.h>
#include <vector>
#include <memory>

/**
 * @class SPTManager
 * @brief Gestionnaire temporel et optimisé des arbres de plus courts chemins (SPT).
 * Cette classe centralise la création, le stockage et l'accès aux Shortest Path Trees
 * pour chaque noeud et chaque Time Slot de l'instance.
 */
class SPTManager
{
public:
    /**
     * @brief Construit le gestionnaire et pré-alloue la table d'accès.
     * @param g Le graphe de l'instance, utilisé pour extraire le nombre de
     * Time Slots et le nombre de noeuds afin de dimensionner la table.
     */
    SPTManager(const Graph &g)
    {
        access_table_.assign(g.num_time_slots(),
                             std::vector<ShortestPathTree *>(g.nodes_count(), nullptr));
    }

    /**
     * @brief Construit et propage tous les arbres de plus courts chemins sur la timeline.
     * @param g Le graphe contenant la topologie temporelle (mer de bits des pannes).
     */
    void build_all(const Graph &g);

    /**
     * @brief Récupère l'arbre de plus courts chemins pour un noeud et un Time Slot donnés.
     * @param t Le Time Slot (instant temporel ciblé).
     * @param u L'identifiant du noeud source.
     * @return const ShortestPathTree* Un pointeur constant vers le SPT demandé.
     */
    inline const ShortestPathTree *get_spt(uint8_t t, uint16_t u) const
    {
        return access_table_[t][u];
    }

private:
    std::vector<std::unique_ptr<ShortestPathTree>> spt_pool_;   ///< Pool de stockage pour chaque arbre instancié.
    std::vector<std::vector<ShortestPathTree *>> access_table_; ///< Table d'accès rapide 2D indexée par [Time Slot][ID Noeud]. Contient des pointeurs bruts (non-propriétaires) pointant vers les éléments du spt_pool_.
};