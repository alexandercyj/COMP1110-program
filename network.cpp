#include "network.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// TransportNetwork implementation
// ---------------------------------------------------------------------------

void TransportNetwork::addStop(const Stop& stop) {
    stops[stop.id] = stop;
}

void TransportNetwork::addLine(const Line& line) {
    lines[line.id] = line;
}

void TransportNetwork::addSegment(const Segment& seg) {
    segments.push_back(seg);
    adjacency[seg.fromStop].push_back(seg);
}

const Stop* TransportNetwork::getStop(const std::string& id) const {
    auto it = stops.find(id);
    if (it == stops.end()) return nullptr;
    return &it->second;
}

const Line* TransportNetwork::getLine(const std::string& id) const {
    auto it = lines.find(id);
    if (it == lines.end()) return nullptr;
    return &it->second;
}

const std::vector<Segment>& TransportNetwork::getSegmentsFrom(const std::string& fromStop) const {
    static const std::vector<Segment> emptyVec;
    auto it = adjacency.find(fromStop);
    if (it == adjacency.end()) return emptyVec;
    return it->second;
}

const std::unordered_map<std::string, Stop>& TransportNetwork::getAllStops() const {
    return stops;
}

const std::unordered_map<std::string, Line>& TransportNetwork::getAllLines() const {
    return lines;
}

size_t TransportNetwork::getStopCount() const {
    return stops.size();
}

size_t TransportNetwork::getSegmentCount() const {
    return segments.size();
}

size_t TransportNetwork::getLineCount() const {
    return lines.size();
}

// ---------------------------------------------------------------------------
// Helper functions
// ---------------------------------------------------------------------------

std::vector<std::string> getAllStopIds(const TransportNetwork& network) {
    std::vector<std::string> ids;
    for (const auto& kv : network.getAllStops()) {
        ids.push_back(kv.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::string getStopNameById(const TransportNetwork& network, const std::string& id) {
    const Stop* s = network.getStop(id);
    if (!s) return id;
    return s->nameEn.empty() ? s->id : s->nameEn;
}

std::string getStopNameCnById(const TransportNetwork& network, const std::string& id) {
    const Stop* s = network.getStop(id);
    if (!s) return "";
    return s->nameCn;
}

std::string getLineDisplayName(const TransportNetwork& network, const std::string& lineId) {
    const Line* line = network.getLine(lineId);
    if (!line) return lineId;
    return line->name + " (" + lineId + ")";
}
