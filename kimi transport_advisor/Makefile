# ---------------------------------------------------------------------------
# Smart Public Transport Advisor - Makefile
# ---------------------------------------------------------------------------
# Supports: Linux, macOS, Windows (via MinGW or WSL)
# 
# Usage:
#   make          - Build the project
#   make clean    - Remove compiled files
#   make run      - Build and run with default data
#   make debug    - Build with debug symbols
# ---------------------------------------------------------------------------

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
DEBUGFLAGS = -std=c++11 -Wall -Wextra -g -DDEBUG

# Target executable
TARGET = transport_advisor

# Source files
SOURCES = main.cpp \
          network.cpp \
          file_io.cpp \
          pathfinder.cpp \
          ui.cpp \
          utils.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Header files (for dependency tracking)
HEADERS = network.h \
          file_io.h \
          pathfinder.h \
          ui.h \
          utils.h

# Default data file
DATA_FILE = STOP.txt

# ---------------------------------------------------------------------------
# Build targets
# ---------------------------------------------------------------------------

# Default target: build the executable
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build complete: ./$(TARGET)"

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build
debug: CXXFLAGS = $(DEBUGFLAGS)
debug: clean $(TARGET)
	@echo "Debug build complete"

# ---------------------------------------------------------------------------
# Utility targets
# ---------------------------------------------------------------------------

# Clean compiled files
clean:
	@echo "Cleaning build files..."
	@rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete"

# Build and run with default data
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# Build and run with valgrind (Linux only)
valgrind: debug
	@echo "Running with valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Show help
help:
	@echo "Smart Public Transport Advisor - Build Options"
	@echo "=============================================="
	@echo "  make        - Build the project (release mode)"
	@echo "  make clean  - Remove all compiled files"
	@echo "  make run    - Build and run the program"
	@echo "  make debug  - Build with debug symbols"
	@echo "  make help   - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - g++ with C++11 support"
	@echo "  - Linux/macOS/Windows(WSL)"

# ---------------------------------------------------------------------------
# Phony targets (not actual files)
# ---------------------------------------------------------------------------

.PHONY: all clean run debug valgrind help
