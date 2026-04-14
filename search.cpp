#include "search.h"
#include <iostream>

// DFS helper implementation
void dfs(const TransportNetwork& network,
         const std::string& currentId,
         const std::string& destId,
         std::vector<Segment>& currentJourney,
         std::unordered_set<std::string>& visited,
         std::vector<std::vector<Segment>>& results,
         int depth,
         int maxDepth,
         int& journeyCount,
         int maxJourneys) {
    if (journeyCount >= maxJourneys) return;
    if (depth > maxDepth) return;

    if (currentId == destId) {
        // Found a journey
        results.push_back(currentJourney);
        ++journeyCount;
        return;
    }

    // Explore outgoing segments
    const auto& edges = network.getSegmentsFrom(currentId);
    for (const auto& seg : edges) {
        if (journeyCount >= maxJourneys) break;
        const std::string& nextId = seg.toStop;
        if (visited.count(nextId)) continue;
        // Take this edge
        visited.insert(nextId);
        currentJourney.push_back(seg);

        dfs(network, nextId, destId, currentJourney, visited, results, depth + 1, maxDepth, journeyCount, maxJourneys);

        // Backtrack
        currentJourney.pop_back();
        visited.erase(nextId);
    }
}

// GenerateJourneys implementation
std::vector<std::vector<Segment>> generateJourneys(const TransportNetwork& network,
                                                   const std::string& originId,
                                                   const std::string& destinationId,
                                                   int maxSegments,
                                                   int maxJourneys) {
    std::vector<std::vector<Segment>> results;
    std::unordered_set<std::string> visited;
    std::vector<Segment> currentJourney;
    int journeyCount = 0;

    visited.insert(originId);
    dfs(network, originId, destinationId, currentJourney, visited, results, 0, maxSegments, journeyCount, maxJourneys);

    return results;
}