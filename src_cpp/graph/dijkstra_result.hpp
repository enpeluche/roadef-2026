// dijkstra_result.hpp

#pragma once

#include <vector>
#include <iostream>

struct DijkstraResult
{
    std::vector<double> distances;

    int source_node;

    std::vector<std::vector<int>> predecessors;
    std::vector<std::vector<int>> successors;

    size_t get_memory_usage() const
    {
        size_t total = 0;

        total += sizeof(distances);
        total += distances.size() * sizeof(double);

        total += sizeof(predecessors);
        for (const auto &v : predecessors)
        {
            total += sizeof(v);
            total += v.size() * sizeof(int);
        }

        return total;
    }

    friend std::ostream &operator<<(std::ostream &os, const DijkstraResult &dr)
    {
        for (size_t i = 0; i < dr.distances.size(); ++i)
        {
            os << "Noeud " << i << " | Dist: " << dr.distances[i] << " | Preds: [";

            for (size_t j = 0; j < dr.predecessors[i].size(); ++j)
            {
                os << dr.predecessors[i][j];

                if (j < dr.predecessors[i].size() - 1)
                    os << ", ";
            }
            os << "]\n";
        }

        return os;
    }
};