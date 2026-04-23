#include "pathfinder.h"
#include "utils.h"
#include <algorithm>
#include <iostream>

// ---------------------------------------------------------------------------
// SearchCriteria helpers
// ---------------------------------------------------------------------------

SearchCriteria parseSearchCriteria(const std::string& preference) {
    std::string p = toLower(trim(preference));
    if (p == "fastest" || p == "time" || p == " quickest") {
        return SearchCriteria::FASTEST;
    } else if (p == "cheapest" || p == "cost" || p == "price" || p == "money") {
        return SearchCriteria::CHEAPEST;
    } else if (p == "fewest_transfers" || p == "transfers" || p == "fewest" || p == "transfer") {
        return SearchCriteria::FEWEST_TRANSFERS;
    }
    // Default to fastest
    return SearchCriteria::FASTEST;
}

std::string criteriaToString(SearchCriteria criteria) {
    switch (criteria) {
        case SearchCriteria::FASTEST: return "Fastest (Minimum Time)";
        case SearchCriteria::CHEAPEST: return "Cheapest (Minimum Cost)";
        case SearchCriteria::FEWEST_TRANSFERS: return "Fewest Transfers";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// Edge cost functions
// ---------------------------------------------------------------------------

// Get edge weight based on criteria
static double getEdgeWeight(const Segment& seg, SearchCriteria criteria, UserType userType) {
    switch (criteria) {
        case SearchCriteria::FASTEST:
            return seg.duration;
        case SearchCriteria::CHEAPEST:
            return seg.getCost(userType);
        case SearchCriteria::FEWEST_TRANSFERS:
            // For fewest transfers, we use a small base cost plus a large penalty for changing lines
            // This will be handled at the node level, not edge level
            return 1.0;  // Each segment counts as 1
        default:
            return seg.duration;
    }
}

// ---------------------------------------------------------------------------
// Dijkstra's algorithm implementation
// ---------------------------------------------------------------------------

DijkstraResult findOptimalPath(
    const TransportNetwork& network,
    const std::string& originId,
    const std::string& destinationId,
    SearchCriteria criteria,
    UserType userType
) {
    DijkstraResult result;
    result.reachable = false;
    
    // Validate origin and destination
    if (!network.getStop(originId)) {
        std::cerr << "Error: Origin stop '" << originId << "' not found.\n";
        return result;
    }
    if (!network.getStop(destinationId)) {
        std::cerr << "Error: Destination stop '" << destinationId << "' not found.\n";
        return result;
    }
    
    if (originId == destinationId) {
        result.path = {originId};
        result.reachable = true;
        return result;
    }
    
    // Priority queue: (cost, stopId, lineId)
    // We track lineId to detect transfers
    using QueueItem = std::tuple<double, std::string, std::string>;
    std::priority_queue<
        QueueItem,
        std::vector<QueueItem>,
        std::greater<QueueItem>
    > pq;
    
    // Distance map: stopId -> minimum cost
    std::unordered_map<std::string, double> dist;
    
    // Previous node map for path reconstruction: stopId -> (prevStopId, lineId)
    std::unordered_map<std::string, std::pair<std::string, std::string>> prev;
    
    // Track which line we arrived on for each stop
    std::unordered_map<std::string, std::string> arrivalLine;
    
    // Initialize
    dist[originId] = 0.0;
    arrivalLine[originId] = "";
    pq.push({0.0, originId, ""});
    
    while (!pq.empty()) {
        QueueItem item = pq.top();
        double currentCost = std::get<0>(item);
        std::string currentStop = std::get<1>(item);
        std::string currentLine = std::get<2>(item);
        pq.pop();
        
        // If we've already found a better path, skip
        if (dist.count(currentStop) && currentCost > dist[currentStop] + 1e-9) {
            continue;
        }
        
        // Check if we reached the destination
        if (currentStop == destinationId) {
            break;
        }
        
        // Explore neighbors
        const auto& edges = network.getSegmentsFrom(currentStop);
        for (const auto& seg : edges) {
            const std::string& nextStop = seg.toStop;
            
            // Calculate edge weight
            double edgeWeight = getEdgeWeight(seg, criteria, userType);
            
            // For fewest transfers, add penalty for line changes
            double transferPenalty = 0.0;
            if (criteria == SearchCriteria::FEWEST_TRANSFERS) {
                if (!currentLine.empty() && seg.lineId != currentLine) {
                    transferPenalty = 1000.0;  // Large penalty for transfer
                }
            }
            
            double newCost = currentCost + edgeWeight + transferPenalty;
            
            // Relaxation
            if (!dist.count(nextStop) || newCost < dist[nextStop] - 1e-9) {
                dist[nextStop] = newCost;
                prev[nextStop] = {currentStop, seg.lineId};
                arrivalLine[nextStop] = seg.lineId;
                pq.push({newCost, nextStop, seg.lineId});
            }
        }
    }
    
    // Check if destination is reachable
    if (!dist.count(destinationId)) {
        return result;  // Not reachable
    }
    
    // Reconstruct path
    std::vector<std::string> path;
    std::string current = destinationId;
    while (current != originId) {
        path.push_back(current);
        auto it = prev.find(current);
        if (it == prev.end()) {
            // Path broken
            return result;
        }
        current = it->second.first;
    }
    path.push_back(originId);
    std::reverse(path.begin(), path.end());
    
    // Build result
    result.path = path;
    result.reachable = true;
    result.totalCost = dist[destinationId];
    
    // Calculate detailed stats
    result.totalDuration = 0.0;
    result.totalFare = 0.0;
    result.numTransfers = 0;
    
    std::string prevLine = "";
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const std::string& fromStop = path[i];
        const std::string& toStop = path[i + 1];
        
        // Find the segment
        const auto& edges = network.getSegmentsFrom(fromStop);
        for (const auto& seg : edges) {
            if (seg.toStop == toStop) {
                result.totalDuration += seg.duration;
                result.totalFare += seg.getCost(userType);
                
                if (!prevLine.empty() && seg.lineId != prevLine) {
                    ++result.numTransfers;
                }
                prevLine = seg.lineId;
                break;
            }
        }
    }
    
    return result;
}

// ---------------------------------------------------------------------------
// Find all three optimal paths
// ---------------------------------------------------------------------------

std::vector<DijkstraResult> findAllOptimalPaths(
    const TransportNetwork& network,
    const std::string& originId,
    const std::string& destinationId,
    UserType userType
) {
    std::vector<DijkstraResult> results;
    
    // Find fastest path
    auto fastest = findOptimalPath(network, originId, destinationId, 
                                   SearchCriteria::FASTEST, userType);
    if (fastest.reachable) {
        results.push_back(fastest);
    }
    
    // Find cheapest path
    auto cheapest = findOptimalPath(network, originId, destinationId, 
                                    SearchCriteria::CHEAPEST, userType);
    if (cheapest.reachable) {
        results.push_back(cheapest);
    }
    
    // Find fewest transfers path
    auto fewest = findOptimalPath(network, originId, destinationId, 
                                  SearchCriteria::FEWEST_TRANSFERS, userType);
    if (fewest.reachable) {
        results.push_back(fewest);
    }
    
    return results;
}

// ---------------------------------------------------------------------------
// Convert DijkstraResult to Journey
// ---------------------------------------------------------------------------

Journey convertToJourney(
    const DijkstraResult& result,
    const TransportNetwork& network,
    UserType userType
) {
    Journey journey;
    
    if (!result.reachable || result.path.size() < 2) {
        return journey;
    }
    
    // Build segments from path
    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        const std::string& fromStop = result.path[i];
        const std::string& toStop = result.path[i + 1];
        
        // Find matching segment
        const auto& edges = network.getSegmentsFrom(fromStop);
        for (const auto& seg : edges) {
            if (seg.toStop == toStop) {
                journey.segments.push_back(seg);
                break;
            }
        }
    }
    
    // Compute stats
    journey.computeStats(userType);
    
    return journey;
}
