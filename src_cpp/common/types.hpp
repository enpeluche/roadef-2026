/**
 * @file types.hpp
 * @brief Définitions des types primitifs.
 * Usage: #include "common/types.hpp"
 */
#pragma once

#include <cstdint>

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
 * @brief Représente le nombre total de nœuds présents dans le graphe.
 */
using NodeCount = uint16_t;

/**
 * @brief Identifiant unique d'un arc.
 * Les instances ne dépassent pas 2 000 arcs.
 */
using EdgeId = uint16_t;

/**
 * @brief Représente le nombre total d'arcs présents dans le graphe.
 */
using EdgeCount = uint16_t;