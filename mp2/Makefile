CC=g++
OBJ_DIR=src
INC_DIR=include
CFLAGS=-I$(INC_DIR) -c -Wall -std=c++11 
LDFLAGS=
SOURCES=$(OBJ_DIR)/socketlib.cpp $(OBJ_DIR)/node.cpp $(OBJ_DIR)/daemon.cpp $(OBJ_DIR)/main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
  
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm $(OBJ_DIR)/*.o
		rm $(EXECUTABLE)