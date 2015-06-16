#
#   Copyright (C) Noah Huetter. All rights reserved.
#   Author: Noah Huetter <noah.huetter@gmail.com>
#

#
# PROJECT name
#
PROJECT = stick

# Output directory
OUTPUT = $(PROJ_BASE)/target

#
# Global Defines
#
UDEFS = -DSTM32F373xC

PROJ_BASE = $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
PROJ_SRC_DIR = $(abspath $(PROJ_BASE)/src)
HAL_SRC_DIR = $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src
CFG_SRC_DIR = $(PROJ_BASE)/config
START_SRC_DIR = $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc



#
# User Sources
#
CSRC = $(wildcard $(PROJ_SRC_DIR)/*.c)     \
       $(wildcard $(PROJ_SRC_DIR)/*/*.c)   \
       $(wildcard $(PROJ_SRC_DIR)/*/*/*.c)

CPPSRC = $(wildcard $(CSRCDIR)/*.cpp)   \
         $(wildcard $(CSRCDIR)/*/*.cpp)

UINC =   $(CSRCDIR)/  		      

#
# Config sources
#
CFGSRC = $(wildcard $(PROJ_BASE)/config/*.c)

#
# HAL Library
#
HALSRC = $(wildcard $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src/*.c)
HALINC = $(PROJ_BASE)/STM32F3xx_HAL_Driver/Inc/ \
		 $(PROJ_BASE)/CMSIS/Include/			\
		 $(PROJ_BASE)/config/					\
		 $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Include/

UDEFS += -DUSE_HAL_DRIVER

#
# Startup Code
#
STARTSRC = $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/startup_stm32f373xc.s

#
# Objects
#
HALOBJ = $(patsubst %.c, %.o, $(HALSRC))

COBJ = $(patsubst %.c, %.o, $(CSRC))

CPPOBJ = $(patsubst %.cpp, %.o, $(CPPSRC))

CFGOBJ = $(patsubst %.c, %.o, $(CFGSRC))

STARTOBJ = $(patsubst %.s, %.o, $(STARTSRC))


#
# Compiler options
#

MCU  = cortex-m4

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

#
# Git commit hash
#
GIT_HASH := $(shell git rev-parse --short HEAD)
ifneq ($(words $(GIT_HASH)),1)
    GIT_HASH := "?"
endif
UDEFS += -DGIT_HASH=0x$(GIT_HASH)


# THUMB-specific options here
TOPT = -mthumb -DTHUMB -mlittle-endian -Os

#
# Linker Script
#
LDSCR = $(PROJ_BASE)/CMSIS\Device/ST/STM32F3xx/Source/Templates/gcc/linker/STM32F373XC_FLASH.ld
LDFLAGS = -Wl,--gc-sections,-Map,$(OUTPUT)/$(PROJECT).map


all:  $(HALOBJ) $(STARTOBJ) $(COBJ) $(CFGOBJ)
	@echo Linking..
	@echo Using linker script $(notdir $(LDSCR))
	@$(LD) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -T,$(LDSCR)) $(LDFLAGS) $(HALOBJ) $(CFGOBJ) $(STARTOBJ) $(COBJ) -o $(OUTPUT)/$(PROJECT).elf
	$(CP) -Oihex $(OUTPUT)/$(PROJECT).elf $(OUTPUT)/$(PROJECT).hex
	@echo GIT_HASH = $(GIT_HASH)
	@echo 
	@echo 
	@echo Linking successful
	
# HAL driver sources
$(HALOBJ): $(HAL_SRC_DIR)/%.o : $(HAL_SRC_DIR)/%.c
	@echo Compiling HAL: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# Config sources
$(CFGOBJ): $(CFG_SRC_DIR)/%.o : $(CFG_SRC_DIR)/%.c
	@echo Compiling config: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# Startup code
$(STARTOBJ): $(START_SRC_DIR)/%.o : $(START_SRC_DIR)/%.s
	@echo Compiling startup Code: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# User Code
$(COBJ): $(PROJ_SRC_DIR)/%.o : $(PROJ_SRC_DIR)/%.c
	@echo Compiling user Code: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) $(addprefix -I,$(UINC)) -c $< -o $@


clean:
	rm -f $(PROJ_SRC_DIR)/*.o
	rm -f $(HAL_SRC_DIR)/*.o
	rm -f $(CFG_SRC_DIR)/*.o
	rm -f $(START_SRC_DIR)/*.o
	rm -f $(OUTPUT)/*.elf
	rm -f $(OUTPUT)/*.map
