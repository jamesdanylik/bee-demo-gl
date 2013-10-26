# A generic makefile

CC=g++
CFLAGS=-c -std=c++11 -Wall
LDFLAGS=-lGLEW -lglut -lGL
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -f $(OBJECTS) $(EXECUTABLE)
