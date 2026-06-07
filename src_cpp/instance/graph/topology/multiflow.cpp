// graph/topology/multiflow.cpp

#include "graph/topology/multiflow.hpp"
#include "graph/core/graph.hpp"
#include "graph/algorithm/shortest_path_tree.hpp"
#include "graph/topology/mf_workspace.hpp"
#include <algorithm>
#include <iomanip>

MultiFlow::MultiFlow(uint16_t source, uint16_t target, const ShortestPathTree &spt, const Graph &g,
                     MultiFlowWorkspace &ws)
    : source_(source), target_(target)
{
    compute_fractions(spt, g, ws);
}

std::vector<uint16_t> MultiFlow::mandatory_arcs() const
{
    std::vector<uint16_t> mandatory;

    for (const auto &entry : arc_flow_fractions_)
        if (entry.fraction.is_one())
            mandatory.push_back(entry.id);

    return mandatory;
}

void MultiFlow::compute_fractions(const ShortestPathTree &spt, const Graph &g, MultiFlowWorkspace &ws)
{
    uint16_t n_nodes = g.nodes_count();
    uint16_t source = spt.source;

    ws.prepare(n_nodes);

    ws.bfs_queue.push_back(target_);
    ws.visited[target_] = 1;
    size_t head = 0;

    while (head < ws.bfs_queue.size())
    {
        uint16_t v = ws.bfs_queue[head++];
        ws.involved.push_back(v);

        if (v == source)
            continue;

        for (uint16_t i = spt.predecessor_edge_offsets[v]; i < spt.predecessor_edge_offsets[v + 1]; ++i)
        {
            uint16_t arc_id = spt.predecessor_edge_ids[i];
            uint16_t u = g.edge(arc_id).source;

            ws.temp_arcs.push_back({u, arc_id});

            if (!ws.visited[u])
            {
                ws.visited[u] = 1;
                ws.bfs_queue.push_back(u);
            }
        }
    }

    if (!ws.visited[source])
        return;

    // CSR : utilise ws.fg_head, ws.fg_succs_flat, ws.current_head
    for (const auto &edge : ws.temp_arcs)
        ws.fg_head[edge.u]++;

    uint16_t sum = 0;
    for (uint16_t i = 0; i <= n_nodes; ++i)
    {
        uint16_t temp = ws.fg_head[i];
        ws.fg_head[i] = sum;
        sum += temp;
    }

    ws.fg_succs_flat.resize(ws.temp_arcs.size());
    ws.current_head = ws.fg_head; // copie

    for (const auto &edge : ws.temp_arcs)
        ws.fg_succs_flat[ws.current_head[edge.u]++] = edge.arc_id;

    // Tri topologique sur involved
    std::sort(ws.involved.begin(), ws.involved.end(),
              [&spt](uint16_t a, uint16_t b)
              { return spt.distances[a] < spt.distances[b]; });

    // Propagation : utilise ws.node_flow
    ws.node_flow[source] = CompactFraction(1, 1);

    // Initialiser edge_membership_
    edge_membership_.resize(g.edges_count());
    edge_membership_.reset();

    for (uint16_t u : ws.involved)
    {
        if (ws.node_flow[u].num() == 0 || u == target_)
            continue;

        uint16_t start_idx = ws.fg_head[u];
        uint16_t end_idx = ws.fg_head[u + 1];
        uint16_t degree = end_idx - start_idx;
        if (degree == 0)
            continue;

        CompactFraction share(ws.node_flow[u].num(), ws.node_flow[u].den() * degree);

        for (uint16_t i = start_idx; i < end_idx; ++i)
        {
            uint16_t arc_id = ws.fg_succs_flat[i];
            arc_flow_fractions_.push_back({arc_id, share});
            edge_membership_.set(arc_id);

            uint16_t v = g.edge(arc_id).target;
            ws.node_flow[v] = ws.node_flow[v] + share;
        }
    }
}