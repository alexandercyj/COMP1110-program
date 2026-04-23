#ifndef UI_H
#define UI_H

#include "network.h"
#include "pathfinder.h"
#include <vector>
#include <string>

/**
 * @file ui.h
 * @brief User interface module for all input/output operations.
 * 
 * Handles menu display, user input collection, and formatted output
 * of search results.
 */

// ---------------------------------------------------------------------------
// Menu and display functions
// ---------------------------------------------------------------------------

/**
 * @brief Display the main menu banner.
 */
void displayMainMenu();

/**
 * @brief Display the user type selection menu.
 */
void displayUserTypeMenu();

/**
 * @brief Display the search criteria menu.
 */
void displaySearchCriteriaMenu();

/**
 * @brief Display network summary.
 */
void displayNetworkSummary(const TransportNetwork& network);

/**
 * @brief Display a single journey with detailed information.
 */
void displayJourney(const Journey& journey, const TransportNetwork& network, 
                    int index = -1, SearchCriteria criteria = SearchCriteria::FASTEST);

/**
 * @brief Display multiple journeys (comparison view).
 */
void displayJourneyComparison(const std::vector<Journey>& journeys,
                              const std::vector<SearchCriteria>& criteria,
                              const TransportNetwork& network,
                              UserType userType);

/**
 * @brief Display a formatted separator line.
 */
void displaySeparator(char c = '-', int length = 60);

// ---------------------------------------------------------------------------
// Input helper functions
// ---------------------------------------------------------------------------

/**
 * @brief Get user type selection from user.
 * @return Selected UserType
 */
UserType promptUserType();

/**
 * @brief Get search criteria from user.
 * @return Selected SearchCriteria
 */
SearchCriteria promptSearchCriteria();

/**
 * @brief Get stop input from user and resolve to stop ID.
 * @param network The transport network
 * @param prompt Display prompt (e.g., "Enter origin")
 * @return Resolved stop ID, or empty string if not found
 */
std::string promptStopId(const TransportNetwork& network, const std::string& prompt);

/**
 * @brief Get yes/no confirmation from user.
 */
bool promptYesNo(const std::string& message);

/**
 * @brief Wait for user to press Enter.
 */
void promptContinue();

/**
 * @brief Resolve user input (stop ID or English name) to stop ID.
 * @return Empty string if not found
 */
std::string getStopIdFromInput(const TransportNetwork& network, const std::string& input);

#endif // UI_H
