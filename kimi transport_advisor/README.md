# Smart Public Transport Advisor

A comprehensive C++ application for planning optimal public transport journeys in Hong Kong, supporting both MTR and bus networks with multi-criteria pathfinding.

## Features

- **Multi-Criteria Pathfinding**: Find optimal routes based on:
  - **Fastest**: Minimize total travel time
  - **Cheapest**: Minimize fare cost
  - **Fewest Transfers**: Minimize number of line changes
- **Passenger Type Support**: Apply appropriate fare discounts:
  - **Adult**: Full fare (no discount)
  - **Student**: 50% discount on MTR fares
  - **Elderly**: 20% discount (80% of adult fare)
  - **Child**: 50% discount on all fares
- **Journey Comparison**: Compare all three optimization criteria side-by-side
- **Cross-Platform**: Works on Linux, macOS, and Windows (via WSL/MinGW)
- **Clean User Interface**: Formatted output with clear navigation

## File Structure

```
transport_advisor/
├── main.cpp          # Program entry point and main loop
├── network.h         # Core data structures (Stop, Line, Segment, Journey)
├── network.cpp       # TransportNetwork class implementation
├── file_io.h         # File parsing interface
├── file_io.cpp       # STOP.txt parser
├── pathfinder.h      # Dijkstra pathfinding algorithms
├── pathfinder.cpp    # Three-criteria optimal path implementation
├── ui.h              # User interface functions
├── ui.cpp            # Menu display and input handling
├── utils.h           # Utility functions
├── utils.cpp         # String processing and screen clearing
├── Makefile          # Build configuration
├── STOP.txt          # Sample network data (Hong Kong MTR + Bus)
└── README.md         # This file
```

## Module Responsibilities

| Module | Files | Description |
|--------|-------|-------------|
| **Main** | `main.cpp` | Program entry, menu loop, session management |
| **Network** | `network.h/cpp` | Data structures: stops, lines, segments, journeys |
| **File I/O** | `file_io.h/cpp` | Parse STOP.txt format, build network graph |
| **Pathfinder** | `pathfinder.h/cpp` | Dijkstra algorithm for 3 optimization criteria |
| **UI** | `ui.h/cpp` | All user interaction: menus, input, formatted output |
| **Utils** | `utils.h/cpp` | String helpers, cross-platform screen clearing |

## Data Format (STOP.txt)

The data file uses a pipe-delimited format with three record types:

### STOP Records
```
STOP|id|name_cn|name_en|x|y|is_accessible
```
Example:
```
STOP|KET|堅尼地城|Kennedy Town|150|800|true
```

### LINE Records
```
LINE|id|name|color|mode
```
Example:
```
LINE|ISL|港島線|#007DC5|MTR
LINE|BUS1|巴士1|#000000|BUS
```

### SEGMENT Records
Two formats supported:

**7-field format** (legacy):
```
SEGMENT|from_stop|to_stop|line|duration|cost|mode
```

**8-field format** (with student fare):
```
SEGMENT|from_stop|to_stop|line|duration|cost_adult|cost_student|mode
```

Example:
```
SEGMENT|KET|HKU|ISL|3|5.0|3.2|MTR
SEGMENT|ADM|WCH|SIL|8|10.0|4.9|MTR
SEGMENT|KET|HKU|BUS1|8|4.5|N/A|BUS
```

**Notes:**
- Use `N/A` or `n/a` for unavailable student fares (bus routes)
- Lines starting with `#` are treated as comments
- Empty lines are ignored
- Coordinates (x, y) are for display/reference only

## Fare Calculation Rules

### Adult Fares
- Use the `cost_adult` field directly from segment data
- Bus fares: as specified in data file

### Student Fares (MTR)
- If `cost_student` is provided and valid, use it
- Otherwise, apply 50% discount to adult fare
- Bus routes: 50% discount on adult fare

### Elderly Fares
- 20% discount on all fares (pay 80% of adult fare)

### Child Fares
- 50% discount on all fares

## How to Compile

