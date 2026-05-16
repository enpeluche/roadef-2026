// graph/core/shortest_path_tree.hpp

#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <queue>
#include <boost/dynamic_bitset.hpp>
#include <iostream>

struct DijkstraWorkspace
{
    static constexpr double INF = std::numeric_limits<double>::infinity();
    using Element = std::pair<double, uint16_t>;

    std::vector<double> distances;
    std::vector<std::vector<uint16_t>> temp_predecessors;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    void prepare(uint16_t n)
    {
        distances.assign(n, INF);

        if (temp_predecessors.size() < n)
            temp_predecessors.resize(n);
        for (auto &v : temp_predecessors)
            v.clear();

        decltype(pq) empty;
        std::swap(pq, empty);
    }
};

/**
 * @brief Représente un arbre des plus courts chemins partant d'un noeud source.
 */
struct ShortestPathTree
{
    uint16_t source_node;                           ///< Identifiant du noeud source.
    std::vector<double> distances;                  ///< Vecteur des distances minimales de chaque noeud au noeud source.
    std::vector<uint16_t> predecessor_edge_offsets; ///< Indices des prédecesseurs dans predecessors_edge_ids.
    std::vector<uint16_t> predecessor_edge_ids;     ///< Indices des prédecesseurs.
    boost::dynamic_bitset<> edge_membership;        ///< Indique si un arc est utilisé ou pas

    /**
     * @brief Le nombre de prédecesseur d'un noeud
     */
    inline uint16_t get_predecessor_count(uint16_t node_id) const
    {
        return predecessor_edge_offsets[node_id + 1] - predecessor_edge_offsets[node_id];
    }

    /**
     * @brief Retourne un itérateur (pointeur) vers le premier arc prédecesseur du noeud donné.
     * @param node_id L'identifiant du noeud dont on veut explorer les prédecesseurs.
     * @return const uint16_t* Pointeur constant vers le premier ID d'arc prédecesseur.
     */
    inline const uint16_t *get_predecessors_begin(uint16_t node_id) const
    {
        return &predecessor_edge_ids[predecessor_edge_offsets[node_id]];
    }

    /**
     * @brief Retourne un itérateur (pointeur) marquant la fin de la liste des prédecesseurs du noeud.
     * @param node_id L'identifiant du noeud.
     * @return const uint16_t* Pointeur constant situé juste après le dernier arc prédecesseur (limite d'arrêt).
     */
    inline const uint16_t *get_predecessors_end(uint16_t node_id) const
    {
        return &predecessor_edge_ids[predecessor_edge_offsets[node_id + 1]];
    }

    /**
     * @brief Surcharge de l'opérateur d'affichage.
     */
    friend std::ostream &operator<<(std::ostream &os, const ShortestPathTree &spt);
};
