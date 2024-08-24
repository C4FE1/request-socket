# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Define the libraries to link against
LIBS = -lssl -lcrypto

# Define the target executable
TARGET = get

# Define source and object files
SRCS = src/hget.cpp
OBJS = $(SRCS:.cpp=.o)

# Define the rule to build the target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up build artifacts
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean

