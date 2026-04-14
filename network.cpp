#include "network.h"
#include "utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

// Add a stop to the network
void TransportNetwork::addStop(const Stop& stop) {
    stops[stop.id] = stop;
}

// Add a segment to the network
void TransportNetwork::addSegment(const Segment& seg) {
    segments.push_back(seg);
    adjacency[seg.fromStop].push_back(seg);
}

// Get pointer to stop or nullptr if not found
const Stop* TransportNetwork::getStop(const std::string& id) const {
    auto it = stops.find(id);
    if (it == stops.end()) return nullptr;
    return &it->second;
}

// Get segments originating from a stop id (may be empty)
const std::vector<Segment>& TransportNetwork::getSegmentsFrom(const std::string& fromStop) const {
    static const std::vector<Segment> emptyVec;
    auto it = adjacency.find(fromStop);
    if (it == adjacency.end()) return emptyVec;
    return it->second;
}

// Access all stops
const std::unordered_map<std::string, Stop>& TransportNetwork::getAllStops() const {
    return stops;
}

size_t TransportNetwork::getStopCount() const {
    return stops.size();
}

size_t TransportNetwork::getSegmentCount() const {
    return segments.size();
}

// Helper to parse boolean from "true"/"false" (case-insensitive)
static bool parseBool(const std::string& s) {
    std::string t = toLower(trim(s));
    return (t == "true" || t == "1");
}

// Helper to parse double; returns pair(ok, value)
static std::pair<bool,double> tryParseDouble(const std::string& s) {
    std::string t = trim(s);
    if (t.empty() || t == "N/A" || t == "n/a") return {false, 0.0};
    try {
        double v = std::stod(t);
        return {true, v};
    } catch (...) {
        return {false, 0.0};
    }
}

// Load network from file - throws runtime_error if cannot open
TransportNetwork loadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Unable to open file: " + filepath);
    }

    TransportNetwork net;
    std::string line;
    size_t lineNo = 0;
    while (std::getline(ifs, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;
        if (line.size() > 0 && line[0] == '#') continue;

        auto tokens = split(line, '|');
        if (tokens.empty()) continue;

        std::string type = tokens[0];
        if (type == "STOP") {
            if (tokens.size() != 7) {
                std::cerr << "Malformed STOP line " << lineNo << ": wrong number of fields\n";
                continue;
            }
            Stop s;
            s.id = trim(tokens[1]);
            s.nameCn = trim(tokens[2]);
            s.nameEn = trim(tokens[3]);
            try {
                s.x = std::stoi(trim(tokens[4]));
                s.y = std::stoi(trim(tokens[5]));
            } catch (...) {
                std::cerr << "Malformed STOP coordinates at line " << lineNo << "\n";
                continue;
            }
            s.isAccessible = parseBool(tokens[6]);
            if (s.id.empty()) {
                std::cerr << "Empty STOP id at line " << lineNo << "\n";
                continue;
            }
            net.addStop(s);
        } else if (type == "LINE") {
            // LINE token present; optional to store. Skip or ignore malformed.
            if (tokens.size() < 2) {
                std::cerr << "Malformed LINE line " << lineNo << "\n";
            }
            // Ignored for core functionality.
        } else if (type == "SEGMENT") {
            // Support both formats:
            //  - legacy 7-field: SEGMENT|from|to|line|duration|cost|mode
            //  - extended 8-field: SEGMENT|from|to|line|duration|cost_adult|cost_student|mode
            if (tokens.size() < 6) {
                std::cerr << "Malformed SEGMENT line " << lineNo << ": wrong number of fields\n";
                continue;
            }
            Segment seg;
            seg.fromStop = trim(tokens[1]);
            seg.toStop = trim(tokens[2]);
            seg.lineId = trim(tokens[3]);

            // duration is expected at tokens[4]
            auto durRes = tryParseDouble(tokens[4]);
            if (!durRes.first) {
                std::cerr << "Malformed SEGMENT duration at line " << lineNo << "\n";
                continue;
            }
            seg.duration = durRes.second;

            // cost: prefer adult cost at tokens[5]; if not numeric, set to 0 and warn
            auto costRes = tryParseDouble(tokens[5]);
            if (!costRes.first) {
                std::cerr << "Warning: SEGMENT cost not numeric at line " << lineNo << " (using 0)\n";
                seg.cost = 0.0;
            } else {
                seg.cost = costRes.second;
            }

            // mode is expected to be the last token (handles both 7- and 8-field formats)
            seg.mode = trim(tokens.back());

            if (seg.fromStop.empty() || seg.toStop.empty()) {
                std::cerr << "SEGMENT missing stop id at line " << lineNo << "\n";
                continue;
            }
            net.addSegment(seg);
        } else {
            std::cerr << "Unknown record type at line " << lineNo << ": " << type << "\n";
            continue;
        }
    }

    return net;
}

// Returns sorted list of stop ids
std::vector<std::string> getAllStopIds(const TransportNetwork& network) {
    std::vector<std::string> ids;
    for (const auto& kv : network.getAllStops()) {
        ids.push_back(kv.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

// Returns English name or id if not found
std::string getStopNameById(const TransportNetwork& network, const std::string& id) {
    const Stop* s = network.getStop(id);
    if (!s) return id;
    return s->nameEn.empty() ? s->id : s->nameEn;
}