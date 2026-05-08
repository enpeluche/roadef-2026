// graph_io.cpp

// TODO : convertir nlohmann en simdjson

#include "graph.hpp"
#include <nlohmann/json.hpp>
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

    nlohmann::json j;

    // informations toujours vraies
    j["directed"] = true;
    j["multigraph"] = false;

    // préparation des noeuds
    j["nodes"] = nlohmann::json::array();
    uint16_t n = nodes_count();
    for (uint16_t i = 0; i < n; ++i)
        j["nodes"].push_back({{"name", node_names_[i], {"id", i}}});

    // préparatio, des arcs
    j["links"] = nlohmann::json::array();
    for (const auto &edge : all_edges_)
        j["links"].push_back({{"id", edge.id}, {"from", edge.source_node}, {"to", edge.target_node}, {"metric", edge.weight}, {"capacity", edge.capacity}});

    // on pousse
    std::ofstream file(json_path);
    if (!file.is_open())
        throw std::runtime_error("Impossible de sauvegarder la solution dans : " + json_path);

    file << j;
}