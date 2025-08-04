# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

# Source and build paths
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN = game

# Find all source files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default build target
all: $(BIN)

# Build executable
$(BIN): $(OBJS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o bin/$(BIN) $(OBJS)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) bin/$(BIN)

# Run the game
run: all
	./bin/$(BIN)
