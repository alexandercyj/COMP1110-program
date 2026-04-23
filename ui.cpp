#include "ui.h"
#include "utils.h"
#include <iostream>
#include <iomanip>
#include <limits>

// ---------------------------------------------------------------------------
// Display functions
// ---------------------------------------------------------------------------

void displayMainMenu() {
    std::cout << "\n";
    displaySeparator('=', 60);
    std::cout << "     SMART PUBLIC TRANSPORT ADVISOR\n";
    std::cout << "     Hong Kong MTR & Bus Journey Planner\n";
    displaySeparator('=', 60);
    std::cout << "\n";
    std::cout << "  [1] Load network data file\n";
    std::cout << "  [2] Show network summary\n";
    std::cout << "  [3] Plan journey (single criteria)\n";
    std::cout << "  [4] Plan journey (compare all criteria)\n";
    std::cout << "  [5] Clear screen\n";
    std::cout << "  [6] Exit\n";
    std::cout << "\n";
}

void displayUserTypeMenu() {
    std::cout << "\n";
    displaySeparator('-', 50);
    std::cout << "  Select passenger type:\n";
    std::cout << "  [1] Adult        - Full fare\n";
    std::cout << "  [2] Student      - 50% discount on MTR\n";
    std::cout << "  [3] Elderly      - 20% discount (80% of adult fare)\n";
    std::cout << "  [4] Child        - 50% discount on all fares\n";
    displaySeparator('-', 50);
    std::cout << "  Enter choice (1-4): ";
}

void displaySearchCriteriaMenu() {
    std::cout << "\n";
    displaySeparator('-', 50);
    std::cout << "  Select optimization criteria:\n";
    std::cout << "  [1] Fastest          - Minimize travel time\n";
    std::cout << "  [2] Cheapest         - Minimize fare cost\n";
    std::cout << "  [3] Fewest Transfers - Minimize line changes\n";
    displaySeparator('-', 50);
    std::cout << "  Enter choice (1-3): ";
}

void displayNetworkSummary(const TransportNetwork& network) {
    std::cout << "\n";
    displaySeparator('=', 50);
    std::cout << "  NETWORK SUMMARY\n";
    displaySeparator('=', 50);
    
    std::cout << "  Total Stops:    " << network.getStopCount() << "\n";
    std::cout << "  Total Lines:    " << network.getLineCount() << "\n";
    std::cout << "  Total Segments: " << network.getSegmentCount() << "\n";
    
    // Display lines
    std::cout << "\n  Lines:\n";
    for (const auto& kv : network.getAllLines()) {
        const Line& line = kv.second;
        std::cout << "    " << std::left << std::setw(8) << line.id 
                  << " " << std::setw(20) << line.name
                  << " [" << line.mode << "]\n";
    }
    
    // Display stops
    std::cout << "\n  Stops (ID - English Name - Chinese Name):\n";
    auto ids = getAllStopIds(network);
    for (const auto& id : ids) {
        const Stop* stop = network.getStop(id);
        if (stop) {
            std::cout << "    " << std::left << std::setw(6) << id 
                      << " " << std::setw(25) << stop->nameEn
                      << " " << stop->nameCn;
            std::cout << "\n";
        }
    }
    displaySeparator('=', 50);
}

