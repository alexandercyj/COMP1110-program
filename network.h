#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

/**
 * @file network.h
 * @brief Core data structures for the transport network.
 * 
 * Defines Stop, Line, Segment, and TransportNetwork classes.
 * Supports both adult and student pricing, as well as MTR/BUS modes.
 */

// ---------------------------------------------------------------------------
// User type enum for fare calculation
// ---------------------------------------------------------------------------
enum class UserType {
    ADULT,      // Full fare
    STUDENT,    // 50% discount on MTR (or as per segment data)
    ELDERLY,    // 20% discount (80% of adult fare)
    CHILD       // 50% discount
};

// ---------------------------------------------------------------------------
// Stop structure
// ---------------------------------------------------------------------------
struct Stop {
    std::string id;           // Unique stop ID (e.g., "KET")
    std::string nameCn;       // Chinese name
    std::string nameEn;       // English name
    int x;                    // Coordinate X
    int y;                    // Coordinate Y
    bool isAccessible;        // Wheelchair accessible
};

// ---------------------------------------------------------------------------
// Line structure (new - stores line metadata)
// ---------------------------------------------------------------------------
struct Line {
    std::string id;           // Line ID (e.g., "ISL")
    std::string name;         // Line name (e.g., "港島線")
    std::string color;        // Display color (hex)
    std::string mode;         // "MTR" or "BUS"
};

// ---------------------------------------------------------------------------
// Segment structure (edge between two stops)
// ---------------------------------------------------------------------------
struct Segment {
    std::string fromStop;     // Origin stop ID
    std::string toStop;       // Destination stop ID
    std::string lineId;       // Line ID
    double duration;          // Travel time in minutes
    double costAdult;         // Adult fare in HKD
    double costStudent;       // Student fare in HKD (may be N/A for bus)
    std::string mode;         // "MTR" or "BUS"
    
    // Get cost based on user type
    double getCost(UserType userType) const {
        switch (userType) {
            case UserType::STUDENT:
            case UserType::CHILD:
                // If student fare is available (not N/A), use it
                if (costStudent >= 0) return costStudent;
                // Otherwise fall through to adult with discount
                return costAdult * 0.5;
            case UserType::ELDERLY:
                return costAdult * 0.8;
            case UserType::ADULT:
            default:
                return costAdult;
        }
    }
};

// ---------------------------------------------------------------------------
// Journey result structure
// ---------------------------------------------------------------------------
struct Journey {
    std::vector<Segment> segments;
    double totalDuration;
    double totalCost;
    int numTransfers;
    
    Journey() : totalDuration(0.0), totalCost(0.0), numTransfers(0) {}
    
    void computeStats(UserType userType = UserType::ADULT) {
        totalDuration = 0.0;
        totalCost = 0.0;
        numTransfers = 0;
        for (const auto& seg : segments) {
            totalDuration += seg.duration;
            totalCost += seg.getCost(userType);
        }
        // Count transfers (line changes)
        for (size_t i = 1; i < segments.size(); ++i) {
            if (segments[i].lineId != segments[i-1].lineId) {
                ++numTransfers;
            }
        }
    }
};

// ---------------------------------------------------------------------------
// TransportNetwork class
// ---------------------------------------------------------------------------
class TransportNetwork {
public:
    TransportNetwork() = default;

    // Add a stop to the network
    void addStop(const Stop& stop);

    // Add a line to the network
    void addLine(const Line& line);

    // Add a segment to the network
    void addSegment(const Segment& seg);

    // Get pointer to stop or nullptr if not found
    const Stop* getStop(const std::string& id) const;

    // Get line by ID or nullptr if not found
    const Line* getLine(const std::string& id) const;

    // Get segments originating from a stop id (may be empty)
    const std::vector<Segment>& getSegmentsFrom(const std::string& fromStop) const;

    // Access all stops
    const std::unordered_map<std::string, Stop>& getAllStops() const;

    // Access all lines
    const std::unordered_map<std::string, Line>& getAllLines() const;

    size_t getStopCount() const;
    size_t getSegmentCount() const;
    size_t getLineCount() const;

private:
    std::unordered_map<std::string, Stop> stops;
    std::unordered_map<std::string, Line> lines;
    std::vector<Segment> segments;
    std::unordered_map<std::string, std::vector<Segment>> adjacency;
};

// Returns sorted list of stop ids
std::vector<std::string> getAllStopIds(const TransportNetwork& network);

// Returns English name or id if not found
std::string getStopNameById(const TransportNetwork& network, const std::string& id);

// Returns Chinese name or empty if not found
std::string getStopNameCnById(const TransportNetwork& network, const std::string& id);

// Get line display name
std::string getLineDisplayName(const TransportNetwork& network, const std::string& lineId);

#endif // NETWORK_H
