// #include "graph/topology/multiflow/mf_workspace.hpp"

#pragma once

#include <cstdint>
#include <vector>

#include "common/types.hpp"
#include "graph/topology/multiflow/compact_fraction.hpp"

/**
 * @brief Buffers réutilisables pour le calcul des flux multiples (MultiFlow).
 * @details Conçu pour être instancié une fois par thread. La méthode prepare(n)
 *          réinitialise la mémoire à moindre coût (sans désallocation).
 */
struct MultiFlowWorkspace
{
    /**
     * @brief Représentation temporaire d'un arc pour construire le sous-graphe.
     */
    struct TempArc
    {
        uint16_t u;      ///< Nœud source de l'arc.
        uint16_t arc_id; ///< Identifiant unique de l'arc.
    };

    std::vector<uint8_t> visited;    ///< Marqueurs de visite pour le BFS (uint8_t évite le proxy de vector<bool>).
    std::vector<uint16_t> involved;  ///< Liste des nœuds atteints par le flux.
    std::vector<uint16_t> bfs_queue; ///< File d'attente contiguë pour le parcours BFS.
    std::vector<TempArc> temp_arcs;  ///< Buffer des arcs collectés lors de la remontée du graphe.

    std::vector<uint16_t> fg_head;       ///< CSR : Offsets de départ des arcs sortants (sous-graphe).
    std::vector<uint16_t> fg_succs_flat; ///< CSR : Identifiants des arcs sortants.
    std::vector<uint16_t> current_head;  ///< Offsets temporaires de travail pour remplir le CSR.

    std::vector<CompactFraction> node_flow; ///< Fraction du flux total arrivant à chaque nœud.

    /**
     * @brief Prépare l'espace de travail pour un nouveau calcul de flux.
     * @details Agrandit les capacités si nécessaire, puis utilise std::fill_n
     *          pour une réinitialisation mémoire ultra-rapide (souvent vectorisée).
     *
     * @param n_nodes Le nombre total de nœuds dans le graphe.
     */
    void prepare(uint16_t n_nodes)
    {
        if (visited.size() < n_nodes)
        {
            visited.resize(n_nodes);
            node_flow.resize(n_nodes);
            fg_head.resize(n_nodes + 1);
        }

        std::fill_n(visited.data(), n_nodes, uint8_t(0));
        std::fill_n(fg_head.data(), n_nodes + 1, uint16_t(0));

        std::fill_n(node_flow.data(), n_nodes, CompactFraction(0, 1));

        involved.clear();
        bfs_queue.clear();
        temp_arcs.clear();
        fg_succs_flat.clear();
        current_head.clear();
    }
};