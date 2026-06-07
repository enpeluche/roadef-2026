/**
 * @file demand_base.hpp
 * Usage: #include "traffic/demand_base.hpp"
 */

#pragma once

#include "common/types.hpp"

/**
 * @brief Routing endpoints and precomputed metrics for a single traffic demand.
 */
struct DemandBase
{
    DemandId id;
    NodeId source;
    NodeId target;
    float n2; ///< L2 norm of the demand's volume vector over all time slots.
};