#include "graph/algorithm/dijkstra_workspace.hpp"
#include "graph/algorithm/shortest_path_tree.hpp"
#include "graph/core/graph.hpp"

#include <iomanip>

/**
 * @brief Calcule l'arbre des plus courts chemins (SPT) depuis un nœud source.
 *
 * @param source Identifiant du nœud de départ.
 * @param t Pas de temps définissant la topologie active du graphe.
 * @param ws Espace de travail réutilisable (évite les réallocations mémoire).
 * @return ShortestPathTree L'arbre résultant contenant distances et prédécesseurs.
 */
ShortestPathTree Graph::shortest_path_tree(uint16_t source, uint8_t t, DijkstraWorkspace &ws) const
{
    assert(frozen_ && "shortest_path_tree requires a frozen graph");

    const uint16_t n = nodes_count();
    ws.prepare(n);
    ws.distances[source] = 0;
    ws.pq.push({0, source});

    const auto &timeline = topology_timeline_[t];

    while (!ws.pq.empty())
    {
        auto [d, u] = ws.pq.top();
        ws.pq.pop();
        if (d > ws.distances[u])
            continue;

        const uint32_t begin = out_offsets_[u];
        const uint32_t end = out_offsets_[u + 1];

        for (uint32_t i = begin; i < end; ++i)
        {
            const uint16_t edge_id = out_edge_ids_[i];

            if (!timeline.test(edge_id))
                continue;

            const Edge &edge = all_edges_[edge_id];
            const uint64_t new_dist = d + edge.weight;
            const uint16_t v = edge.target;

            if (new_dist < ws.distances[v])
            {
                ws.distances[v] = new_dist;
                ws.tmp_predecessors[v].clear();
                ws.tmp_predecessors[v].push_back(edge_id);
                ws.pq.push({new_dist, v});
            }
            else if (new_dist == ws.distances[v])
                ws.tmp_predecessors[v].push_back(edge_id);
        }
    }

    ShortestPathTree result;

    result.source = source;
    result.distances = ws.distances;
    result.edge_membership.resize(all_edges_.size(), false);
    result.predecessor_edge_offsets.resize(n + 1);

    size_t total_preds = 0;
    for (const auto &preds : ws.tmp_predecessors)
        total_preds += preds.size();
    result.predecessor_edge_ids.reserve(total_preds);

    uint16_t current_offset = 0;
    for (uint16_t v = 0; v < n; ++v)
    {
        result.predecessor_edge_offsets[v] = current_offset;
        for (uint16_t edge_id : ws.tmp_predecessors[v])
        {
            result.predecessor_edge_ids.push_back(edge_id);
            result.edge_membership.set(edge_id);
            current_offset++;
        }
    }
    result.predecessor_edge_offsets[n] = current_offset;

    return result;
}

/**
 * @brief Surcharge l'affichage.
 */
std::ostream &operator<<(std::ostream &os, const ShortestPathTree &spt)
{
    os << "========== SHORTEST PATH TREE ==========\n";
    os << " Source Node : " << spt.source << "\n";
    os << " Nodes found : " << spt.distances.size() << "\n";

    os << "  Dest | Distance | Predecessor Edges (IDs)\n";
    os << "  -----------------------------------------\n";

    for (uint16_t i = 0; i < spt.distances.size(); ++i)
    {
        if (i == spt.source)
            continue;

        os << "  " << std::setw(4) << i << " | ";

        if (spt.distances[i] == DijkstraWorkspace::MAX)
            os << std::setw(8) << "INF" << " | [";
        else
            os << std::setw(8) << spt.distances[i] << " | [";

        auto preds = spt.predecessors(i);
        for (size_t p = 0; p < preds.size(); ++p)
            os << preds[p] << (p + 1 == preds.size() ? "" : ", ");

        os << "]\n";
    }
    return os;
}