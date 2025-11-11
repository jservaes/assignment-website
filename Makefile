# Makefile for Assignment Tracker

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = assignment_tracker
SOURCES = server.cpp
HEADERS = assignment.h

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
