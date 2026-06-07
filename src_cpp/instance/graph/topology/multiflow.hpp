/**
 * @file multiflow.hpp
 * Usage: #include "graph/topology/multiflow.hpp"
 */

#pragma once

#include "common/types.hpp"
#include "graph/topology/compact_fraction.hpp"

#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <utility>
#include <vector>

class ShortestPathTree;
class Graph;
struct MultiFlowWorkspace;

class MultiFlow
{
public:
    struct ArcFraction
    {
        EdgeId id;
        CompactFraction fraction;

        bool operator<(const ArcFraction &other) const { return id < other.id; }
    };

    /**
     * @brief Construit le flux multi-chemins entre source et target basé sur le SPT.
     */
    MultiFlow(NodeId source, NodeId target, const ShortestPathTree &spt, const Graph &graph, MultiFlowWorkspace &ws);

    const std::vector<ArcFraction> &get_fractions() const { return arc_flow_fractions_; }
    std::vector<EdgeId> mandatory_arcs() const;

    NodeId source() const { return source_; }
    NodeId target() const { return target_; }

    const boost::dynamic_bitset<> &edge_membership() const { return edge_membership_; }

private:
    NodeId source_;                               ///< Source node of the flow.
    NodeId target_;                               ///< Destination node of the flow.
    std::vector<ArcFraction> arc_flow_fractions_; ///< Traffic fractions per active arc.
    boost::dynamic_bitset<> edge_membership_;     ///< Fast lookup mask for edges used in this flow.

    void compute_fractions(const ShortestPathTree &spt, const Graph &g, MultiFlowWorkspace &ws);
};