### Requirements
- g++ with C++11 support (or later)
- Linux, macOS, or Windows with WSL/MinGW

### Compile Commands

**Using Make (recommended):**
```bash
make              # Build release version
make debug        # Build with debug symbols
make clean        # Remove compiled files
make run          # Build and run
```

**Manual compilation:**
```bash
g++ -std=c++11 -Wall -Wextra -O2 -o transport_advisor \
    main.cpp network.cpp file_io.cpp pathfinder.cpp ui.cpp utils.cpp
```

## How to Run

```bash
./transport_advisor
```

The program will automatically attempt to load `STOP.txt` from the current directory. If not found, you can load a data file manually via the menu (Option 1).

### Menu Options

1. **Load network data file** - Load a custom data file
2. **Show network summary** - Display all stops and lines
3. **Plan journey (single criteria)** - Search with one optimization goal
4. **Plan journey (compare all criteria)** - Compare time/cost/transfers
5. **Clear screen** - Clear the terminal
6. **Exit** - Quit the program

## Usage Examples

### Example 1: Fastest Route (Student)
```
=== SMART PUBLIC TRANSPORT ADVISOR ===
[3] Plan journey (single criteria)

Select passenger type: [2] Student
Enter origin: Kennedy Town
Enter destination: Wong Chuk Hang
Select criteria: [1] Fastest

RESULT:
  Route: Kennedy Town -> HKU -> Central -> Admiralty -> Wong Chuk Hang
  Total Time: 18.0 minutes
  Total Fare: HK$ 14.50 (student discount applied)
  Transfers: 1
```

### Example 2: Journey Comparison (Adult)
```
=== SMART PUBLIC TRANSPORT ADVISOR ===
[4] Plan journey (compare all criteria)

Select passenger type: [1] Adult
Enter origin: KET
Enter destination: WCH

JOURNEY COMPARISON:
  Criteria              Time(min)  Fare(HK$)  Transfers  Segments
  ----------------------------------------------------------------
  Fastest (Min Time)    18.0       25.00      1          4
  Cheapest (Min Cost)   18.0       25.00      1          4
  Fewest Transfers      18.0       25.00      1          4
```

### Example 3: Using Stop IDs
```
Enter origin: KET        (Kennedy Town)
Enter destination: AIR   (Airport)
```

## Cross-Platform Screen Clearing

The program uses ANSI escape codes as the primary method for clearing the screen, which works on:
- Linux terminals
- macOS Terminal/iTerm2
- Windows 10+ Command Prompt and PowerShell
- Windows Terminal

Fallback to system commands (`clear` on Unix, `cls` on Windows) is available if ANSI is not supported.

## Algorithm Details

### Dijkstra's Algorithm
The pathfinding uses Dijkstra's shortest path algorithm with custom edge weights:

- **Fastest**: Edge weight = travel duration (minutes)
- **Cheapest**: Edge weight = fare cost (HKD, adjusted for user type)
- **Fewest Transfers**: Edge weight = 1 per segment + 1000 penalty for line changes

Time complexity: O((V + E) log V) where V = stops, E = segments

## Sample Network Data

The included `STOP.txt` contains a simplified Hong Kong transport network:

**MTR Lines:**
- 港島線 (Island Line): KET-HKU-CEN-ADM
- 荃灣線 (Tsuen Wan Line): CEN-ADM-TST-MOK-MEI-LAI
- 南港島線 (South Island Line): ADM-WCH
- 東鐵線 (East Rail Line): ADM-HUH-KOT-SHS
- 觀塘線 (Kwun Tong Line): DIH-KOT-MOK-HOM
- 屯馬線 (Tuen Ma Line): MEI-NAC-AUS-ETS-HUH-HOM-KAT-DIH
- 機場快線 (Airport Express): HOK-KOW-NAC-LAI-SUN-AIR

**Bus Routes:**
- 巴士1: KET-HKU-CEN-ADM-WCH
- 巴士2: TST-HUH-HOM-MOK
- 巴士3: MEI-NAC-AUS-KAT

## License

This project is for educational purposes.
