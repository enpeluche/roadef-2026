/**
 * @file shortest_path_tree.cpp
 *
 */

#include "common/types.hpp"
#include "graph/algorithm/dijkstra_workspace.hpp"
#include "graph/algorithm/shortest_path_tree.hpp"
#include "graph/core/graph.hpp"

#include <iomanip>

/**
 * @brief Calcule le graphe des plus courts chemins (SPT/DAG) depuis un nœud source.
 * @details Cette implémentation de l'algorithme de Dijkstra gère les chemins multiples de poids égal.
 *
 * @param source Identifiant du nœud de départ.
 * @param t Pas de temps (Tick) définissant le contexte temporel de l'évaluation.
 * @param ws Espace de travail réutilisable (DijkstraWorkspace) pour éviter les réallocations mémoire lors d'appels successifs.
 * @param timeline Masque binaire indiquant l'état des arêtes pour cette évaluation (1 pour actif/traversable, 0 pour ignoré).
 * @return ShortestPathTree L'arbre résultant contenant les distances, les prédécesseurs, et le masque des arêtes appartenant au SPT.
 */
ShortestPathTree Graph::shortest_path_tree(NodeId source, Tick t, DijkstraWorkspace &ws, const boost::dynamic_bitset<> &timeline) const
{
    assert(frozen_ && "shortest_path_tree requires a frozen graph");

    const NodeCount n = nodes_count();
    ws.prepare(n);
    ws.distances[source] = 0;
    ws.pq.push({0, source});

    while (!ws.pq.empty())
    {
        auto [d, u] = ws.pq.top();
        ws.pq.pop();
        if (d > ws.distances[u])
            continue;

        const EdgeCount begin = out_offsets_[u];
        const EdgeCount end = out_offsets_[u + 1];

        for (EdgeCount i = begin; i < end; ++i)
        {
            const EdgeId edge_id = out_edge_ids_[i];

            if (!timeline.test(edge_id))
                continue;

            const Edge &edge = all_edges_[edge_id];
            const Weight new_dist = d + edge.weight;
            const NodeId v = edge.target;

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

    EdgeCount current_offset = 0;
    for (NodeId v = 0; v < n; ++v)
    {
        result.predecessor_edge_offsets[v] = current_offset;
        for (EdgeId edge_id : ws.tmp_predecessors[v])
        {
            result.predecessor_edge_ids.push_back(edge_id);
            result.edge_membership.set(edge_id);
            current_offset++;
        }
    }
    result.predecessor_edge_offsets[n] = current_offset;

    return result;
}
