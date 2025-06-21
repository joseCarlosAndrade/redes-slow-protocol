# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++2a -Wall -Wextra -Iinclude -I./src -pthread # c++2a is the c++20 for the g++ version 8 and 9

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# using patsubts (a make function):
# patsubst pattern,replacement,text -> finds the 'pattern' in 'text' and replaces it with 'replacement'
SRCS := $(shell find $(SRC_DIR) -name '*.cpp') # finds all .cpp files recursively and assigns it to SRCS
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS)) 
# src/main.cpp src/example/ex.cpp -> build/main.o build/example/ex.o

# target = bin/app
TARGET := $(BIN_DIR)/app

all: $(TARGET) # the 'make all' rule will depend on the 'bin/app' (TARGET variable, the prerequisite)

#this rule creates a folder (if it doesnt already exists) for bin/
#then, it compiles every object (.o) file into the bin/ as executable (which first calls the BUILD_DIR target)
$(TARGET): $(OBJS) # the 'bin/app' (target) will depend on every .o (prerequisites)
	@mkdir -p $(BIN_DIR) 
	$(CXX) $(CXXFLAGS) -o $@ $^
# $@ is the target variable, and the $^ is all the prerequisites of this
# so it expands to: g++ (flags) -o bin/app build/main.go build/example/ex.o ...

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp # every file that matches build/../../X.o will have its prerequisite as the same thing but .cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
# same thing as the previous rule, but here is .cpp -> .o . $< gets the first prerequisite (in this case there'll always be only one)
# also, it creates a folder to mimic the same code structure, thats why it used dir $@ (the directory of the target .o)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# .phony explicitly tells makefile that all and clean are commands and not files
.PHONY: all clean