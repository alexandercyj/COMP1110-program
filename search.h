#ifndef SEARCH_H
#define SEARCH_H

#include <vector>
#include <string>
#include <unordered_set>
#include "network.h"

// Generate candidate journeys using depth-limited DFS.
// Returns a list of journeys (each journey is a vector of Segment).
std::vector<std::vector<Segment>> generateJourneys(const TransportNetwork& network,
                                                   const std::string& originId,
                                                   const std::string& destinationId,
                                                   int maxSegments = 6,
                                                   int maxJourneys = 50);

// DFS helper
void dfs(const TransportNetwork& network,
         const std::string& currentId,
         const std::string& destId,
         std::vector<Segment>& currentJourney,
         std::unordered_set<std::string>& visited,
         std::vector<std::vector<Segment>>& results,
         int depth,
         int maxDepth,
         int& journeyCount,
         int maxJourneys);

#endif // SEARCH_H