void displayJourney(const Journey& journey, const TransportNetwork& network, 
                    int index, SearchCriteria criteria) {
    if (journey.segments.empty()) {
        std::cout << "  (No journey data)\n";
        return;
    }
    
    std::cout << "\n";
    if (index >= 0) {
        displaySeparator('-', 50);
        std::cout << "  OPTION " << (index + 1) << ": " << criteriaToString(criteria) << "\n";
        displaySeparator('-', 50);
    }
    
    // Build route description
    std::cout << "  ROUTE:\n";
    std::string currentLine = "";
    int segmentInLine = 0;
    
    for (size_t i = 0; i < journey.segments.size(); ++i) {
        const Segment& seg = journey.segments[i];
        
        // Print origin stop at start
        if (i == 0) {
            std::cout << "    " << getStopNameById(network, seg.fromStop) 
                      << " (" << seg.fromStop << ")\n";
        }
        
        // Check if line changed
        if (seg.lineId != currentLine) {
            if (!currentLine.empty()) {
                std::cout << "      (" << segmentInLine << " segment" 
                          << (segmentInLine > 1 ? "s" : "") << ")\n";
            }
            currentLine = seg.lineId;
            segmentInLine = 0;
            const Line* line = network.getLine(seg.lineId);
            std::cout << "    ↓ Take " << (line ? line->name : seg.lineId) 
                      << " [" << seg.mode << "]\n";
        }
        ++segmentInLine;
        
        // Print destination stop
        std::cout << "    " << getStopNameById(network, seg.toStop) 
                  << " (" << seg.toStop << ")";
        std::cout << "  ~" << std::fixed << std::setprecision(1) << seg.duration << "min";
        std::cout << "  HK$" << std::fixed << std::setprecision(2) << seg.costAdult;
        std::cout << "\n";
    }
    
    // Print last line segment count
    if (!currentLine.empty() && segmentInLine > 0) {
        std::cout << "      (" << segmentInLine << " segment" 
                  << (segmentInLine > 1 ? "s" : "") << ")\n";
    }
    
    // Summary
    std::cout << "\n  SUMMARY:\n";
    std::cout << "    Total Time:     " << std::fixed << std::setprecision(1) 
              << journey.totalDuration << " minutes\n";
    std::cout << "    Total Fare:     HK$ " << std::fixed << std::setprecision(2) 
              << journey.totalCost << "\n";
    std::cout << "    Transfers:      " << journey.numTransfers << "\n";
    std::cout << "    Segments:       " << journey.segments.size() << "\n";
}

void displayJourneyComparison(const std::vector<Journey>& journeys,
                              const std::vector<SearchCriteria>& criteria,
                              const TransportNetwork& network,
                              UserType userType) {
    if (journeys.empty()) {
        std::cout << "  No journeys found.\n";
        return;
    }
    
    std::cout << "\n";
    displaySeparator('=', 70);
    std::cout << "  JOURNEY COMPARISON (" << userTypeToString(userType) << ")\n";
    displaySeparator('=', 70);
    
    // Header
    std::cout << "\n  " << std::left << std::setw(20) << "Criteria"
              << std::setw(12) << "Time(min)"
              << std::setw(12) << "Fare(HK$)"
              << std::setw(12) << "Transfers"
              << std::setw(10) << "Segments" << "\n";
    displaySeparator('-', 70);
    
    // Data rows
    for (size_t i = 0; i < journeys.size() && i < criteria.size(); ++i) {
        std::cout << "  " << std::left << std::setw(20) << criteriaToString(criteria[i])
                  << std::setw(12) << std::fixed << std::setprecision(1) << journeys[i].totalDuration
                  << std::setw(12) << std::fixed << std::setprecision(2) << journeys[i].totalCost
                  << std::setw(12) << journeys[i].numTransfers
                  << std::setw(10) << journeys[i].segments.size() << "\n";
    }
    
    // Detailed breakdown
    std::cout << "\n";
    for (size_t i = 0; i < journeys.size() && i < criteria.size(); ++i) {
        displayJourney(journeys[i], network, static_cast<int>(i), criteria[i]);
    }
    
    displaySeparator('=', 70);
}

