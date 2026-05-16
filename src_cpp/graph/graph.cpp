// graph/graph.cpp
// clang-format off

#include "graph.hpp"

/**
 * @brief Calcul la capacité entrante d'un noeud.
 * 
 * @param node_id Identifiant du noeud.
 */
double Graph::node_ingress_cap(uint16_t node_id) const
{
    const std::vector<uint16_t> &in = incoming_edges_ids(node_id);

    double sum = 0;

    for (uint16_t id : in) sum += edge(id).capacity;

    return sum;
}
/**
 * @brief Calcul la capacité sortante d'un noeud.
 * 
 * @param node_id Identifiant du noeud.
 */
double Graph::node_egress_cap(uint16_t node_id) const
{
    const std::vector<uint16_t> &out = outgoing_ids(node_id);

    double sum = 0;

    for (uint16_t id : out) sum += edge(id).capacity;

    return sum;
}
/**
 * @brief  Calcul la capacité totale du graphe.
 * 
 */
double Graph::total_cap() const
{
    double total_capacity = 0;

    for (const auto &edge : all_edges_) total_capacity += edge.capacity;

    return total_capacity;
}
/**
 * @brief Calcul la pression d'un noeud.
 * 
 * @param node_id Identifiant du noeud.
 */
double Graph::node_pressure_index(uint16_t node_id) const
{
    double in = node_ingress_cap(node_id);
    double out = node_egress_cap(node_id);

    if (out == 0) return in;

    return in / out;
}

/**
 * @brief Retire physiquement les arcs des listes d'adjacence selon un masque.
 * @param to_remove_mask Un bitset où 'true' signifie que l'arc doit être retiré.
 */
void Graph::filter_edges(const boost::dynamic_bitset<>& to_remove_mask) {
    if (to_remove_mask.none()) return;

    for (uint16_t i = 0; i < node_count_; ++i) {
        // Filtrage des arcs sortants
        auto& out = out_edges_[i];
        out.erase(std::remove_if(out.begin(), out.end(), [&](uint16_t id) {
            return to_remove_mask.test(id);
        }), out.end());

        // Filtrage des arcs entrants
        auto& in = in_edges_[i];
        in.erase(std::remove_if(in.begin(), in.end(), [&](uint16_t id) {
            return to_remove_mask.test(id);
        }), in.end());
    }

    // On s'assure que la timeline est synchronisée (un arc filtré est mort pour toujours)
    for (auto& slot_bits : topology_timeline_) {
        slot_bits &= ~to_remove_mask;
    }
}

uint32_t Graph::active_edges_count() const {
    uint32_t count = 0;
    for (const auto& out : out_edges_) count += out.size();
    return count;
}