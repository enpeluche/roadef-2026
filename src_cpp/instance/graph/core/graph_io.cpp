/**
 * @file graph_io.cpp
 *
 * @todo nlohmann -> simdjson
 */

#include "instance/graph/core/edge.hpp"
#include "instance/graph/core/graph.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Construit un graphe du challenge ROADEF depuis un fichier json.
 * Va chercher les instances dans le dossier instances.
 *
 * @param dataset Le nom du dataset par exemple 'setA'
 * @param id L'identifiant de l'instance en chaine de caractère.
 */
Graph Graph::load(const std::string &dataset, const std::string &id)
{
    // --- 0. Lecture des fichiers ------------------------------------------------------

    std::string base_path = "instances/" + dataset + "/" + dataset + "-" + id;

    auto load_json = [](const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("Impossible d'ouvrir le fichier : " + filepath);
        return json::parse(file);
    };

    json network_data = load_json(base_path + "-net.json");

    // --- 1. Parsing des fichiers ------------------------------------------------------

    const auto &nodes = network_data["nodes"];
    const auto &links = network_data["links"];

    Graph graph(nodes.size());
    graph.all_edges_.reserve(links.size());

    for (const auto &node : nodes)
        graph.node_names_[node["id"].get<NodeId>()] = node["name"].get<std::string>();

    for (const auto &link : links)
    {
        const double metric_raw = link["metric"].get<double>();
        const Weight metric_int = EdgeConsts::to_int(metric_raw);

        graph.add_edge(
            link["from"].get<NodeId>(),
            link["to"].get<NodeId>(),
            metric_int,
            link["capacity"].get<Capacity>());
    }

    graph.freeze();
    return graph;
}