# Compiler settings - Can change based on the compiler you use
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

# Build settings
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/memory_scraper

# Automatically collect all source files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Linking all the object files to make the final binary
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@

# Compiling each source file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Help menu
help:
	@echo "Available targets:"
	@echo "  all (default): Builds the application."
	@echo "  clean: Removes all build artifacts."
	@echo "  help: Displays this help message."

.PHONY: all clean help
