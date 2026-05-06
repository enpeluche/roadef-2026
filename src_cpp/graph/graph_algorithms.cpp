// graph_algorithms.cpp

#include "graph.hpp"
#include "dijkstra_result.hpp"

#include <fstream>
#include <limits>
#include <queue>

DijkstraResult Graph::compute_dijkstra(int source_node) const
{

    double infty = std::numeric_limits<double>::infinity();

    std::vector<double> distances;
    distances.assign(in_edges_.size(), infty);

    std::vector<std::vector<int>> predecessors;
    predecessors.assign(in_edges_.size(), std::vector<int>());

    distances[source_node] = 0;

    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>>
        pq;

    pq.push({0, source_node});

    while (!pq.empty())
    {

        std::pair<double, int> top = pq.top();

        double d = top.first;
        int u = top.second;

        pq.pop();

        if (d > distances[u])
            continue;

        for (int link : out_edges_[u])
        {
            double poids = all_edges_[link].weight;

            double nouvelle_dist = distances[u] + poids;

            int v = all_edges_[link].to;

            if (nouvelle_dist < distances[v])
            {

                distances[v] = nouvelle_dist;
                predecessors[v].clear();
                predecessors[v].push_back(all_edges_[link].id);
                pq.push({nouvelle_dist, v});
            }
            else if (nouvelle_dist == distances[v])
            {
                predecessors[v].push_back(all_edges_[link].id);
            }
        }
    }

    std::vector<std::vector<int>> successors(num_nodes());
    for (int v = 0; v < num_nodes(); ++v)
    {
        for (int arc_id : predecessors[v])
        {
            int u = get_edge(arc_id).from;
            successors[u].push_back(arc_id);
        }
    }

    return DijkstraResult{distances, source_node, predecessors, successors};
}
