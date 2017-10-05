CC=g++
SRC_DIR=src
INC_DIR=include
CFLAGS=-I$(INC_DIR) -c -Wall -std=c++11 
LDFLAGS=
SOURCES= $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
  
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm -f $(SRC_DIR)/*.o
		rm -f $(EXECUTABLE)