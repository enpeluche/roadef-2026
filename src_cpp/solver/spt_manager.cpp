// solver/spt_manager.cpp
// clang-format off

#include "solver/spt_manager.hpp"
#include "graph/shortest_path_tree.hpp"

void SPTManager::build_all(const Graph &g)
{
    const uint16_t n_nodes = g.nodes_count();
    const uint8_t n_slots = g.num_time_slots();

    // --- On calcule tout le TimeSlot 0
    
    std::vector<std::unique_ptr<ShortestPathTree>> tmp_spts(n_nodes);

    #pragma omp parallel for schedule(dynamic)
    for (uint16_t i = 0; i < n_nodes; ++i)
        tmp_spts[i] = std::make_unique<ShortestPathTree>(g.shortest_path_tree(i, 0));

    // On remplit le pool et la table d'accès proprement
    for (uint16_t i = 0; i < n_nodes; ++i)
    {
        access_table_[0][i] = tmp_spts[i].get();
        spt_pool_.push_back(std::move(tmp_spts[i]));
    }

    // --- Propagation temporelle
    
    std::vector<boost::dynamic_bitset<>> outages(n_slots);

    for (uint8_t t = 0; t < n_slots; ++t)
        outages[t] = ~g.get_timeline(t); // On pré-calcule l'inverse une seule fois par slot

    for (uint8_t t = 1; t < n_slots; ++t) // inverser l'ordre des boucles et faire un pragma critical ?
        for (uint16_t i = 0; i < n_nodes; ++i)
        {
            ShortestPathTree *prev_spt = access_table_[t - 1][i];

            // Si aucun arc de mon SPT n'est dans la liste des pannes actuelles
            if ((prev_spt->edge_membership & outages[t]).none())
                access_table_[t][i] = prev_spt;
            else
            {
                // Si l'arc est mort, on recalcule proprement pour ce slot
                auto new_spt = std::make_unique<ShortestPathTree>(g.shortest_path_tree(i, t));
                spt_pool_.push_back(std::move(new_spt));
                access_table_[t][i] = spt_pool_.back().get();
            }
        }

    std::cout << "Nombres de pools différents : " << spt_pool_.size() << "\n";
}