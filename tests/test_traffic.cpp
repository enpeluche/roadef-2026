// tests/test_traffic.cpp
#include "lib/doctest.h"
#include "../src_cpp/traffic/traffic_data.hpp"
#include <cmath>

TEST_CASE("TrafficData: chargement instance toy-00")
{
    TrafficData td = TrafficData::load("toy", "00");

    CHECK(td.demands_count() == 2);
    CHECK(td.slots_count() == 2);
}

TEST_CASE("TrafficData: volumes corrects sur toy-00")
{
    TrafficData td = TrafficData::load("toy", "00");

    // Demande 0 : v = [50, 100], s=0, t=5
    CHECK(td.volume(0, 0) == doctest::Approx(50.0));
    CHECK(td.volume(0, 1) == doctest::Approx(100.0));

    // Demande 1 : v = [100, 50], s=2, t=5
    CHECK(td.volume(1, 0) == doctest::Approx(100.0));
    CHECK(td.volume(1, 1) == doctest::Approx(50.0));
}

TEST_CASE("TrafficData: source et target corrects sur toy-00")
{
    TrafficData td = TrafficData::load("toy", "00");

    const DemandBase &d0 = td.get_info(0);
    CHECK(d0.id == 0);
    CHECK(d0.source == 0);
    CHECK(d0.target == 5);

    const DemandBase &d1 = td.get_info(1);
    CHECK(d1.id == 1);
    CHECK(d1.source == 2);
    CHECK(d1.target == 5);
}

TEST_CASE("TrafficData: normes calculées correctement sur toy-00")
{
    TrafficData td = TrafficData::load("toy", "00");

    SUBCASE("demande 0 : v = [50, 100]")
    {
        const DemandBase &d = td.get_info(0);
        CHECK(d.n1 == doctest::Approx(150.0));                                  // 50 + 100
        CHECK(d.n2 == doctest::Approx(std::sqrt(50.0 * 50.0 + 100.0 * 100.0))); // ~111.80
        CHECK(d.ninf == doctest::Approx(100.0));                                // max(50, 100)
    }

    SUBCASE("demande 1 : v = [100, 50]")
    {
        const DemandBase &d = td.get_info(1);
        CHECK(d.n1 == doctest::Approx(150.0));                                  // 100 + 50
        CHECK(d.n2 == doctest::Approx(std::sqrt(100.0 * 100.0 + 50.0 * 50.0))); // ~111.80
        CHECK(d.ninf == doctest::Approx(100.0));                                // max(100, 50)
    }
}

TEST_CASE("TrafficData: invariants généraux sur toy-00")
{
    TrafficData td = TrafficData::load("toy", "00");

    SUBCASE("n1 = somme des volumes")
    {
        for (size_t i = 0; i < td.demands_count(); ++i)
        {
            double sum = 0;
            for (uint16_t t = 0; t < td.slots_count(); ++t)
            {
                sum += td.volume(static_cast<uint16_t>(i), t);
            }
            CHECK(td.get_info(static_cast<uint16_t>(i)).n1 == doctest::Approx(sum));
        }
    }

    SUBCASE("ninf >= max des volumes")
    {
        for (size_t i = 0; i < td.demands_count(); ++i)
        {
            double max_v = 0;
            for (uint16_t t = 0; t < td.slots_count(); ++t)
            {
                double v = td.volume(static_cast<uint16_t>(i), t);
                if (v > max_v)
                    max_v = v;
            }
            CHECK(td.get_info(static_cast<uint16_t>(i)).ninf == doctest::Approx(max_v));
        }
    }

    SUBCASE("n1 >= ninf (toujours vrai pour valeurs positives)")
    {
        for (size_t i = 0; i < td.demands_count(); ++i)
        {
            const DemandBase &d = td.get_info(static_cast<uint16_t>(i));
            CHECK(d.n1 >= d.ninf - 1e-9);
        }
    }

    SUBCASE("n2 >= ninf (Cauchy-Schwarz)")
    {
        for (size_t i = 0; i < td.demands_count(); ++i)
        {
            const DemandBase &d = td.get_info(static_cast<uint16_t>(i));
            CHECK(d.n2 >= d.ninf - 1e-9);
        }
    }
}