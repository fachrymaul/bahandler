TARGET:=MDB2SerialConverter
TOOLCHAIN_PREFIX:=arm-none-eabi-
# Optimization level, can be [0, 1, 2, 3, s].
OPTLVL:=0
DBG:=-g
STARTUP:=$(CURDIR)
LINKER_SCRIPT:=$(CURDIR)/stm32f103c8tx_flash.ld
INCLUDE+=-I$(CURDIR)/Util/inc
INCLUDE+=-I$(CURDIR)/PeriphInterface
INCLUDE+=-I$(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/inc
INCLUDE+=-I$(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/sys/cmsis/inc
INCLUDE+=-I$(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/sys/STM32F10x_StdPeriph_Driver/inc
INCLUDE+=-I$(CURDIR)/LibMDBConverter/inc
INCLUDE+=-I$(CURDIR)/CRC16/inc
INCLUDE+=-I$(CURDIR)/MasterAndProxyHeader

BUILD_DIR = $(CURDIR)/build
BIN_DIR = $(CURDIR)/binary

# vpath is used so object files are written to the current directory instead
# of the same directory as their source files
vpath %.c $(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/sys/STM32F10x_StdPeriph_Driver/src \
			$(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/sys/cmsis/src
			
vpath %.cpp $(CURDIR)/Comm/src \
			$(CURDIR)/Util/src \
			$(CURDIR)/PeriphInterface/PeriphImplementation/stm32f10x/src \
			$(CURDIR)/LibMDBConverter/src \
			$(CURDIR)/CRC16/src \


vpath %.s $(STARTUP)
ASRC=startup_stm32f10x_md.s

# Main app Files
SRCXX+=main.cpp

# Communication Protocol
SRCXX+=MDBProxy.cpp

# Peripheral 
# SRCXX+=stm32f103xxGPIO.cpp
# SRCXX+=stm32f103xxUART.cpp
# SRCXX+=stm32f103xxUARTInterrupt.cpp

# Utility
SRCXX+=CRC16.cpp

# System Source Files
SRC+=system_stm32f10x.c

# Standard Peripheral Source Files
SRC+=misc.c
SRC+=stm32f10x_adc.c
SRC+=stm32f10x_bkp.c
SRC+=stm32f10x_can.c
SRC+=stm32f10x_cec.c
SRC+=stm32f10x_crc.c
SRC+=stm32f10x_dac.c
SRC+=stm32f10x_dbgmcu.c
SRC+=stm32f10x_dma.c
SRC+=stm32f10x_exti.c
SRC+=stm32f10x_flash.c
SRC+=stm32f10x_fsmc.c
SRC+=stm32f10x_gpio.c
SRC+=stm32f10x_i2c.c
SRC+=stm32f10x_iwdg.c
SRC+=stm32f10x_pwr.c
SRC+=stm32f10x_rcc.c
SRC+=stm32f10x_rtc.c
SRC+=stm32f10x_sdio.c
SRC+=stm32f10x_spi.c
SRC+=stm32f10x_tim.c
SRC+=stm32f10x_usart.c
SRC+=stm32f10x_wwdg.c

CDEFS=-DUSE_STDPERIPH_DRIVER
CDEFS+=-DSTM32F10X_MD
CDEFS+=-DHSE_VALUE=8000000

MCUFLAGS=-mcpu=cortex-m4 -mthumb -finline-functions -Wdouble-promotion
COMMONFLAGS=-O$(OPTLVL) $(DBG) -Wall -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS) -std=gnu99
CXXFLAGS=$(COMMONFLAGS) $(MCUFLAGS) $(INCLUDE) $(CDEFS) -std=c++11


LDLIBS=
LDFLAGS=$(MCUFLAGS) --specs=nosys.specs -fno-rtti -fno-exceptions -Wl,--gc-sections,-T$(LINKER_SCRIPT),-Map,$(BIN_DIR)/$(TARGET).map

CC=$(TOOLCHAIN_PREFIX)gcc
CXX=$(TOOLCHAIN_PREFIX)g++
LD=$(TOOLCHAIN_PREFIX)g++
OBJCOPY=$(TOOLCHAIN_PREFIX)objcopy
AS=$(TOOLCHAIN_PREFIX)as
AR=$(TOOLCHAIN_PREFIX)ar
GDB=$(TOOLCHAIN_PREFIX)gdb

OBJ += $(SRCXX:%.cpp=$(BUILD_DIR)/%.o)
OBJ += $(SRC:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: %.cpp
	@echo [CXX] $(notdir $<)
	@$(CXX) $(CXXFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o: %.c
	@echo [CC] $(notdir $<)
	@$(CC) $(CFLAGS) $< -c -o $@

all: $(OBJ)
	@echo [AS] $(ASRC)
	@$(AS) -o $(ASRC:%.s=$(BUILD_DIR)/%.o) $(STARTUP)/$(ASRC)
	@echo [LD] $(TARGET).elf
	@$(CXX) -o $(BIN_DIR)/$(TARGET).elf $(LDFLAGS) $(OBJ) $(ASRC:%.s=$(BUILD_DIR)/%.o) $(LDLIBS)
	@echo [HEX] $(TARGET).hex
	@$(OBJCOPY) -O ihex $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).hex
	@echo [BIN] $(TARGET).bin
	@$(OBJCOPY) -O binary $(BIN_DIR)/$(TARGET).elf $(BIN_DIR)/$(TARGET).bin

.PHONY: clean

ifdef OS
   RM = del /Q
   FixPath = $(subst /,\,$1)
else
   ifeq ($(shell uname), Linux)
      RM = rm -f
      FixPath = $1
   endif
endif

#clean:
#    $(RM) $(call FixPath,objs/*)
clean:
	@echo [RM] OBJ
	$(RM) $(call FixPath,$(OBJ))
	$(RM) $(call FixPath,$(ASRC:%.s=$(BUILD_DIR)/%.o))
	@echo [RM] BIN
	$(RM) $(call FixPath,$(BIN_DIR)/$(TARGET).elf)
	$(RM) $(call FixPath,$(BIN_DIR)/$(TARGET).hex)
	$(RM) $(call FixPath,$(BIN_DIR)/$(TARGET).bin)

flash:
	@st-flash write $(BIN_DIR)/$(TARGET).bin 0x8000000

debug:
	$(GDB) $(BIN_DIR)/$(TARGET).elf