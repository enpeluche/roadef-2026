// edge.hpp

#pragma once

#include <iostream>

struct Edge
{
    int from;
    int to;
    int id;
    double weight;
    double capacity;

    friend std::ostream &operator<<(std::ostream &os, const Edge &e)
    {
        os << "Edge(id=" << e.id
           << " | " << e.from << "->" << e.to
           << " | w=" << e.weight
           << " | cap=" << e.capacity << ")";

        return os;
    }
};