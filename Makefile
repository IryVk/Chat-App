# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -Wno-deprecated-declarations -Iinclude -I/usr/include/crypto++

# Linker flags
LDFLAGS = -lcrypto -lcryptopp

# Name of the executable
TARGET = chat-app

# Source directory
SRC_DIR = src

# Object directory
OBJ_DIR = obj

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default rule to make everything
all: $(TARGET)

# Rule to make the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

# Rule to compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up
clean:
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)
