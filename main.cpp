#include <iostream>
#include <string>
#include <limits>
#include "network.h"
#include "search.h"
#include "ranking.h"
#include "utils.h"

int main() {
    TransportNetwork network;
    bool networkLoaded = false;

    while (true) {
        std::cout << "=== Smart Public Transport Advisor ===\n";
        std::cout << "1. Load data file\n";
        std::cout << "2. Show network summary\n";
        std::cout << "3. Query journeys\n";
        std::cout << "4. Exit\n";
        std::cout << "Choose option: ";

        int choice;
        if (!(std::cin >> choice)) {
            // Handle invalid input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 4.\n\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            std::cout << "Enter file path: ";
            std::string path;
            std::getline(std::cin, path);
            try {
                network = loadFromFile(path);
                networkLoaded = true;
                std::cout << "Loaded " << network.getStopCount()
                          << " stops, " << network.getSegmentCount() << " segments\n\n";
            } catch (const std::exception& ex) {
                std::cerr << "Error loading file: " << ex.what() << "\n\n";
            }
        } else if (choice == 2) {
            if (!networkLoaded) {
                std::cout << "No network loaded. Please load a data file first.\n\n";
            } else {
                std::cout << "Network Summary:\n";
                displayNetworkSummary(network);
                std::cout << "\n";
            }
        } else if (choice == 3) {
            if (!networkLoaded) {
                std::cout << "No network loaded. Please load a data file first.\n\n";
                continue;
            }
            std::cout << "Enter origin (stop ID or name): ";
            std::string originInput;
            std::getline(std::cin, originInput);
            originInput = trim(originInput);

            std::cout << "Enter destination (stop ID or name): ";
            std::string destInput;
            std::getline(std::cin, destInput);
            destInput = trim(destInput);

            std::string originId = getStopIdFromInput(network, originInput);
            std::string destId = getStopIdFromInput(network, destInput);

            if (originId.empty()) {
                std::cout << "Origin not found: " << originInput << "\n\n";
                continue;
            }
            if (destId.empty()) {
                std::cout << "Destination not found: " << destInput << "\n\n";
                continue;
            }

            std::cout << "Enter preference (cheapest/fastest/fewest_segments): ";
            std::string preference;
            std::getline(std::cin, preference);
            preference = trim(preference);
            if (preference != "cheapest" && preference != "fastest" && preference != "fewest_segments") {
                std::cout << "Invalid preference. Use one of: cheapest, fastest, fewest_segments\n\n";
                continue;
            }

            auto journeys = generateJourneys(network, originId, destId, 6, 50);
            if (journeys.empty()) {
                std::cout << "No journeys found from " << originInput << " to " << destInput << ".\n\n";
                continue;
            }

            auto ranked = rankJourneys(journeys, preference);
            displayJourneyList(ranked, network, preference);
            std::cout << "\n";
        } else if (choice == 4) {
            std::cout << "Exiting.\n";
            break;
        } else {
            std::cout << "Invalid choice. Please enter 1-4.\n\n";
        }
    }

    return 0;
}