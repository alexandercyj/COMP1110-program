#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "network.h"
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>

/**
 * @file pathfinder.h
 * @brief Pathfinding algorithms for multi-criteria journey planning.
 * 
 * Implements Dijkstra's algorithm for three optimization criteria:
 * - Fastest: minimize total travel time
 * - Cheapest: minimize total fare cost
 * - Fewest transfers: minimize number of line changes
 */

// ---------------------------------------------------------------------------
// Search criteria enum
// ---------------------------------------------------------------------------
enum class SearchCriteria {
    FASTEST,         // Minimize total duration
    CHEAPEST,        // Minimize total cost
    FEWEST_TRANSFERS // Minimize number of line changes
};

// Convert string preference to enum
SearchCriteria parseSearchCriteria(const std::string& preference);

// Convert enum to display string
std::string criteriaToString(SearchCriteria criteria);

// ---------------------------------------------------------------------------
// Dijkstra result structure
// ---------------------------------------------------------------------------
struct DijkstraResult {
    std::vector<std::string> path;           // Stop IDs from origin to destination
    double totalCost;                         // Cost according to criteria
    double totalDuration;                     // Total time in minutes
    double totalFare;                         // Total fare in HKD
    int numTransfers;                         // Number of line changes
    bool reachable;                           // Whether destination was reached
    
    DijkstraResult() : totalCost(0.0), totalDuration(0.0), totalFare(0.0), 
                       numTransfers(0), reachable(false) {}
};

// ---------------------------------------------------------------------------
// Path reconstruction helpers
// ---------------------------------------------------------------------------
struct PathNode {
    std::string stopId;
    std::string lineId;  // Line used to arrive at this stop
    double cost;
    
    PathNode() : cost(0.0) {}
    PathNode(const std::string& sid, const std::string& lid, double c) 
        : stopId(sid), lineId(lid), cost(c) {}
};

// ---------------------------------------------------------------------------
// Main pathfinding functions
// ---------------------------------------------------------------------------

/**
 * @brief Find optimal path using Dijkstra's algorithm.
 * @param network The transport network
 * @param originId Starting stop ID
 * @param destinationId Target stop ID
 * @param criteria Optimization criteria
 * @param userType User type for fare calculation
 * @return DijkstraResult containing path and statistics
 */
DijkstraResult findOptimalPath(
    const TransportNetwork& network,
    const std::string& originId,
    const std::string& destinationId,
    SearchCriteria criteria,
    UserType userType = UserType::ADULT
);

/**
 * @brief Find paths optimized for all three criteria.
 * @param network The transport network
 * @param originId Starting stop ID
 * @param destinationId Target stop ID
 * @param userType User type for fare calculation
 * @return Vector of three results (fastest, cheapest, fewest transfers)
 */
std::vector<DijkstraResult> findAllOptimalPaths(
    const TransportNetwork& network,
    const std::string& originId,
    const std::string& destinationId,
    UserType userType = UserType::ADULT
);

/**
 * @brief Convert DijkstraResult to Journey object.
 */
Journey convertToJourney(
    const DijkstraResult& result,
    const TransportNetwork& network,
    UserType userType = UserType::ADULT
);

#endif // PATHFINDER_H
