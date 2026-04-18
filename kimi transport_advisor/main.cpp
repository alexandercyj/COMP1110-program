#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include "network.h"
#include "pathfinder.h"
#include "ui.h"
#include "file_io.h"
#include "utils.h"

/**
 * @file main.cpp
 * @brief Smart Public Transport Advisor - Main program entry point.
 * 
 * Provides an interactive menu-driven interface for:
 * - Loading transport network data from files
 * - Viewing network summaries
 * - Planning journeys with multiple optimization criteria
 * - Comparing journeys across time, cost, and transfer dimensions
 * 
 * Supports passenger types: Adult, Student, Elderly, Child
 * with appropriate fare discounts.
 */

// ---------------------------------------------------------------------------
// Function prototypes
// ---------------------------------------------------------------------------

void handleLoadNetwork(TransportNetwork& network, bool& networkLoaded);
void handleShowSummary(const TransportNetwork& network, bool networkLoaded);
void handleSingleSearch(const TransportNetwork& network, bool networkLoaded);
void handleCompareSearch(const TransportNetwork& network, bool networkLoaded);
void handleClearScreen();
int getMenuChoice();

// ---------------------------------------------------------------------------
// Main function
// ---------------------------------------------------------------------------

int main() {
    TransportNetwork network;
    bool networkLoaded = false;
    
    std::cout << "\n  Welcome to Smart Public Transport Advisor!\n";
    std::cout << "  Loading default data file (STOP.txt)...\n";
    
    // Try to load default file
    try {
        network = loadFromFile("STOP.txt");
        networkLoaded = true;
        std::cout << "  Default data loaded successfully!\n";
    } catch (const std::exception& ex) {
        std::cout << "  Note: Could not load default file. " << ex.what() << "\n";
        std::cout << "  Please load a data file manually from the menu.\n";
    }
    
    // Main program loop
    while (true) {
        displayMainMenu();
        
        int choice = getMenuChoice();
        
        switch (choice) {
            case 1:
                handleLoadNetwork(network, networkLoaded);
                break;
            case 2:
                handleShowSummary(network, networkLoaded);
                break;
            case 3:
                handleSingleSearch(network, networkLoaded);
                break;
            case 4:
                handleCompareSearch(network, networkLoaded);
                break;
            case 5:
                handleClearScreen();
                break;
            case 6:
                std::cout << "\n  Thank you for using Smart Public Transport Advisor!\n";
                std::cout << "  Goodbye!\n\n";
                return 0;
            default:
                std::cout << "\n  Invalid choice. Please enter a number between 1 and 6.\n";
                break;
        }
        
        if (choice != 5 && choice != 6) {
            promptContinue();
        }
    }
    
    return 0;
}

// ---------------------------------------------------------------------------
// Menu input helper
// ---------------------------------------------------------------------------

int getMenuChoice() {
    std::cout << "  Enter your choice (1-6): ";
    
    std::string input;
    std::getline(std::cin, input);
    
    // Handle empty input
    if (input.empty()) {
        return -1;
    }
    
    // Handle non-numeric input
    try {
        size_t pos;
        int choice = std::stoi(input, &pos);
        if (pos != input.length()) {
            return -1;  // Extra characters after number
        }
        return choice;
    } catch (...) {
        return -1;
    }
}

// ---------------------------------------------------------------------------
// Menu handlers
// ---------------------------------------------------------------------------

void handleLoadNetwork(TransportNetwork& network, bool& networkLoaded) {
    std::cout << "\n";
    displaySeparator('=', 50);
    std::cout << "  LOAD NETWORK DATA\n";
    displaySeparator('=', 50);
    
    std::cout << "  Enter file path (or press Enter for 'STOP.txt'): ";
    std::string path;
    std::getline(std::cin, path);
    
    if (path.empty()) {
        path = "STOP.txt";
    }
    
    try {
        network = loadFromFile(path);
        networkLoaded = true;
        std::cout << "\n  Success! Network loaded.\n";
        std::cout << "  Stops: " << network.getStopCount() << "\n";
        std::cout << "  Lines: " << network.getLineCount() << "\n";
        std::cout << "  Segments: " << network.getSegmentCount() << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "\n  Error loading file: " << ex.what() << "\n";
        std::cerr << "  Please check the file path and format.\n";
    }
}

