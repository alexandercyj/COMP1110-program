#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "network.h"

// Trim whitespace from both ends
std::string trim(const std::string& str);

// Split string by delimiter
std::vector<std::string> split(const std::string& str, char delimiter);

// Convert string to lowercase
std::string toLower(const std::string& s);

// Display a single journey's details
void displayJourney(const std::vector<Segment>& journey, const TransportNetwork& network, int index = -1);

// Display network summary
void displayNetworkSummary(const TransportNetwork& network);

// Display list of journeys with ranking
void displayJourneyList(const std::vector<std::vector<Segment>>& journeys, const TransportNetwork& network, const std::string& preference);

// Resolve user input (stop id or English name) to stop id; returns empty string if not found
std::string getStopIdFromInput(const TransportNetwork& network, const std::string& input);

#endif // UTILS_H
