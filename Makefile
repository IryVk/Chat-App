# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -Wno-deprecated-declarations -Iinclude -I/usr/include/crypto++ -I/usr/include/openssl -fpermissive

# Linker flags
LDFLAGS = -lcrypto -lcryptopp -pthread -lncurses

# Executable names
CLIENT_TARGET = chat-client.exe
SERVER_TARGET = chat-server.exe

# Source directory for common, client, and server
COMMON_SRC_DIR = src/common
CLIENT_SRC_DIR = src/client
SERVER_SRC_DIR = src/server

# Object directory for common, client, and server
COMMON_OBJ_DIR = obj/common
CLIENT_OBJ_DIR = obj/client
SERVER_OBJ_DIR = obj/server

# Source files
COMMON_SOURCES = $(wildcard $(COMMON_SRC_DIR)/*.cpp)
CLIENT_SOURCES = $(wildcard $(CLIENT_SRC_DIR)/*.cpp)
SERVER_SOURCES = $(wildcard $(SERVER_SRC_DIR)/*.cpp)

# Object files
COMMON_OBJECTS = $(patsubst $(COMMON_SRC_DIR)/%.cpp,$(COMMON_OBJ_DIR)/%.o,$(COMMON_SOURCES))
CLIENT_OBJECTS = $(patsubst $(CLIENT_SRC_DIR)/%.cpp,$(CLIENT_OBJ_DIR)/%.o,$(CLIENT_SOURCES)) $(COMMON_OBJECTS)
SERVER_OBJECTS = $(patsubst $(SERVER_SRC_DIR)/%.cpp,$(SERVER_OBJ_DIR)/%.o,$(SERVER_SOURCES)) $(COMMON_OBJECTS)

# Default rule to make everything
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Rule to make the client executable
$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

# Rule to make the server executable
$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

# Rule to compile client object files
$(CLIENT_OBJ_DIR)/%.o: $(CLIENT_SRC_DIR)/%.cpp
	@mkdir -p $(CLIENT_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile server object files
$(SERVER_OBJ_DIR)/%.o: $(SERVER_SRC_DIR)/%.cpp
	@mkdir -p $(SERVER_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile common object files
$(COMMON_OBJ_DIR)/%.o: $(COMMON_SRC_DIR)/%.cpp
	@mkdir -p $(COMMON_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up
clean:
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET)
	rm -rf $(CLIENT_OBJ_DIR) $(SERVER_OBJ_DIR) $(COMMON_OBJ_DIR)