void handleShowSummary(const TransportNetwork& network, bool networkLoaded) {
    if (!networkLoaded) {
        std::cout << "\n  No network loaded. Please load a data file first (Option 1).\n";
        return;
    }
    displayNetworkSummary(network);
}

void handleSingleSearch(const TransportNetwork& network, bool networkLoaded) {
    if (!networkLoaded) {
        std::cout << "\n  No network loaded. Please load a data file first (Option 1).\n";
        return;
    }
    
    std::cout << "\n";
    displaySeparator('=', 50);
    std::cout << "  PLAN JOURNEY (Single Criteria)\n";
    displaySeparator('=', 50);
    
    // Get user type
    UserType userType = promptUserType();
    
    // Get origin
    std::string originId = promptStopId(network, "Enter origin");
    if (originId.empty()) {
        std::cout << "\n  Invalid origin. Returning to main menu.\n";
        return;
    }
    std::cout << "  Origin set: " << getStopNameById(network, originId) 
              << " (" << originId << ")\n";
    
    // Get destination
    std::string destId = promptStopId(network, "Enter destination");
    if (destId.empty()) {
        std::cout << "\n  Invalid destination. Returning to main menu.\n";
        return;
    }
    std::cout << "  Destination set: " << getStopNameById(network, destId) 
              << " (" << destId << ")\n";
    
    // Check if origin and destination are the same
    if (originId == destId) {
        std::cout << "\n  Origin and destination are the same!\n";
        return;
    }
    
    // Get search criteria
    SearchCriteria criteria = promptSearchCriteria();
    std::cout << "  Criteria: " << criteriaToString(criteria) << "\n";
    
    // Perform search
    std::cout << "\n  Searching...\n";
    DijkstraResult result = findOptimalPath(network, originId, destId, criteria, userType);
    
    if (!result.reachable) {
        std::cout << "\n  No route found from " << getStopNameById(network, originId)
                  << " to " << getStopNameById(network, destId) << ".\n";
        return;
    }
    
    // Convert to Journey and display
    Journey journey = convertToJourney(result, network, userType);
    
    std::cout << "\n  Found route!\n";
    displayJourney(journey, network, 0, criteria);
}

void handleCompareSearch(const TransportNetwork& network, bool networkLoaded) {
    if (!networkLoaded) {
        std::cout << "\n  No network loaded. Please load a data file first (Option 1).\n";
        return;
    }
    
    std::cout << "\n";
    displaySeparator('=', 50);
    std::cout << "  PLAN JOURNEY (Compare All Criteria)\n";
    displaySeparator('=', 50);
    
    // Get user type
    UserType userType = promptUserType();
    
    // Get origin
    std::string originId = promptStopId(network, "Enter origin");
    if (originId.empty()) {
        std::cout << "\n  Invalid origin. Returning to main menu.\n";
        return;
    }
    std::cout << "  Origin set: " << getStopNameById(network, originId) 
              << " (" << originId << ")\n";
    
    // Get destination
    std::string destId = promptStopId(network, "Enter destination");
    if (destId.empty()) {
        std::cout << "\n  Invalid destination. Returning to main menu.\n";
        return;
    }
    std::cout << "  Destination set: " << getStopNameById(network, destId) 
              << " (" << destId << ")\n";
    
    // Check if origin and destination are the same
    if (originId == destId) {
        std::cout << "\n  Origin and destination are the same!\n";
        return;
    }
    
    // Perform searches for all criteria
    std::cout << "\n  Searching for all optimal routes...\n";
    auto results = findAllOptimalPaths(network, originId, destId, userType);
    
    if (results.empty()) {
        std::cout << "\n  No routes found from " << getStopNameById(network, originId)
                  << " to " << getStopNameById(network, destId) << ".\n";
        return;
    }
    
    // Convert to journeys
    std::vector<Journey> journeys;
    std::vector<SearchCriteria> criteriaList = {
        SearchCriteria::FASTEST,
        SearchCriteria::CHEAPEST,
        SearchCriteria::FEWEST_TRANSFERS
    };
    
    for (const auto& result : results) {
        journeys.push_back(convertToJourney(result, network, userType));
    }
    
    // Display comparison
    displayJourneyComparison(journeys, criteriaList, network, userType);
}

void handleClearScreen() {
    clearScreen();
    std::cout << "  Screen cleared.\n\n";
}
