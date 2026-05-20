#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <span>
#include "graph/core/edge.hpp"
#include "graph/algorithm/shortest_path_tree.hpp"
#include <boost/dynamic_bitset.hpp>

struct DijkstraWorkspace;

class Graph
{
public:
    static constexpr uint16_t INVALID_EDGE = 0xFFFF;

    Graph() = default;

    Graph(uint16_t nodes_count)
        : node_count_(nodes_count), build_in_edges_(nodes_count), build_out_edges_(nodes_count), node_names_(nodes_count)
    {
    }

    // I/O
    static Graph from_json(const std::string &dataset, const std::string &instance_id);
    void to_json(const std::string &dataset, const std::string &instance_id) const;

    /**
     * @brief Ajoute un arc au graphe. Possible uniquement avant freeze().
     */
    uint16_t add_edge(uint16_t source, uint16_t target, uint64_t weight, double capacity)
    {
        assert(!frozen_ && "Cannot add edges after freeze()");

        uint16_t id = static_cast<uint16_t>(all_edges_.size());
        all_edges_.push_back({id, source, target, weight, capacity});

        // Push simple, on triera dans freeze()
        build_out_edges_[source].push_back(id);
        build_in_edges_[target].push_back(id);

        return id;
    }

    /**
     * @brief Convertit les listes d'adjacence en format CSR.
     * À appeler après que tous les arcs aient été ajoutés.
     * Libère la mémoire des build_*_edges_.
     */
    void freeze()
    {
        if (frozen_)
            return;

        const uint16_t n = node_count_;

        // 1. Tri intra-listes : out par target, in par source
        for (uint16_t u = 0; u < n; ++u)
        {
            std::sort(build_out_edges_[u].begin(), build_out_edges_[u].end(),
                      [this](uint16_t a, uint16_t b)
                      {
                          return all_edges_[a].target < all_edges_[b].target;
                      });
            std::sort(build_in_edges_[u].begin(), build_in_edges_[u].end(),
                      [this](uint16_t a, uint16_t b)
                      {
                          return all_edges_[a].source < all_edges_[b].source;
                      });
        }

        // 2. Construction CSR sortant
        out_offsets_.resize(n + 1);
        uint32_t total_out = 0;
        for (uint16_t u = 0; u < n; ++u)
        {
            out_offsets_[u] = total_out;
            total_out += build_out_edges_[u].size();
        }
        out_offsets_[n] = total_out;
        out_edge_ids_.reserve(total_out);
        for (uint16_t u = 0; u < n; ++u)
            for (uint16_t e : build_out_edges_[u])
                out_edge_ids_.push_back(e);

        // 3. Construction CSR entrant
        in_offsets_.resize(n + 1);
        uint32_t total_in = 0;
        for (uint16_t u = 0; u < n; ++u)
        {
            in_offsets_[u] = total_in;
            total_in += build_in_edges_[u].size();
        }
        in_offsets_[n] = total_in;
        in_edge_ids_.reserve(total_in);
        for (uint16_t u = 0; u < n; ++u)
            for (uint16_t e : build_in_edges_[u])
                in_edge_ids_.push_back(e);

        // 4. Libération mémoire des buffers de construction
        std::vector<std::vector<uint16_t>>().swap(build_out_edges_);
        std::vector<std::vector<uint16_t>>().swap(build_in_edges_);

        frozen_ = true;
    }

    bool is_frozen() const { return frozen_; }

    /**
     * @brief Trouve l'arc (u, v) s'il existe. Renvoie INVALID_EDGE sinon.
     * Les out_edge_ids_ sont triés par target, donc binary search.
     */
    uint16_t edge_id(uint16_t u, uint16_t v) const
    {
        assert(frozen_ && "edge_id requires a frozen graph");

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
        assert(frozen_ && "outgoing_ids requires a frozen graph");
        return {out_edge_ids_.data() + out_offsets_[node_id],
                out_edge_ids_.data() + out_offsets_[node_id + 1]};
    }

    /**
     * @brief Renvoie une vue (span) sur les IDs des arcs entrants du nœud.
     */
    std::span<const uint16_t> incoming_ids(uint16_t node_id) const
    {
        assert(frozen_ && "incoming_ids requires a frozen graph");
        return {in_edge_ids_.data() + in_offsets_[node_id],
                in_edge_ids_.data() + in_offsets_[node_id + 1]};
    }

    // Degrés

    /**
     * @brief Retourne le nombre d'arcs sortants d'un nœud (degré sortant).
     * @note Le graphe doit être figé (frozen).
     *
     * @param node_id L'identifiant du nœud.
     * @return uint16_t Le nombre d'arcs partant de ce nœud.
     */
    uint16_t out_degree(uint16_t node_id) const
    {
        assert(frozen_);
        return static_cast<uint16_t>(out_offsets_[node_id + 1] - out_offsets_[node_id]);
    }

