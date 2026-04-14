#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <unordered_map>

struct Stop {
    std::string id;
    std::string nameEn;
    std::string nameCn;
    int x;
    int y;
    bool isAccessible;
};

struct Segment {
    std::string fromStop;
    std::string toStop;
    std::string lineId;
    double duration; // minutes
    double cost;     // HKD
    std::string mode;
};

class TransportNetwork {
public:
    TransportNetwork() = default;

    // Add a stop to the network
    void addStop(const Stop& stop);

    // Add a segment to the network
    void addSegment(const Segment& seg);

    // Get pointer to stop or nullptr if not found
    const Stop* getStop(const std::string& id) const;

    // Get segments originating from a stop id (may be empty)
    const std::vector<Segment>& getSegmentsFrom(const std::string& fromStop) const;

    // Access all stops
    const std::unordered_map<std::string, Stop>& getAllStops() const;

    size_t getStopCount() const;
    size_t getSegmentCount() const;

private:
    std::unordered_map<std::string, Stop> stops;
    std::vector<Segment> segments;
    std::unordered_map<std::string, std::vector<Segment>> adjacency;
};

// Loads network from file. Throws std::runtime_error if file cannot be opened.
TransportNetwork loadFromFile(const std::string& filepath);

// Returns sorted list of stop ids
std::vector<std::string> getAllStopIds(const TransportNetwork& network);

// Returns English name or id if not found
std::string getStopNameById(const TransportNetwork& network, const std::string& id);

#endif // NETWORK_H
