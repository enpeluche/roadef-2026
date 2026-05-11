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
    // Lecture du fichier
    std::string json_path = "instances/" + dataset + "/" + dataset + "-" + instance_id + "-net.json";

    std::ifstream file(json_path);

    if (!file.is_open())
        throw std::runtime_error("Impossible d'ouvrir le fichier :" + json_path);

    // Parsing du fichier
    json data = json::parse(file);

    const auto &nodes = data["nodes"];
    const auto &links = data["links"];

    Graph g(nodes.size());
    g.all_edges_.reserve(links.size());

    // Création des noeuds
    for (const auto &node : nodes)
    {
        uint16_t id = node["id"].get<uint16_t>();
        g.node_names_[id] = node["name"].get<std::string>();
    }

    // Création des arcs
    for (const auto &link : links)
        g.add_edge(link["from"].get<uint16_t>(), link["to"].get<uint16_t>(), link["metric"].get<double>(), link["capacity"].get<double>());

    // Tri des listes d'arcs sortants par noeud cible
    for (uint16_t i = 0; i < g.nodes_count(); ++i)
    {
        std::sort(g.out_edges_[i].begin(), g.out_edges_[i].end(),
                  [&g](uint32_t a, uint32_t b)
                  {
                      return g.all_edges_[a].target_node < g.all_edges_[b].target_node;
                  });
    }

    // Tri des listes d'arcs entrants par noeud source
    for (uint16_t i = 0; i < g.nodes_count(); ++i)
    {
        std::sort(g.in_edges_[i].begin(), g.in_edges_[i].end(),
                  [&g](uint32_t a, uint32_t b)
                  {
                      return g.all_edges_[a].source_node < g.all_edges_[b].source_node;
                  });
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
    os << "========== GRAPH SUMMARY ==========\n";
    os << " Nodes       : " << graph.nodes_count() << "\n";
    os << " Edges       : " << graph.edges_count() << "\n";
    os << " Density     : " << (graph.density() * 100.0) << " %\n";

    os << "\n First 5 edges samples:\n";
    for (uint16_t i = 0; i < std::min((uint16_t)5, (uint16_t)graph.edges_count()); ++i)
        os << "  " << graph.edge(i) << "\n";

    os << " Incoming Edges (Sample for first 5 nodes):\n";

    // Boucle 1 : On parcourt chaque nœud (le vecteur extérieur)
    // On force std::min à traiter les deux comme des uint16_t
    for (uint16_t n = 0; n < std::min<uint16_t>(5, graph.nodes_count()); ++n)
    {
        os << "  Node " << n << " <- [";

        // On récupère le vecteur d'IDs d'arcs entrant pour ce nœud
        const std::vector<uint16_t> &incoming = graph.in_edges_[n];

        // Boucle 2 : On parcourt les IDs stockés dans ce sous-vecteur
        for (size_t i = 0; i < incoming.size(); ++i)
        {
            os << incoming[i] << (i == incoming.size() - 1 ? "" : ", ");
        }
        os << "]\n";
    }
    os << " Outcoming Edges (Sample for first 5 nodes):\n";
    for (uint16_t n = 0; n < std::min<uint16_t>(5, graph.nodes_count()); ++n)
    {
        os << "  Node " << n << " -> [";

        // On récupère le vecteur d'IDs d'arcs entrant pour ce nœud
        const std::vector<uint16_t> &outcoming = graph.out_edges_[n];

        // Boucle 2 : On parcourt les IDs stockés dans ce sous-vecteur
        for (size_t i = 0; i < outcoming.size(); ++i)
        {
            os << outcoming[i] << (i == outcoming.size() - 1 ? "" : ", ");
        }
        os << "]\n";
    }
    os << "===================================\n";

    return os;
}