    /**
     * @brief Retourne le nombre d'arcs entrants d'un nœud (degré entrant).
     * @note Le graphe doit être figé (frozen).
     *
     * @param node_id L'identifiant du nœud.
     * @return uint16_t Le nombre d'arcs arrivant à ce nœud.
     */
    uint16_t in_degree(uint16_t node_id) const
    {
        assert(frozen_);
        return static_cast<uint16_t>(in_offsets_[node_id + 1] - in_offsets_[node_id]);
    }

    /**
     * @brief Retourne le degré total d'un nœud (somme des arcs entrants et sortants).
     *
     * @param node_id L'identifiant du nœud.
     * @return uint16_t Le nombre total d'arcs liés à ce nœud.
     */
    uint16_t degree(uint16_t node_id) const
    {
        return in_degree(node_id) + out_degree(node_id);
    }

    // === Métriques ===

    const std::string &node_name(uint16_t id) const { return node_names_[id]; }

    uint16_t edges_count() const { return static_cast<uint16_t>(all_edges_.size()); }
    uint16_t nodes_count() const { return node_count_; }
    uint8_t time_slots_count() const { return num_time_slots_; }

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

    // Topology timeline

    /**
     * @brief Récupère le masque d'activation des arcs pour un pas de temps donné.
     *
     * @param t Le pas de temps (time slot).
     * @return const boost::dynamic_bitset<>& Référence constante vers le masque binaire.
     */
    const boost::dynamic_bitset<> &timeline(uint8_t t) const { return topology_timeline_[t]; }

    /**
     * @brief Initialise la structure temporelle de la topologie du graphe.
     *
     * @param n_slots Le nombre total de pas de temps à allouer.
     */
    void init_topology(uint8_t n_slots)
    {
        num_time_slots_ = n_slots;
        topology_timeline_.assign(n_slots, boost::dynamic_bitset<>(all_edges_.size()));
    }

    /**
     * @brief Définit l'état des arcs pour un pas de temps spécifique.
     *
     * @param t Le pas de temps cible.
     * @param bits Le masque binaire d'activation des arcs (transféré par déplacement).
     */
    void set_timeline_slot(uint8_t t, boost::dynamic_bitset<> bits)
    {
        topology_timeline_[t] = std::move(bits);
    }

    // SPT (Dijkstra)

    ShortestPathTree shortest_path_tree(uint16_t source, uint8_t t, DijkstraWorkspace &ws) const;

    // Affichage

    friend std::ostream &operator<<(std::ostream &os, const Graph &graph);

    // Compactage (utilisé par kernelize)

    /**
     * @brief Construit une copie compactée du graphe.
     * Le graphe résultant est automatiquement frozen.
     */
    Graph compacted(const boost::dynamic_bitset<> &keep_node,
                    const boost::dynamic_bitset<> &keep_edge,
                    std::vector<uint16_t> &node_map,
                    std::vector<uint16_t> &edge_map,
                    const std::vector<double> &edge_new_capacity = {}) const;

    void set_node_name(uint16_t id, std::string name) { node_names_[id] = std::move(name); }

private:
    uint16_t node_count_ = 0;    ///< Nombre total de nœuds dans le graphe.
    bool frozen_ = false;        ///< Indique si le graphe est verrouillé (structure CSR figée) et prêt pour le routage.
    uint8_t num_time_slots_ = 0; ///< Nombre total de pas de temps (topologie dynamique).

    std::vector<Edge> all_edges_;                            ///< Liste contiguë de tous les arcs du graphe.
    std::vector<std::string> node_names_;                    ///< Identifiants textuels des nœuds.
    std::vector<boost::dynamic_bitset<>> topology_timeline_; ///< Masques d'activation des arcs pour chaque pas de temps.

    // Buffers de construction (vivants pendant le build, vidés au freeze)
    std::vector<std::vector<uint16_t>> build_in_edges_;  ///< Buffer temporaire des arcs entrants par nœud.
    std::vector<std::vector<uint16_t>> build_out_edges_; ///< Buffer temporaire des arcs sortants par nœud.

    // CSR final (utilisé après freeze)
    std::vector<uint32_t> out_offsets_;  ///< Indices de début des arcs sortants pour chaque nœud (CSR).
    std::vector<uint16_t> out_edge_ids_; ///< Liste contiguë des identifiants d'arcs sortants (CSR).
    std::vector<uint32_t> in_offsets_;   ///< Indices de début des arcs entrants pour chaque nœud (CSC).
    std::vector<uint16_t> in_edge_ids_;  ///< Liste contiguë des identifiants d'arcs entrants (CSC).
};