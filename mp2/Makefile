CC=g++
SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include
CFLAGS=-I$(INC_DIR) -c -Wall -std=c++11 
LDFLAGS=
SOURCES=$(SRC_DIR)/socketlib.cpp \
		$(SRC_DIR)/node.cpp \
		$(SRC_DIR)/daemon.cpp \
		$(SRC_DIR)/daemon_member.cpp \
		$(SRC_DIR)/main.cpp 
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