#pragma once

#include <vector>
#include <string>
#include "network.h"

struct JourneyStats {
    double totalDuration;
    double totalCost;
    int numSegments;
};

// Compute stats for a given journey
JourneyStats computeJourneyStats(const std::vector<Segment>& journey);

// Rank journeys according to preference: "cheapest", "fastest", "fewest_segments"
std::vector<std::vector<Segment>> rankJourneys(const std::vector<std::vector<Segment>>& journeys,
                                               const std::string& preference);
