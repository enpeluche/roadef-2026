// graph/core/graph_io.cpp

// TODO : convertir nlohmann en simdjson

#include "graph/core/edge.hpp"
#include "graph/core/graph.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>

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

    // Étape 1 : Début du parsing
    const auto &nodes = network_data["nodes"];
    const auto &links = network_data["links"];

    Graph g(nodes.size());
    g.all_edges_.reserve(links.size());

    // Création des nœuds
    for (const auto &node : nodes)
        g.node_names_[node["id"].get<uint16_t>()] = node["name"].get<std::string>();

    // Création des arcs avec conversion poids double → uint64
    for (const auto &link : links)
    {
        const double metric_raw = link["metric"].get<double>();
        const uint64_t metric_int = EdgeConsts::to_int(metric_raw);

        g.add_edge(
            link["from"].get<uint16_t>(),
            link["to"].get<uint16_t>(),
            metric_int,
            link["capacity"].get<double>());
    }

    // Mer de bits
    g.num_time_slots_ = tm_data["num_time_slots"];
    g.topology_timeline_.assign(g.num_time_slots_, boost::dynamic_bitset<>(g.all_edges_.size()));
    for (int t = 0; t < g.num_time_slots_; ++t)
        g.topology_timeline_[t].set();

    if (scenario_data.contains("interventions"))
        for (const auto &inter : scenario_data["interventions"])
        {
            int t = inter["t"].get<int>();
            for (int link_id : inter["links"])
                g.topology_timeline_[t].reset(link_id);
        }

    g.freeze();
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
    j["directed"] = true;
    j["multigraph"] = false;

    j["nodes"] = nlohmann::json::array();
    uint16_t n = nodes_count();
    for (uint16_t i = 0; i < n; ++i)
        j["nodes"].push_back({{"name", node_names_[i]}, {"id", i}});

    j["links"] = nlohmann::json::array();
    for (const auto &edge : all_edges_)
    {
        const double metric_double = EdgeConsts::to_double(edge.weight);
        j["links"].push_back({{"id", edge.id},
                              {"from", edge.source},
                              {"to", edge.target},
                              {"metric", metric_double},
                              {"capacity", edge.capacity}});
    }

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
    os << "Graph [|V|=" << graph.nodes_count()
       << ", |E|=" << graph.edges_count()
       << ", d=" << std::fixed << std::setprecision(2) << (graph.density() * 100.0) << "%]\n";

    std::map<uint16_t, uint16_t> in_degree_dist;
    std::map<uint16_t, uint16_t> out_degree_dist;

    for (uint16_t n = 0; n < graph.nodes_count(); ++n)
    {
        in_degree_dist[graph.in_degree(n)]++;
        out_degree_dist[graph.out_degree(n)]++;
    }

    auto print_dist = [&](const std::string &label, const std::map<uint16_t, uint16_t> &dist)
    {
        os << " " << label << ": ";
        bool first = true;
        for (auto const &[deg, freq] : dist)
        {
            if (!first)
                os << " ";
            os << "d=" << deg << "(" << freq << ")";
            first = false;
        }
        os << "\n";
    };

    print_dist("In-Deg ", in_degree_dist);
    print_dist("Out-Deg", out_degree_dist);

    os << " Edges  : ";
    uint16_t limit = std::min(static_cast<uint16_t>(5), graph.edges_count());

    for (uint16_t i = 0; i < limit; ++i)
    {
        const auto &e = graph.edge(i);
        os << e.id << "[" << e.source << "->" << e.target << "]";
        if (i < limit - 1)
            os << " ";
    }

    os << "\n";

    os.unsetf(std::ios_base::floatfield);

    return os;
}