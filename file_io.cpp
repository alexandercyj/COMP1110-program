#include "file_io.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// Helper to parse boolean from "true"/"false" (case-insensitive)
static bool parseBool(const std::string& s) {
    std::string t = toLower(trim(s));
    return (t == "true" || t == "1");
}

// Helper to parse double; returns pair(ok, value)
static std::pair<bool, double> tryParseDouble(const std::string& s) {
    std::string t = trim(s);
    if (t.empty() || t == "N/A" || t == "n/a") return {false, 0.0};
    try {
        double v = std::stod(t);
        return {true, v};
    } catch (...) {
        return {false, 0.0};
    }
}

// ---------------------------------------------------------------------------
// Main file loading function
// ---------------------------------------------------------------------------
TransportNetwork loadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        throw std::runtime_error("Unable to open file: " + filepath);
    }

    TransportNetwork net;
    std::string line;
    size_t lineNo = 0;
    int stopCount = 0;
    int lineCount = 0;
    int segmentCount = 0;

    while (std::getline(ifs, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;
        if (line.size() > 0 && line[0] == '#') continue;

        auto tokens = split(line, '|');
        if (tokens.empty()) continue;

        std::string type = trim(tokens[0]);
        
        if (type == "STOP") {
            // Format: STOP|id|name_cn|name_en|x|y|is_accessible
            if (tokens.size() != 7) {
                std::cerr << "Warning: Malformed STOP line " << lineNo 
                          << ": expected 7 fields, got " << tokens.size() << "\n";
                continue;
            }
            Stop s;
            s.id = trim(tokens[1]);
            s.nameCn = trim(tokens[2]);
            s.nameEn = trim(tokens[3]);
            try {
                s.x = std::stoi(trim(tokens[4]));
                s.y = std::stoi(trim(tokens[5]));
            } catch (...) {
                std::cerr << "Warning: Malformed STOP coordinates at line " << lineNo << "\n";
                continue;
            }
            s.isAccessible = parseBool(tokens[6]);
            if (s.id.empty()) {
                std::cerr << "Warning: Empty STOP id at line " << lineNo << "\n";
                continue;
            }
            net.addStop(s);
            ++stopCount;
            
        } else if (type == "LINE") {
            // Format: LINE|id|name|color|mode
            if (tokens.size() < 5) {
                std::cerr << "Warning: Malformed LINE line " << lineNo 
                          << ": expected 5 fields, got " << tokens.size() << "\n";
                continue;
            }
            Line l;
            l.id = trim(tokens[1]);
            l.name = trim(tokens[2]);
            l.color = trim(tokens[3]);
            l.mode = trim(tokens[4]);
            if (l.id.empty()) {
                std::cerr << "Warning: Empty LINE id at line " << lineNo << "\n";
                continue;
            }
            net.addLine(l);
            ++lineCount;
            
        } else if (type == "SEGMENT") {
            // Support both formats:
            //  - legacy 7-field: SEGMENT|from|to|line|duration|cost|mode
            //  - extended 8-field: SEGMENT|from|to|line|duration|cost_adult|cost_student|mode
            if (tokens.size() < 7) {
                std::cerr << "Warning: Malformed SEGMENT line " << lineNo 
                          << ": expected at least 7 fields, got " << tokens.size() << "\n";
                continue;
            }
            
            Segment seg;
            seg.fromStop = trim(tokens[1]);
            seg.toStop = trim(tokens[2]);
            seg.lineId = trim(tokens[3]);

            // Duration at tokens[4]
            auto durRes = tryParseDouble(tokens[4]);
            if (!durRes.first) {
                std::cerr << "Warning: Malformed SEGMENT duration at line " << lineNo << "\n";
                continue;
            }
            seg.duration = durRes.second;

            // Check if 8-field format (has separate student fare)
            if (tokens.size() >= 8) {
                // 8-field: SEGMENT|from|to|line|duration|cost_adult|cost_student|mode
                auto adultRes = tryParseDouble(tokens[5]);
                if (!adultRes.first) {
                    std::cerr << "Warning: Invalid adult fare at line " << lineNo << "\n";
                    seg.costAdult = 0.0;
                } else {
                    seg.costAdult = adultRes.second;
                }
                
                auto studentRes = tryParseDouble(tokens[6]);
                if (!studentRes.first) {
                    seg.costStudent = -1.0;  // Mark as N/A
                } else {
                    seg.costStudent = studentRes.second;
                }
                
                seg.mode = trim(tokens[7]);
            } else {
                // 7-field: SEGMENT|from|to|line|duration|cost|mode
                auto costRes = tryParseDouble(tokens[5]);
                if (!costRes.first) {
                    std::cerr << "Warning: Invalid cost at line " << lineNo << "\n";
                    seg.costAdult = 0.0;
                } else {
                    seg.costAdult = costRes.second;
                }
                seg.costStudent = -1.0;  // N/A
                seg.mode = trim(tokens[6]);
            }

            if (seg.fromStop.empty() || seg.toStop.empty()) {
                std::cerr << "Warning: SEGMENT missing stop id at line " << lineNo << "\n";
                continue;
            }
            net.addSegment(seg);
            ++segmentCount;
            
        } else {
            std::cerr << "Warning: Unknown record type at line " << lineNo << ": " << type << "\n";
            continue;
        }
    }

    std::cout << "[FileIO] Loaded " << stopCount << " stops, " 
              << lineCount << " lines, " << segmentCount << " segments.\n";
    
    return net;
}
