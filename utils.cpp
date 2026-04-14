#include "utils.h"
#include "ranking.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <iostream>

// Trim implementation
std::string trim(const std::string& str) {
    const char* ws = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(ws);
    return str.substr(start, end - start + 1);
}

// Split by delimiter
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> out;
    std::string token;
    std::istringstream ss(str);
    while (std::getline(ss, token, delimiter)) {
        out.push_back(token);
    }
    return out;
}

// toLower
std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::tolower(c); });
    return out;
}

// Display single journey
void displayJourney(const std::vector<Segment>& journey, const TransportNetwork& network, int index) {
    JourneyStats stats{0.0, 0.0, static_cast<int>(journey.size())};
    std::ostringstream route;
    if (!journey.empty()) {
        // Start from first segment's fromStop
        std::string startId = journey.front().fromStop;
        route << getStopNameById(network, startId) << " (" << startId << ")";
        for (const auto& seg : journey) {
            route << " -> " << getStopNameById(network, seg.toStop) << " (" << seg.toStop << ")";
            stats.totalDuration += seg.duration;
            stats.totalCost += seg.cost;
        }
    } else {
        route << "(no segments)";
    }

    if (index >= 0) {
        std::cout << "Journey #" << (index + 1) << ":\n";
    } else {
        std::cout << "Journey:\n";
    }
    std::cout << "  Route: " << route.str() << "\n";
    std::cout << "  Total Duration: " << std::fixed << std::setprecision(1) << stats.totalDuration << " min\n";
    std::cout << "  Total Cost: HK$ " << std::fixed << std::setprecision(2) << stats.totalCost << "\n";
    std::cout << "  Segments: " << stats.numSegments << "\n\n";
}

// Display network summary
void displayNetworkSummary(const TransportNetwork& network) {
    std::cout << "Stops: " << network.getStopCount() << "\n";
    std::cout << "Segments: " << network.getSegmentCount() << "\n";
    std::cout << "Stop list:\n";
    auto ids = getAllStopIds(network);
    for (const auto& id : ids) {
        std::cout << id << " - " << getStopNameById(network, id) << "\n";
    }
}

// Display list of journeys with ranking metric
void displayJourneyList(const std::vector<std::vector<Segment>>& journeys, const TransportNetwork& network, const std::string& preference) {
    std::string metric;
    if (preference == "cheapest") metric = "CHEAPEST";
    else if (preference == "fastest") metric = "FASTEST";
    else if (preference == "fewest_segments") metric = "FEWEST_SEGMENTS";
    else metric = preference;

    std::cout << "Ranking journeys by " << metric << ":\n";
    for (size_t i = 0; i < journeys.size(); ++i) {
        displayJourney(journeys[i], network, static_cast<int>(i));
    }
}

// Resolve input to stop id
std::string getStopIdFromInput(const TransportNetwork& network, const std::string& input) {
    if (input.empty()) return "";
    std::string trimmed = trim(input);
    // First, check if it's an exact ID
    if (network.getStop(trimmed) != nullptr) return trimmed;

    // Otherwise try matching English name (case-insensitive)
    std::string low = toLower(trimmed);
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (toLower(stop.nameEn) == low) return stop.id;
    }

    // Try partial match (contain)
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (toLower(stop.nameEn).find(low) != std::string::npos) return stop.id;
    }

    return "";
}