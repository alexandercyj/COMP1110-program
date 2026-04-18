#ifndef UTILS_H
#define UTILS_H

#include "network.h"
#include <string>
#include <vector>

/**
 * @file utils.h
 * @brief Utility functions for string processing and display.
 */

// ---------------------------------------------------------------------------
// String utilities
// ---------------------------------------------------------------------------

/**
 * @brief Trim whitespace from both ends of a string.
 */
std::string trim(const std::string& str);

/**
 * @brief Split string by delimiter.
 */
std::vector<std::string> split(const std::string& str, char delimiter);

/**
 * @brief Convert string to lowercase.
 */
std::string toLower(const std::string& s);

// ---------------------------------------------------------------------------
// Cross-platform screen clearing
// ---------------------------------------------------------------------------

/**
 * @brief Clear the terminal screen in a cross-platform manner.
 * 
 * Uses ANSI escape codes as the primary method (works on Linux, macOS, 
 * Windows 10+). Falls back to system("clear") on Unix or system("cls") 
 * on Windows if ANSI is not available.
 */
void clearScreen();

/**
 * @brief Check if the terminal supports ANSI escape codes.
 */
bool supportsAnsi();

// ---------------------------------------------------------------------------
// User type helpers
// ---------------------------------------------------------------------------

/**
 * @brief Parse user type string to enum.
 */
UserType parseUserType(const std::string& input);

/**
 * @brief Get display name for user type.
 */
std::string userTypeToString(UserType type);

/**
 * @brief Get discount description for user type.
 */
std::string getDiscountDescription(UserType type);

#endif // UTILS_H
