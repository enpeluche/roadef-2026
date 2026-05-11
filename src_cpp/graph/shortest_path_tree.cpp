// graph/shortest_path_tree.cpp

#include "shortest_path_tree.hpp"
#include "graph.hpp"

// des vecteurs de travail en paramètre ?
#include <iostream>
#include <iomanip> // pour std::setw

std::ostream &operator<<(std::ostream &os, const ShortestPathTree &spt)
{
    os << "========== SHORTEST PATH TREE ==========\n";
    os << " Source Node : " << spt.source_node << "\n";
    os << " Nodes found : " << spt.distances.size() << "\n";

    os << "\n First 5 destinations (Sample):\n";
    os << "  Dest | Distance | Predecessor Edges (IDs)\n";
    os << "  -----------------------------------------\n";

    // On affiche les 5 premiers nœuds (différents de la source)
    uint16_t displayed = 0;
    for (uint16_t i = 0; i < spt.distances.size(); ++i)
    {
        if (i == spt.source_node)
            continue; // On saute la source elle-même

        os << "  " << std::setw(4) << i << " | "
           << std::setw(8) << spt.distances[i] << " | [";

        // Récupération des prédécesseurs via tes méthodes inline
        const uint16_t *begin = spt.get_predecessors_begin(i);
        const uint16_t *end = spt.get_predecessors_end(i);

        for (const uint16_t *it = begin; it != end; ++it)
        {
            os << *it << (it + 1 == end ? "" : ", ");
        }

        os << "]\n";
        displayed++;
    }
    os << "========================================\n";
    return os;
}
ShortestPathTree Graph::shortest_path_tree(uint16_t source_node) const
{
    const uint16_t n = nodes_count();
    const double infty = std::numeric_limits<double>::infinity();

    std::vector<double> distances(n, infty);
    std::vector<std::vector<uint16_t>> temp_predecessors(n);

    distances[source_node] = 0;

    using Element = std::pair<double, uint16_t>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    pq.push({0.0, source_node});

    while (!pq.empty())
    {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > distances[u])
            continue;

        for (uint16_t edge_id : out_edges_[u])
        {
            const auto &edge = all_edges_[edge_id];
            double new_dist = d + edge.weight;
            uint16_t v = edge.target_node;

            const double EPS = 1e-9;

            if (new_dist < distances[v])
            {
                distances[v] = new_dist;
                temp_predecessors[v].clear();
                temp_predecessors[v].push_back(edge_id);
                pq.push({new_dist, v});
            }
            else if (std::abs(new_dist - distances[v]) < EPS)
            {
                // multi-path
                temp_predecessors[v].push_back(edge_id);
            }
        }
    }

    ShortestPathTree result;
    result.source_node = source_node;
    result.distances = std::move(distances);

    result.predecessor_edge_offsets.resize(n + 1);

    // Nombre total de prédecesseur
    size_t total_preds = 0;
    for (const auto &preds : temp_predecessors)
        total_preds += preds.size();

    result.predecessor_edge_ids.reserve(total_preds);

    //
    uint16_t current_offset = 0;
    for (uint16_t v = 0; v < n; ++v)
    {
        result.predecessor_edge_offsets[v] = current_offset;
        for (uint16_t edge_id : temp_predecessors[v])
        {
            result.predecessor_edge_ids.push_back(edge_id);
            current_offset++;
        }
    }
    result.predecessor_edge_offsets[n] = current_offset;

    return result;
}