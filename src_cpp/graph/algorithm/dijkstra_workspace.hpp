// #include "graph/algorithm/dijkstra_workspace.hpp"

#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <queue>

struct DijkstraWorkspace
{
    static constexpr uint64_t MAX = std::numeric_limits<uint64_t>::max();

    using Element = std::pair<uint64_t, uint16_t>; // étiquette poids - ID

    std::vector<uint64_t> distances;                                              ///< Distances minimales trouvées depuis la source vers chaque nœud.
    std::vector<std::vector<uint16_t>> tmp_predecessors;                          ///< IDs des arcs prédécesseurs pour chaque nœud (gère le multi-path ECMP).
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq; ///< File de priorité (Min-Heap) pour explorer le nœud le plus proche.

    /**
     * @param n Le nombre de noeuds.
     */
    void init(uint16_t n)
    {
        distances.resize(n);
        tmp_predecessors.resize(n);
    }

    /**
     * @param n Le nombre de noeuds.
     */
    void prepare(uint16_t n)
    {
        distances.assign(n, MAX);

        for (auto &v : tmp_predecessors)
            v.clear();

        decltype(pq) empty;
        std::swap(pq, empty);
    }
};