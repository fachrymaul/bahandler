ODIR = $(CURDIR)/build

INCLUDE = -I$(CURDIR)/../CRC16/inc \
			-I$(CURDIR)/catch2 \
			-I$(CURDIR)/fakeit \
			-I$(CURDIR) \
			-I$(CURDIR)/../LibMDBConverter/inc \

vpath %.cpp $(CURDIR) \
			$(CURDIR)/../CRC16/src \
			$(CURDIR)/../CRC16/test \
			$(CURDIR)/../LibMDBConverter/src \
			$(CURDIR)/../LibMDBConverter/test \

# Main app Files
SRCXX+=main.cpp
SRCXX+=CRC16.cpp
SRCXX+=CRC16Test.cpp

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 $(INCLUDE) 

OBJ += $(SRCXX:%.cpp=$(ODIR)/%.o)

$(ODIR)/%.o: %.cpp
	@echo [CXX] $(notdir $<)
	@$(CXX) $(CXXFLAGS) $< -c -o $@

all: $(OBJ)
	$(CXX) -o unittest $^ $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

run:
	./unittest

run_complete:
	./unittest --success