// tests/test_graph.cpp
// g++ -std=c++17 -Wall -Wextra tests/test_main.cpp tests/test_graph.cpp src_cpp/graph/graph.cpp src_cpp/graph/graph_io.cpp src_cpp/graph/shortest_path_tree.cpp -o run_tests
#include "lib/doctest.h"

#include "../src_cpp/graph/graph.hpp"
#include "../src_cpp/graph/shortest_path_tree.hpp"

TEST_CASE("Graph: construction vide")
{
    Graph g(5);
    CHECK(g.nodes_count() == 5);
    CHECK(g.edges_count() == 0);
}

TEST_CASE("Graph: ajout d'arcs")
{
    Graph g(3);
    uint32_t id1 = g.add_edge(0, 1, 100.0, 1000.0);
    uint32_t id2 = g.add_edge(1, 2, 200.0, 500.0);

    CHECK(g.edges_count() == 2);
    CHECK(id1 == 0);
    CHECK(id2 == 1);

    const Edge &e = g.edge(id1);
    CHECK(e.source_node == 0);
    CHECK(e.target_node == 1);
    CHECK(e.weight == doctest::Approx(100.0));
    CHECK(e.capacity == doctest::Approx(1000.0));
}

TEST_CASE("Graph: degrés")
{
    Graph g(4);
    g.add_edge(0, 1, 1.0, 1.0);
    g.add_edge(0, 2, 1.0, 1.0);
    g.add_edge(1, 3, 1.0, 1.0);

    CHECK(g.out_degree(0) == 2);
    CHECK(g.in_degree(0) == 0);
    CHECK(g.out_degree(1) == 1);
    CHECK(g.in_degree(1) == 1);
    CHECK(g.in_degree(3) == 1);
    CHECK(g.degree(1) == 2);
}

TEST_CASE("Graph: chargement depuis JSON instance toy")
{
    Graph g = Graph::from_json("toy", "00");

    CHECK(g.nodes_count() == 7);
    CHECK(g.edges_count() == 22);
}

TEST_CASE("Dijkstra: distances sur instance toy depuis v0")
{
    Graph g = Graph::from_json("toy", "00");
    auto spt = g.shortest_path_tree(0);

    // Vérifie que toutes les distances sont calculées
    CHECK(spt.distances[0] == doctest::Approx(0.0));
    CHECK(spt.distances[6] < std::numeric_limits<double>::infinity());

    // Vérifie quelques distances précises (basées sur le graphe toy)
    // Ces valeurs dépendent des poids dans ton JSON
    // CHECK(spt.distances[3] == doctest::Approx(200.0));
}

TEST_CASE("Dijkstra: ECMP - plusieurs prédécesseurs au même coût")
{
    // Petit graphe avec ECMP simple :
    //   0 → 1 (poids 1)
    //   0 → 2 (poids 1)
    //   1 → 3 (poids 1)
    //   2 → 3 (poids 1)
    Graph g(4);
    g.add_edge(0, 1, 1.0, 1.0);
    g.add_edge(0, 2, 1.0, 1.0);
    g.add_edge(1, 3, 1.0, 1.0);
    g.add_edge(2, 3, 1.0, 1.0);

    auto spt = g.shortest_path_tree(0);

    CHECK(spt.distances[3] == doctest::Approx(2.0));
    CHECK(spt.get_predecessor_count(3) == 2); // ECMP : 2 chemins
}

TEST_CASE("dijkstra: invariants sur instance toy")
{
    Graph g = Graph::from_json("toy", "00");

    SUBCASE("distance source→source = 0 pour toute source")
    {
        for (uint16_t s = 0; s < g.nodes_count(); ++s)
        {
            auto spt = g.shortest_path_tree(s);
            CHECK(spt.distances[s] == doctest::Approx(0.0));
        }
    }

    SUBCASE("inégalité triangulaire pour tout arc")
    {
        for (uint16_t s = 0; s < g.nodes_count(); ++s)
        {
            auto spt = g.shortest_path_tree(s);
            for (uint32_t a = 0; a < g.edges_count(); ++a)
            {
                const Edge &e = g.edge(a);
                if (spt.distances[e.source_node] < 1e18)
                {
                    CHECK(spt.distances[e.target_node] <= spt.distances[e.source_node] + e.weight + 1e-9);
                }
            }
        }
    }

    SUBCASE("prédécesseurs ECMP cohérents avec distances")
    {
        for (uint16_t s = 0; s < g.nodes_count(); ++s)
        {
            auto spt = g.shortest_path_tree(s);
            for (uint16_t v = 0; v < g.nodes_count(); ++v)
            {
                if (v == s)
                    continue;
                if (spt.distances[v] >= 1e18)
                    continue;

                uint32_t count = spt.get_predecessor_count(v);
                CHECK(count > 0); // si v atteignable et != s, il a un prédécesseur

                const uint16_t *preds = spt.get_predecessors_begin(v);
                for (uint32_t i = 0; i < count; ++i)
                {
                    const Edge &e = g.edge(preds[i]);
                    CHECK(e.target_node == v);
                    CHECK(spt.distances[e.source_node] + e.weight == doctest::Approx(spt.distances[v]));
                }
            }
        }
    }
}
