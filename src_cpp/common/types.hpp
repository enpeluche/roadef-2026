/**
 * @file types.hpp
 * @brief Définitions des types primitifs.
 * Usage: #include "common/types.hpp"
 */
#pragma once

#include <cstdint>
#include <limits>

/**
 * @brief Identifiant discret d'un pas de temps.
 * Les instances ne dépassent pas 2 ticks.
 */
using Tick = uint8_t;

/**
 * @brief L'unité du temps.
 */
using TickCount = uint8_t;

/**
 * @brief Identifiant unique d'un nœud.
 * Les instances ne dépassent pas 400 noeuds.
 */
using NodeId = uint16_t;

/**
 * @brief Nœud inexistant ou invalide.
 */
constexpr NodeId INVALID_NODE = std::numeric_limits<NodeId>::max();

/**
 * @brief Représente le nombre total de nœuds présents dans le graphe.
 */
using NodeCount = uint16_t;

/**
 * @brief Identifiant unique d'un arc.
 * Les instances ne dépassent pas 2 000 arcs.
 */
using EdgeId = uint16_t;

/**
 * @brief Arc inexistant ou invalide.
 */
constexpr EdgeId INVALID_EDGE = std::numeric_limits<EdgeId>::max();

/**
 * @brief Représente le nombre total d'arcs présents dans le graphe.
 */
using EdgeCount = uint16_t;

/**
 * @brief Représente le coût/poids d'un arc.
 * Utilisé pour le calcul des plus courts chemins (PCC).
 */
using Weight = uint64_t;

/**
 * @brief Représente la capacité de trafic d'un arc.
 */
using Capacity = double;