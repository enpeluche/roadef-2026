// graph_io.cpp

#include "graph.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <limits>
#include <queue>

using json = nlohmann::json;

Graph::Graph(const std::string &dataset, const std::string &instance_id)
{
    std::string json_path = "instances/" + dataset + "/" + dataset + "-" + instance_id + "-net.json";
    std::ifstream file(json_path);

    if (!file.is_open())
        throw std::runtime_error("Impossible d'ouvrir le fichier :" + json_path);

    json data = json::parse(file);

    const auto &nodes = data["nodes"];
    int num_nodes = nodes.size();

    in_edges_.assign(num_nodes, std::vector<int>());
    out_edges_.assign(num_nodes, std::vector<int>());

    for (const auto &node : nodes)
    {
        int id = node["id"].get<int>();
        std::string name = node["name"].get<std::string>();
    }

    const auto &links = data["links"];

    for (const auto &link : links)
        add_edge(link["from"].get<int>(), link["to"].get<int>(), link["metric"].get<double>(), link["capacity"].get<double>());
}
