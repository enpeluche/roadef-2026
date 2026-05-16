// graph_io.cpp

// TODO : convertir nlohmann en simdjson

#include "graph.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <limits>
#include <queue>

using json = nlohmann::json;

/**
 * @brief Construit un graphe du challenge ROADEF depuis un fichier json.
 * Va chercher les instances dans le dossier instances.
 *
 * @param dataset Le nom du dataset par exemple 'setA'
 * @param instance_id L'identifiant de l'instance en chaine de caractère.
 */
Graph Graph::from_json(const std::string &dataset, const std::string &instance_id)
{
    // Étape 0 : Lecture des fichiers

    std::string base_path = "instances/" + dataset + "/" + dataset + "-" + instance_id;

    auto load_json = [](const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("Impossible d'ouvrir le fichier : " + filepath);
        return json::parse(file);
    };

    json network_data = load_json(base_path + "-net.json");
    json scenario_data = load_json(base_path + "-scenario.json");
    json tm_data = load_json(base_path + "-tm.json");

    // Étape 1 :  Début du parsing

    const auto &nodes = network_data["nodes"]; // Liste des noeuds.
    const auto &links = network_data["links"]; // Liste des arcs.

    Graph g(nodes.size());
    g.all_edges_.reserve(links.size());

    // --- Création des noeuds
    for (const auto &node : nodes)
        g.node_names_[node["id"].get<uint16_t>()] = node["name"].get<std::string>();

    // === Création des arcs
    for (const auto &link : links)
        g.add_edge(link["from"].get<uint16_t>(), link["to"].get<uint16_t>(), link["metric"].get<double>(), link["capacity"].get<double>());

    // ---> Tri des d'arcs sortants par noeud cible
    for (uint16_t i = 0; i < g.nodes_count(); ++i)
        std::sort(g.out_edges_[i].begin(), g.out_edges_[i].end(),
                  [&g](uint32_t a, uint32_t b)
                  {
                      return g.all_edges_[a].target_node < g.all_edges_[b].target_node;
                  });

    // ---> Tri des d'arcs entrants par noeud source
    for (uint16_t i = 0; i < g.nodes_count(); ++i)
        std::sort(g.in_edges_[i].begin(), g.in_edges_[i].end(),
                  [&g](uint32_t a, uint32_t b)
                  {
                      return g.all_edges_[a].source_node < g.all_edges_[b].source_node;
                  });

    // === Création de la mer de bits.

    g.num_time_slots_ = tm_data["num_time_slots"]; // Nombre de times slots

    // Initialiser la mer de bits (tous les arcs sont UP par défaut)
    g.topology_timeline_.assign(g.num_time_slots_, boost::dynamic_bitset<>(g.all_edges_.size()));
    for (int t = 0; t < g.num_time_slots_; ++t)
        g.topology_timeline_[t].set(); // .set() met tous les bits à 1

    // Appliquer les pannes (mettre à 0 les arcs impactés par les interventions)
    if (scenario_data.contains("interventions"))
        for (const auto &inter : scenario_data["interventions"])
        {
            int t = inter["t"].get<int>();
            for (int link_id : inter["links"])
                g.topology_timeline_[t].reset(link_id); // .reset() met le bit à 0 (DOWN)
        }

    return g;
}
/**
 * @brief Exporte un graphe sous le bon format json du challenge ROADEF.
 * Va chercher les instances dans le dossier instances.
 *
 * @param dataset Le nom du dataset par exemple 'setA'
 * @param instance_id L'identifiant de l'instance en chaine de caractère.
 */

void Graph::to_json(const std::string &dataset, const std::string &instance_id) const
{
    std::string json_path = "instances/" + dataset + "/" + dataset + "-" + instance_id + "-net.json";

    std::filesystem::path path(json_path);
    std::filesystem::create_directories(path.parent_path());

    nlohmann::ordered_json j;

    // informations toujours vraies
    j["directed"] = true;
    j["multigraph"] = false;

    // préparation des noeuds
    j["nodes"] = nlohmann::json::array();
    uint16_t n = nodes_count();
    for (uint16_t i = 0; i < n; ++i)
        j["nodes"].push_back({{"name", node_names_[i]}, {"id", i}});

    // préparation des arcs
    j["links"] = nlohmann::json::array();
    for (const auto &edge : all_edges_)
        j["links"].push_back({{"id", edge.id}, {"from", edge.source_node}, {"to", edge.target_node}, {"metric", edge.weight}, {"capacity", edge.capacity}});

    // on pousse
    std::ofstream file(json_path);
    if (!file.is_open())
        throw std::runtime_error("Impossible de sauvegarder la solution dans : " + json_path);

    file << j;
}

/**
 * @brief Surcharge de l'opérateur d'affichage pour la classe Graph.
 *
 */
std::ostream &operator<<(std::ostream &os, const Graph &graph)
{
    os << "Graph:\n";
    os << " Nodes       : " << graph.nodes_count() << "\n";
    os << " Edges (Tot) : " << graph.active_edges_count() << "\n";
    os << " Density     : " << (graph.density() * 100.0) << " %\n";

    // --- Statistiques des degrés ---
    std::map<uint16_t, uint16_t> in_degree_dist;
    std::map<uint16_t, uint16_t> out_degree_dist;

    for (uint16_t n = 0; n < graph.nodes_count(); ++n)
    {
        in_degree_dist[graph.in_degree(n)]++;
        out_degree_dist[graph.out_degree(n)]++;
    }

    auto print_dist = [&](const std::string &label, const std::map<uint16_t, uint16_t> &dist)
    {
        os << " " << label << " Distribution :\n  ";
        int count = 0;
        for (auto const &[deg, freq] : dist)
        {
            os << "Deg[" << deg << "]:" << freq << " | ";
            if (++count % 5 == 0)
                os << "\n  "; // Retour à la ligne pour la lisibilité
        }
        os << "\n";
    };

    os << "\n";
    print_dist("In-Degree ", in_degree_dist);
    print_dist("Out-Degree", out_degree_dist);

    os << "\n First 5 edges samples:\n";
    for (uint16_t i = 0; i < std::min((uint16_t)5, graph.edges_count()); ++i)
        os << "  " << graph.edge(i) << "\n";

    return os;
}