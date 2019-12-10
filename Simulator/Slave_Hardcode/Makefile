CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 -g
LDFLAGS =  -fsanitize=address

LIBPERIPH = peripheral/inc/Serial.hpp peripheral/src/Serial.cpp peripheral/inc/LinuxMillisTimer.hpp peripheral/src/LinuxMillisTimer.cpp
LIBMDB = MDB/inc/BillAcceptor.hpp MDB/src/BillAcceptor.cpp MDB/IMDBDevice.hpp

SRC = main.cpp
OBJ = $(SRC) $(LIBPERIPH) $(LIBMDB)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -rf $(EXEC)