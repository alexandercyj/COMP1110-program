#include "utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>
#include <cstdlib>

// ---------------------------------------------------------------------------
// String utilities
// ---------------------------------------------------------------------------

std::string trim(const std::string& str) {
    const char* ws = " \t\n\r\f\v";
    size_t start = str.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(ws);
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> out;
    std::string token;
    std::istringstream ss(str);
    while (std::getline(ss, token, delimiter)) {
        out.push_back(token);
    }
    return out;
}

std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

// ---------------------------------------------------------------------------
// Cross-platform screen clearing
// ---------------------------------------------------------------------------

bool supportsAnsi() {
    // Check for environment variables that indicate ANSI support
    const char* term = std::getenv("TERM");
    if (term) {
        std::string termStr(term);
        // Most modern terminals support ANSI
        if (termStr.find("xterm") != std::string::npos ||
            termStr.find("screen") != std::string::npos ||
            termStr.find("tmux") != std::string::npos ||
            termStr.find("ansi") != std::string::npos ||
            termStr.find("color") != std::string::npos ||
            termStr.find("linux") != std::string::npos) {
            return true;
        }
    }
    
    // Windows 10+ supports ANSI via ENABLE_VIRTUAL_TERMINAL_PROCESSING
    // Check for WT_SESSION (Windows Terminal) or ANSICON
    if (std::getenv("WT_SESSION") || std::getenv("ANSICON")) {
        return true;
    }
    
    return false;
}

void clearScreen() {
    // Primary method: ANSI escape codes (works on Linux, macOS, Windows 10+)
    std::cout << "\033[2J\033[H";
    
    // Alternative: system commands as fallback
    // Note: system() is used as a last resort for compatibility
    #ifdef _WIN32
        // Windows fallback
        std::system("cls");
    #else
        // Unix/Linux/macOS fallback - only if ANSI didn't work
        // Most modern terminals support ANSI, so this is rarely needed
        // std::system("clear");
    #endif
}

// ---------------------------------------------------------------------------
// User type helpers
// ---------------------------------------------------------------------------

UserType parseUserType(const std::string& input) {
    std::string s = toLower(trim(input));
    if (s == "student" || s == "s" || s == "2") {
        return UserType::STUDENT;
    } else if (s == "elderly" || s == "e" || s == "senior" || s == "3") {
        return UserType::ELDERLY;
    } else if (s == "child" || s == "c" || s == "4") {
        return UserType::CHILD;
    } else {
        return UserType::ADULT;
    }
}

std::string userTypeToString(UserType type) {
    switch (type) {
        case UserType::ADULT: return "Adult";
        case UserType::STUDENT: return "Student";
        case UserType::ELDERLY: return "Elderly";
        case UserType::CHILD: return "Child";
    }
    return "Unknown";
}

std::string getDiscountDescription(UserType type) {
    switch (type) {
        case UserType::ADULT: return "Full fare (no discount)";
        case UserType::STUDENT: return "50% discount on MTR fares";
        case UserType::ELDERLY: return "20% discount (80% of adult fare)";
        case UserType::CHILD: return "50% discount on all fares";
    }
    return "";
}
