# Compiler and flags
CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -Isrc

# Paths
SRC_DIR  = src
INC_DIR  = include
OBJ_DIR  = obj
BIN_DIR  = bin
BIN      = game

# Find all .cpp files recursively under src/
# (If your make is very old, replace the $(shell find ...) with extra wildcards.)
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Map each src file to an obj file under obj/, mirroring subdirs
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Phony targets
.PHONY: all clean run

# Default build target
all: $(BIN_DIR)/$(BIN)

# Link
$(BIN_DIR)/$(BIN): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Compile source files into object files
# Use $(dir $@) so obj subfolders are created automatically
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run the game
run: all
	./$(BIN_DIR)/$(BIN)
