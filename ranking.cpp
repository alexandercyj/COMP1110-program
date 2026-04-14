#include "ranking.h"
#include <algorithm>

// Compute stats for a journey
JourneyStats computeJourneyStats(const std::vector<Segment>& journey) {
    JourneyStats s{0.0, 0.0, 0};
    s.numSegments = static_cast<int>(journey.size());
    for (const auto& seg : journey) {
        s.totalDuration += seg.duration;
        s.totalCost += seg.cost;
    }
    return s;
}

// Rank journeys using stable_sort to preserve relative order on ties
std::vector<std::vector<Segment>> rankJourneys(const std::vector<std::vector<Segment>>& journeys,
                                               const std::string& preference) {
    std::vector<std::vector<Segment>> copy = journeys;
    if (preference == "cheapest") {
        std::stable_sort(copy.begin(), copy.end(), [](const std::vector<Segment>& a, const std::vector<Segment>& b) {
            double costA = 0.0, costB = 0.0;
            for (const auto& s : a) costA += s.cost;
            for (const auto& s : b) costB += s.cost;
            return costA < costB;
        });
    } else if (preference == "fastest") {
        std::stable_sort(copy.begin(), copy.end(), [](const std::vector<Segment>& a, const std::vector<Segment>& b) {
            double durA = 0.0, durB = 0.0;
            for (const auto& s : a) durA += s.duration;
            for (const auto& s : b) durB += s.duration;
            return durA < durB;
        });
    } else if (preference == "fewest_segments") {
        std::stable_sort(copy.begin(), copy.end(), [](const std::vector<Segment>& a, const std::vector<Segment>& b) {
            return a.size() < b.size();
        });
    }
    return copy;
}