void displaySeparator(char c, int length) {
    std::cout << "  ";
    for (int i = 0; i < length; ++i) {
        std::cout << c;
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// Input functions
// ---------------------------------------------------------------------------

UserType promptUserType() {
    while (true) {
        displayUserTypeMenu();
        
        std::string input;
        std::getline(std::cin, input);
        std::string lowered = toLower(trim(input));
        
        // 主动取消
        if (lowered == "q" || lowered == "quit" || lowered == "exit") {
            std::cout << "  Cancelled by user. Defaulting to Adult.\n";
            return UserType::ADULT;
        }
        
        // 空输入默认 Adult（保留原设计）
        if (input.empty()) {
            std::cout << "  Defaulting to Adult.\n";
            return UserType::ADULT;
        }
        
        // 校验有效性：只允许 1-4 或对应英文关键字
        if (lowered == "1" || lowered == "adult" || lowered == "a" ||
            lowered == "2" || lowered == "student" || lowered == "s" ||
            lowered == "3" || lowered == "elderly" || lowered == "e" || lowered == "senior" ||
            lowered == "4" || lowered == "child" || lowered == "c") {
            UserType type = parseUserType(input);
            std::cout << "  Selected: " << userTypeToString(type) 
                      << " - " << getDiscountDescription(type) << "\n";
            return type;
        }
        
        std::cout << "  Invalid choice. Please enter 1-4, or 'q' to cancel.\n";
    }
}

SearchCriteria promptSearchCriteria() {
    while (true) {
        displaySearchCriteriaMenu();
        
        std::string input;
        std::getline(std::cin, input);
        std::string lowered = toLower(trim(input));
        
        // 主动取消
        if (lowered == "q" || lowered == "quit" || lowered == "exit") {
            std::cout << "  Cancelled by user. Defaulting to Fastest.\n";
            return SearchCriteria::FASTEST;
        }
        
        // 空输入默认 Fastest
        if (input.empty()) {
            std::cout << "  Defaulting to Fastest.\n";
            return SearchCriteria::FASTEST;
        }
        
        // 校验有效性
        if (lowered == "1" || lowered == "fastest" || lowered == "time" || lowered == "quickest" ||
            lowered == "2" || lowered == "cheapest" || lowered == "cost" || lowered == "price" || lowered == "money" ||
            lowered == "3" || lowered == "fewest_transfers" || lowered == "transfers" || lowered == "fewest" || lowered == "transfer") {
            if (lowered == "1") return SearchCriteria::FASTEST;
            if (lowered == "2") return SearchCriteria::CHEAPEST;
            if (lowered == "3") return SearchCriteria::FEWEST_TRANSFERS;
            return parseSearchCriteria(input);
        }
        
        std::cout << "  Invalid choice. Please enter 1-3, or 'q' to cancel.\n";
    }
}

std::string promptStopId(const TransportNetwork& network, const std::string& prompt) {
    while (true) {
        std::cout << "  " << prompt << " (ID/English/Chinese name, or 'q' to cancel): ";
        std::string input;
        std::getline(std::cin, input);
        
        std::string lowered = toLower(trim(input));
        if (lowered == "q" || lowered == "quit" || lowered == "exit") {
            std::cout << "  Cancelled by user.\n";
            return "";
        }
        
        if (input.empty()) {
            std::cout << "  Input cannot be empty. Please try again or enter 'q' to cancel.\n";
            continue;
        }
        
        std::string stopId = getStopIdFromInput(network, input);
        if (!stopId.empty()) {
            return stopId;
        }
        
        std::cout << "  Stop not found: '" << input << "'. Please try again or enter 'q' to cancel.\n";
    }
}

bool promptYesNo(const std::string& message) {
    std::cout << "  " << message << " (y/n): ";
    std::string input;
    std::getline(std::cin, input);
    std::string lowered = toLower(trim(input));
    return (lowered == "y" || lowered == "yes" || lowered == "1");
}

void promptContinue() {
    std::cout << "\n  Press Enter to continue...";
    std::cin.get();
}

std::string getStopIdFromInput(const TransportNetwork& network, const std::string& input) {
    if (input.empty()) return "";
    std::string trimmed = trim(input);
    
    // First, check if it's an exact ID match (case-insensitive for convenience)
    for (const auto& kv : network.getAllStops()) {
        if (toLower(kv.first) == toLower(trimmed)) {
            return kv.first;  // Return actual ID with correct case
        }
    }
    
    // Exact ID match (original case)
    if (network.getStop(trimmed) != nullptr) return trimmed;
    
    // Try matching English name (case-insensitive exact match)
    std::string low = toLower(trimmed);
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (toLower(stop.nameEn) == low) return stop.id;
    }
    
    // Try Chinese name exact match
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (stop.nameCn == trimmed) return stop.id;
    }
    
    // Try partial match on English name (contains)
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (toLower(stop.nameEn).find(low) != std::string::npos) return stop.id;
    }
    
    // Try partial match on Chinese name
    for (const auto& kv : network.getAllStops()) {
        const auto& stop = kv.second;
        if (stop.nameCn.find(trimmed) != std::string::npos) return stop.id;
    }
    
    return "";
}
