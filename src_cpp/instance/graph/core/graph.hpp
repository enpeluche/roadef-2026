/**
 * @file graph.hpp
 * Usage: #include "instance/graph/core/graph.hpp"
 */

#pragma once

#include "common/types.hpp"
#include "instance/graph/core/edge.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

class ShortestPathTree;
struct DijkstraWorkspace;

class Graph
{
public:
    static constexpr EdgeCount INVALID_EDGE = std::numeric_limits<EdgeCount>::max();

    Graph() = default;

    Graph(NodeCount nodes_count)
        : node_count_(nodes_count), build_in_edges_(nodes_count), build_out_edges_(nodes_count), node_names_(nodes_count)
    {
    }

    /**
     * @brief Construit un graphe du challenge ROADEF depuis un fichier json.
     * Va chercher les instances dans le dossier instances.
     *
     * @param dataset Le nom du dataset par exemple 'setA'
     * @param id L'identifiant de l'instance en chaine de caractère.
     */
    static Graph load(const std::string &dataset, const std::string &id);

    /**
     * @brief Ajoute un arc au graphe. Possible uniquement avant freeze().
     */
    EdgeId add_edge(NodeId source, NodeId target, Weight weight, Capacity capacity)
    {
        assert(!frozen_ && "add_edge() called on frozen graph");

        EdgeId id = static_cast<EdgeId>(all_edges_.size());
        all_edges_.push_back({id, source, target, weight, capacity});

        // Push simple, on triera dans freeze()
        build_out_edges_[source].push_back(id);
        build_in_edges_[target].push_back(id);

        return id;
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
    void build_csr(std::vector<std::vector<EdgeId>> &adj, KeyFn key,
                   std::vector<EdgeCount> &offsets, std::vector<EdgeId> &edge_ids);

    /**
     * @brief Convertit les listes d'adjacence en format CSR.
     * À appeler après que tous les arcs aient été ajoutés.
     * Libère la mémoire des build_*_edges_.
     */
    void freeze();

    /**
     * @brief Trouve l'arc (u, v) s'il existe. Renvoie INVALID_EDGE sinon.
     * Les out_edge_ids_ sont triés par target, donc binary search.
     */
    uint16_t edge_id(uint16_t u, uint16_t v) const
    {
        if (!frozen_)
        {
            throw std::logic_error("edge_id() called on unfrozen graph");
        }

        const uint16_t *begin = out_edge_ids_.data() + out_offsets_[u];
        const uint16_t *end = out_edge_ids_.data() + out_offsets_[u + 1];

        auto it = std::lower_bound(begin, end, v,
                                   [this](uint16_t edge_id, uint16_t target_to_find)
                                   {
                                       return all_edges_[edge_id].target < target_to_find;
                                   });

        if (it != end && all_edges_[*it].target == v)
            return *it;

        return INVALID_EDGE;
    }

    const Edge &edge(uint16_t edge_id) const { return all_edges_[edge_id]; }

    // Accesseurs adjacence post-freeze uniquement

    /**
     * @brief Renvoie une vue (span) sur les IDs des arcs sortants du nœud.
     */
    std::span<const uint16_t> outgoing_ids(uint16_t node_id) const
    {
        if (!frozen_)
        {
            throw std::logic_error("outgoing_ids() called on unfrozen graph");
        }

        return {out_edge_ids_.data() + out_offsets_[node_id],
                out_edge_ids_.data() + out_offsets_[node_id + 1]};
    }

    /**
     * @brief Renvoie une vue (span) sur les IDs des arcs entrants du nœud.
     */
    std::span<const uint16_t> incoming_ids(uint16_t node_id) const
    {
        if (!frozen_)
        {
            throw std::logic_error("incoming_ids() called on unfrozen graph");
        }

        return {in_edge_ids_.data() + in_offsets_[node_id],
                in_edge_ids_.data() + in_offsets_[node_id + 1]};
    }

    // Degrés

    /**
     * @brief Retourne le nombre d'arcs sortants d'un nœud (degré sortant).
     * @note Le graphe doit être figé (frozen).
     *
     * @param id L'identifiant du nœud.
     * @return uint16_t Le nombre d'arcs partant de ce nœud.
     */
    uint16_t out_degree(NodeId id) const
    {
        if (!frozen_)
        {
            throw std::logic_error("out_degree() called on unfrozen graph");
        }

        return static_cast<uint16_t>(out_offsets_[id + 1] - out_offsets_[id]);
    }

    /**
     * @brief Retourne le nombre d'arcs entrants d'un nœud (degré entrant).
     * @note Le graphe doit être figé (frozen).
     *
     * @param id L'identifiant du nœud.
     * @return uint16_t Le nombre d'arcs arrivant à ce nœud.
     */
    uint16_t in_degree(NodeId id) const
    {
        if (!frozen_)
        {
            throw std::logic_error("in_degree() called on unfrozen graph");
        }

        return static_cast<uint16_t>(in_offsets_[id + 1] - in_offsets_[id]);
    }

    /**
     * @brief Retourne le degré total d'un nœud (somme des arcs entrants et sortants).
     *
     * @param id L'identifiant du nœud.
     * @return uint16_t Le nombre total d'arcs liés à ce nœud.
     */
    uint16_t degree(NodeId id) const
    {
        if (!frozen_)
        {
            throw std::logic_error("degree() called on unfrozen graph");
        }

        return in_degree(id) + out_degree(id);
    }

    // === Métriques ===

    const std::string &node_name(NodeId id) const { return node_names_[id]; }

    EdgeCount edges_count() const { return static_cast<EdgeCount>(all_edges_.size()); }
    NodeCount nodes_count() const { return node_count_; }

    /**
     * @brief Calcule la densité du graphe.
     */
    double density() const
    {
        if (node_count_ <= 1)
            return 0.0;
        double max_edges = static_cast<double>(node_count_) * (node_count_ - 1);
        return static_cast<double>(edges_count()) / max_edges;
    }

    // SPT (Dijkstra)

    ShortestPathTree shortest_path_tree(NodeId source, Tick t, DijkstraWorkspace &ws, const boost::dynamic_bitset<> &timeline) const;

    // Compactage (utilisé par kernelize)

    void set_node_name(NodeId id, std::string name) { node_names_[id] = std::move(name); }

    /**
     * @param kept_edges Masque binaire indiquant les arêtes à conserver (1 pour garder, 0 pour supprimer).
     * @param edge_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des arêtes.
     */
    Graph remove_edges(const boost::dynamic_bitset<> &kept_edges, std::vector<EdgeId> &edge_mapping) const;

    /**
     * @param kept_nodes Masque binaire indiquant les nœuds à conserver (1 pour garder, 0 pour supprimer).
     * @param node_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des nœuds.
     * @param edge_mapping Vecteur de sortie contenant la correspondance entre les anciens et les nouveaux indices des arêtes.
     */
    Graph remove_nodes(const boost::dynamic_bitset<> &kept_nodes, std::vector<NodeId> &node_mapping, std::vector<EdgeId> &edge_mapping) const;

private:
    NodeCount node_count_ = 0; ///< Nombre total de nœuds dans le graphe.
    bool frozen_ = false;      ///< Indique si le graphe est verrouillé (structure CSR figée) et prêt pour le routage.

    std::vector<Edge> all_edges_;         ///< Liste contiguë de tous les arcs du graphe.
    std::vector<std::string> node_names_; ///< Identifiants textuels des nœuds.

    // Buffers de construction (vivants pendant le build, vidés au freeze)
    std::vector<std::vector<EdgeId>> build_in_edges_;  ///< Buffer temporaire des arcs entrants par nœud.
    std::vector<std::vector<EdgeId>> build_out_edges_; ///< Buffer temporaire des arcs sortants par nœud.

    // CSR final (utilisé après freeze)
    std::vector<EdgeId> out_offsets_;  ///< Indices de début des arcs sortants pour chaque nœud (CSR).
    std::vector<EdgeId> out_edge_ids_; ///< Liste contiguë des identifiants d'arcs sortants (CSR).
    std::vector<EdgeId> in_offsets_;   ///< Indices de début des arcs entrants pour chaque nœud (CSC).
    std::vector<EdgeId> in_edge_ids_;  ///< Liste contiguë des identifiants d'arcs entrants (CSC).
};