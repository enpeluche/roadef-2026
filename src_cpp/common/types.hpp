/**
 * @file types.hpp
 * @brief Primitive type aliases and sentinel values.
 * Usage: #include "common/types.hpp"
 */
#pragma once

#include <cstdint>
#include <limits>

/**
 * @brief Discrete time step identifier (guaranteed <= 2).
 */
using Tick = uint8_t;

/**
 * @brief Total number of time steps.
 */
using TickCount = uint8_t;

/**
 * @brief Unique node identifier (guaranteed <= 400).
 */
using NodeId = uint16_t;

/**
 * @brief Sentinel value indicating an unassigned or nonexistent node.
 */
constexpr NodeId INVALID_NODE = std::numeric_limits<NodeId>::max();

/**
 * @brief Total number of nodes in the graph.
 */
using NodeCount = uint16_t;

/**
 * @brief Unique edge identifier (guaranteed <= 2000).
 */
using EdgeId = uint16_t;

/**
 * @brief Sentinel value indicating an unassigned or nonexistent edge.
 */
constexpr EdgeId INVALID_EDGE = std::numeric_limits<EdgeId>::max();

/**
 * @brief Total number of edges in the graph.
 */
using EdgeCount = uint16_t;

/**
 * @brief Edge cost metric for shortest path calculations.
 */
using Weight = uint64_t;

/**
 * @brief Edge traffic capacity limit.
 */
using Capacity = double;

/**
 * @brief Unique identifier for a traffic demand.
 */
using DemandId = uint16_t;

/**
 * @brief Total number of traffic demands.
 */
using DemandCount = uint16_t;