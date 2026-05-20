#pragma once

#include "common/types.hpp"
#include "traffic/demand_base.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

/**
 * @brief Représente un chemin Segment Routing pour une demande à un time slot.
 *
 * Un SRPath décrit le routage d'une demande : la source et la destination sont
 * implicites (via `demand`), seuls les waypoints intermédiaires sont stockés.
 * Le routage entre deux waypoints consécutifs utilise l'ECMP sur le SPT.
 *
 * Taille mémoire : 24 octets (alignée).
 *
 * @note La taille fixe `waypoints[MAX_WAYPOINTS]` limite à MAX_WAYPOINTS waypoints.
 */
struct SRPath
{
    static constexpr uint8_t MAX_WAYPOINTS = 10;

    const DemandBase *demand;        ///< Métadonnées de la demande (source, target).
    NodeId waypoints[MAX_WAYPOINTS]; ///< Waypoints intermédiaires (source et target exclus).
    uint8_t num_waypoints;           ///< Nombre de waypoints effectivement utilisés.
    Tick t;                          ///< Time slot du chemin.

    /**
     * @brief Constructeur par défaut. SRPath vide, sans demande associée.
     */
    SRPath() : demand(nullptr), waypoints{}, num_waypoints(0), t(0) {}

    /**
     * @brief Construit un SRPath pour une demande, un time slot et une liste de waypoints.
     *
     * Si la liste dépasse MAX_WAYPOINTS, seuls les premiers sont conservés.
     *
     * @param d     Pointeur vers la demande.
     * @param slot  Time slot.
     * @param w     Liste des waypoints (par défaut vide = routage direct source->target).
     */
    SRPath(const DemandBase *d, Tick slot, const std::vector<NodeId> &w = {})
        : demand(d), waypoints{}, num_waypoints(0), t(slot)
    {
        num_waypoints = static_cast<uint8_t>(std::min<size_t>(w.size(), MAX_WAYPOINTS));
        for (uint8_t i = 0; i < num_waypoints; ++i)
            waypoints[i] = w[i];
    }

    /**
     * @brief Nombre de segments du chemin (= num_waypoints + 1).
     */
    uint8_t num_segments() const { return num_waypoints + 1; }

    /**
     * @brief Distance ROADEF entre deux SRPath.
     *
     * Compte les segments présents dans un seul des deux chemins (différence symétrique).
     * Les deux chemins doivent appartenir à la même demande (même source, target).
     *
     * @param a Premier SRPath.
     * @param b Second SRPath.
     * @return Nombre de segments distincts. 0 si les deux chemins sont identiques.
     *
     * @note Complexité O((MAX_WAYPOINTS+1)^2). Aucune allocation.
     */
    static uint8_t distance(const SRPath &a, const SRPath &b)
    {
        if (!a.demand || !b.demand)
            return 0;

        struct Segment
        {
            uint16_t u, v;
        };

        auto build = [](const SRPath &p, Segment *out) -> uint8_t
        {
            uint16_t cur = p.demand->source;
            uint8_t n = 0;
            for (uint8_t i = 0; i < p.num_waypoints; ++i)
            {
                out[n++] = {cur, p.waypoints[i]};
                cur = p.waypoints[i];
            }
            out[n++] = {cur, p.demand->target};
            return n;
        };

        Segment sa[MAX_WAYPOINTS + 1];
        Segment sb[MAX_WAYPOINTS + 1];
        const uint8_t na = build(a, sa);
        const uint8_t nb = build(b, sb);

        auto contains = [](const Segment &s, const Segment *set, uint8_t n)
        {
            for (uint8_t i = 0; i < n; ++i)
                if (set[i].u == s.u && set[i].v == s.v)
                    return true;
            return false;
        };

        uint8_t diff = 0;
        for (uint8_t i = 0; i < na; ++i)
            if (!contains(sa[i], sb, nb))
                ++diff;
        for (uint8_t i = 0; i < nb; ++i)
            if (!contains(sb[i], sa, na))
                ++diff;

        return diff;
    }

    /**
     * @brief Distance vers un autre SRPath (méthode d'instance).
     */
    uint8_t distance_to(const SRPath &other) const { return distance(*this, other); }

    /**
     * @brief Affichage pour le debug.
     * Format : SRPath(D:<id>, T:<slot>, W:[w1, w2, ...])
     */
    friend std::ostream &operator<<(std::ostream &os, const SRPath &p)
    {
        if (!p.demand)
            return os << "SRPath(Empty)";

        os << "SRPath(D:" << p.demand->id << ", T:" << static_cast<int>(p.t);
        if (p.num_waypoints > 0)
        {
            os << ", W:[";
            for (uint8_t i = 0; i < p.num_waypoints; ++i)
                os << p.waypoints[i] << (i + 1 < p.num_waypoints ? ", " : "");
            os << "]";
        }
        os << ")";
        return os;
    }
};