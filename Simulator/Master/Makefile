TARGET:=MDBProtocolMaster

# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL:=0
DBG:=-g

STARTUP:=$(CURDIR)

INCLUDE+=-I$(CURDIR)/Comm/inc
INCLUDE+=-I$(CURDIR)/Device/inc
INCLUDE+=-I$(CURDIR)/PeriphInterface
INCLUDE+=-I$(CURDIR)/PeriphInterface/PeriphImplementation/Linux/inc
INCLUDE+=-I$(CURDIR)/Util/inc

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.cpp $(CURDIR)/Comm/src \
			$(CURDIR)/Device/src \
			$(CURDIR)/Util/src \
			$(CURDIR)/PeriphInterface/PeriphImplementation/Linux/src

# Main app Files
# SRCXX+=main.cpp
SRCXX+=main_conv.cpp

# Communication Protocol
SRCXX+=MDBProtocolBase.cpp
SRCXX+=MDBProtocolConv.cpp
SRCXX+=MDBProtocolTermiosDummy.cpp
SRCXX+=MDBProtocolTermiosUSB.cpp
SRCXX+=TermiosSerialUSB.cpp
SRCXX+=TermiosSerialDummy.cpp
# SRCXX+=ICTProtocol.cpp

# Device MDB
SRCXX+=BillValidatorMDB.cpp

# Device ICT
# SRCXX+=BillValidatorICT.cpp

# Peripheral 
SRCXX+=LinuxMillisTimer.cpp

# Util
SRCXX+=Logger.cpp
SRCXX+=BitHelper.cpp

CDEFS=-DDEBUG
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS) -std=gnu99
CXXFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS) -std=c++11

LDLIBS=

CC=gcc
CXX=g++
LD=g++
OBJCOPY=objcopy
GDB=gdb

OBJ += $(SRCXX:%.cpp=$(BUILD_DIR)/%.o)
OBJ += $(SRC:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.cpp
	@echo [CXX] $(notdir $<)
	@$(CXX) $(CXXFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

all: $(OBJ)
	@echo [EXE] $(TARGET)
	@$(CXX) -o $(BIN_DIR)/$(TARGET) $(LDFLAGS) $(OBJ) $(LDLIBS)

.PHONY: clean

clean:
	@echo [RM] OBJ
	@rm -f $(OBJ)
	@echo [RM] BIN
	@rm -f $(BIN_DIR)/$(TARGET)

debug:
	$(GDB) $(BIN_DIR)/$(TARGET)