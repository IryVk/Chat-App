# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -Wno-deprecated-declarations -I../include -I/usr/include/crypto++ -lgtest -lgtest_main -pthread -L../lib

# Linker flags
LDFLAGS = -lcrypto -lcryptopp

# Google Test flags
GTEST_FLAGS = -lgtest -pthread

# Google Test include directory
GTEST_INCLUDE = -I../lib/googletest/include

# Google Test library path
GTEST_LIB = -L../lib -lgtest -lgtest_main

# Source files directory              
SRC_DIR = ../src

# Object files directory
OBJ_DIR = ../obj

# Source files for testing 
TEST_SOURCES = $(wildcard *.cpp)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
SRC_OBJECTS = $(wildcard $(OBJ_DIR)/*.o)

# Exclude the main object file if it exists
MAIN_OBJ = $(OBJ_DIR)/main.o
ifneq ("$(wildcard $(MAIN_OBJ))","")
    SRC_OBJECTS := $(filter-out $(MAIN_OBJ), $(SRC_OBJECTS))
endif

# Target executable for tests
TARGET = run_tests

# Default rule
all: $(TARGET)

$(TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(GTEST_INCLUDE) -o $(TARGET) $(TEST_OBJECTS) $(SRC_OBJECTS) $(GTEST_FLAGS) -L../lib $(LDFLAGS)
	./$(TARGET) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_INCLUDE) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
