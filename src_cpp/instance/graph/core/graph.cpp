/**
 * @file graph.cpp
 */

#include "instance/graph/algorithm/shortest_path_tree.hpp"
#include "instance/graph/core/graph.hpp"

/**
 * @param kept_edges Masque binaire indiquant les arêtes à conserver (1 pour garder, 0 pour supprimer).
 * @param edge_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des arêtes.
 */
Graph Graph::remove_edges(const boost::dynamic_bitset<> &kept_edges, std::vector<EdgeId> &edge_mapping) const
{

    assert(frozen_ && "remove_edges() called on unfrozen graph");
    assert(kept_edges.size() == edges_count() && "Precondition failed: kept_edges bitset length must equal the total edge count.");

    const NodeCount sub_nodes_count = nodes_count();
    const EdgeCount sub_edge_count = static_cast<EdgeCount>(kept_edges.count());

    Graph subgraph(sub_nodes_count);

    subgraph.node_names_ = node_names_;

    subgraph.all_edges_.reserve(sub_edge_count);

    edge_mapping.assign(edges_count(), INVALID_EDGE);

    for (size_t e = kept_edges.find_first(); e != boost::dynamic_bitset<>::npos; e = kept_edges.find_next(e))
    {
        const Edge &edge = all_edges_[e];

        const EdgeId id = subgraph.add_edge(edge.source, edge.target, edge.weight, edge.capacity);

        edge_mapping[e] = id;
    }

    subgraph.freeze();
    return subgraph;
}

/**
 * @param kept_nodes Masque binaire indiquant les nœuds à conserver (1 pour garder, 0 pour supprimer).
 * @param node_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des nœuds.
 * @param edge_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des arêtes.
 */
Graph Graph::remove_nodes(const boost::dynamic_bitset<> &kept_nodes, std::vector<NodeId> &node_mapping, std::vector<EdgeId> &edge_mapping) const
{
    assert(frozen_ && "remove_nodes() called on unfrozen graph");
    assert(kept_nodes.size() == nodes_count() && "Precondition failed: kept_nodes bitset length must equal the total node count.");

    const NodeCount sub_nodes_count = static_cast<NodeCount>(kept_nodes.count());

    node_mapping.assign(nodes_count(), INVALID_NODE);

    Graph subgraph(sub_nodes_count);

    NodeId node_id = 0;

    for (size_t v = kept_nodes.find_first(); v != boost::dynamic_bitset<>::npos; v = kept_nodes.find_next(v))
    {
        node_mapping[v] = node_id++;
        subgraph.node_names_[node_mapping[v]] = node_names_[v];
    }

    edge_mapping.assign(edges_count(), INVALID_EDGE);

    for (size_t e = 0; e < edges_count(); e++)
    {
        const Edge &edge = all_edges_[e];

        if (node_mapping[edge.source] == INVALID_NODE || node_mapping[edge.target] == INVALID_NODE)
            continue;

        const EdgeId edge_id = subgraph.add_edge(node_mapping[edge.source], node_mapping[edge.target], edge.weight, edge.capacity);

        edge_mapping[e] = edge_id;
    }

    subgraph.freeze();
    return subgraph;
}

/**
 * @brief Construit une structure CSR (Compressed Sparse Row) à partir de listes d'adjacence.
 * @details Cette fonction effectue la conversion en deux étapes :
 * 1. Elle trie chaque sous-liste d'adjacence selon la fonction de clé fournie.
 * 2. Elle calcule les indices de décalage (offsets) et aplatit les listes triées dans un vecteur contigu.
 * @tparam KeyFn Type de la fonction ou de la lambda utilisée pour évaluer la clé de tri des arêtes.
 * @param adj Liste d'adjacence d'entrée (vecteur de vecteurs contenant les identifiants des arêtes).
 * \warning Ce paramètre est modifié par la fonction : les sous-listes sont triées de manière permanente.
 * @param key Fonction/lambda prenant un identifiant d'arête (`uint16_t`) et retournant la valeur de tri (ex: le nœud cible ou source).
 * @param offsets Vecteur de sortie qui contiendra les indices de début des arêtes pour chaque nœud.
 * @param edge_ids Vecteur de sortie qui contiendra l'ensemble des identifiants d'arêtes stockés de façon contiguë.
 */
template <class KeyFn>
void Graph::build_csr(std::vector<std::vector<EdgeId>> &adj, KeyFn key,
                      std::vector<EdgeCount> &offsets, std::vector<EdgeId> &edge_ids)
{
    const NodeCount n = static_cast<NodeCount>(adj.size());

    // --- 1. Tri sur place des listes d'adjacence

    for (auto &lst : adj)
        std::sort(lst.begin(), lst.end(), [&](EdgeId a, EdgeId b)
                  { return key(a) < key(b); });

    // --- 2. Calcul des offsets

    offsets.resize(n + 1);
    EdgeCount total = 0;

    for (NodeId u = 0; u < n; ++u)
    {
        offsets[u] = total;
        total += adj[u].size();
    }
    offsets[n] = total;

    // --- 3. Aplatissement dans le vecteur final contigu

    edge_ids.reserve(total);
    for (const auto &lst : adj)
        edge_ids.insert(edge_ids.end(), lst.begin(), lst.end());
}

/**
 * @brief Convertit les listes d'adjacence en format CSR.
 * À appeler après que tous les arcs aient été ajoutés.
 * Libère la mémoire des build_*_edges_.
 */
void Graph::freeze()
{
    if (frozen_)
        return;

    // --- 1. Construction des CSR entrants et sortants
    build_csr(build_out_edges_, [&](EdgeId e)
              { return all_edges_[e].target; }, out_offsets_, out_edge_ids_);

    build_csr(build_in_edges_, [&](EdgeId e)
              { return all_edges_[e].source; }, in_offsets_, in_edge_ids_);

    // --- 2. Libération de la mémoire de construction
    build_out_edges_.clear();
    build_out_edges_.shrink_to_fit();

    build_in_edges_.clear();
    build_in_edges_.shrink_to_fit();

    frozen_ = true;
}