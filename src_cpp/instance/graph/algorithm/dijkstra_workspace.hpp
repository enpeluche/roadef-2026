/**
 * @file dijkstra_workspace.hpp
 * Usage: #include "instance/graph/algorithm/dijkstra_workspace.hpp"
 */

#pragma once

#include "common/types.hpp"

#include <boost/container/small_vector.hpp>
#include <limits>
#include <queue>
#include <vector>

/**
 * @brief File de priorité (min-heap) réutilisable.
 * @tparam T Type des éléments stockés.
 */
template <typename T>
class ReusablePQ : public std::priority_queue<T, std::vector<T>, std::greater<T>>
{
public:
    /**
     * @brief Vide la file sans désallouer la mémoire interne (conserve la capacité).
     */
    void clear() { this->c.clear(); }
};

struct DijkstraWorkspace
{
    static constexpr Weight MAX = std::numeric_limits<Weight>::max();

    using Element = std::pair<Weight, NodeId>; // étiquette poids - ID

    std::vector<Weight> distances;                                           ///< Distances minimales trouvées depuis la source vers chaque nœud.
    std::vector<boost::container::small_vector<EdgeId, 4>> tmp_predecessors; ///< IDs des arcs prédécesseurs pour chaque nœud (gère le multi-path ECMP).
    ReusablePQ<Element> pq;                                                  ///< File de priorité (Min-Heap) pour explorer le nœud le plus proche.

    /**
     * @param n Le nombre de noeuds.
     */
    void init(NodeCount n)
    {
        distances.resize(n);
        tmp_predecessors.resize(n);
    }

    /**
     * @warning toujours appeler prepare après init
     * @param n Le nombre de noeuds.
     */
    void prepare(NodeCount n)
    {
        distances.assign(n, MAX);

        for (auto &v : tmp_predecessors)
            v.clear();

        pq.clear();
    }
};