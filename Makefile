#
#   Copyright (C) Noah Huetter. All rights reserved.
#   Author: Noah Huetter <noah.huetter@gmail.com>
#

#
# PROJECT name
#
PROJECT = stick

# Output directory
OUTPUT = $(PROJ_BASE)/target/

#
# Global Defines
#
UDEFS = -DSTM32F373xC

PROJ_BASE = $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
PROJ_SRC_DIR = $(abspath $(PROJ_BASE)/src)

#
# User Sources
#
CSRC = $(wildcard $(PROJ_SRC_DIR)/*.c)     \
       $(wildcard $(PROJ_SRC_DIR)/*/*.c)   \
       $(wildcard $(PROJ_SRC_DIR)/*/*/*.c)

COBJ = $(addprefix $(OUTPUT),$(notdir $(subst .c,.o, $(CSRC))))

CPPSRC = $(wildcard $(PROJ_SRC_DIR)/*.cpp)   \
         $(wildcard $(PROJ_SRC_DIR)/*/*.cpp)

UINC =   $(PROJ_SRC_DIR)/  		\
         $(PROJ_SRC_DIR)/*/ 	\
         $(PROJ_SRC_DIR)/Inc/         

#
# Config sources
#
CFGSRC = $(wildcard $(PROJ_BASE)/config/*.c)
CFGOBJ = $(addprefix $(OUTPUT),$(notdir $(subst .c,.o, $(CFGSRC))))

#
# HAL Library
#
HALCSRC = $(wildcard $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src/*.c)

HALINC = $(PROJ_BASE)/STM32F3xx_HAL_Driver/Inc/ \
		 $(PROJ_BASE)/CMSIS/Include/			\
		 $(PROJ_BASE)/config /					\
		 $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Include/

UDEFS += -DUSE_HAL_DRIVER

HALOBJ = $(addprefix $(OUTPUT),$(notdir $(subst .c,.o, $(HALCSRC))))

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
# Startup Code
#
STARTSRC = $(PROJ_BASE)/CMSIS\Device/ST/STM32F3xx/Source/Templates/gcc/startup_stm32f373xc.s
STARTOBJ = $(addprefix $(OUTPUT),$(notdir $(subst .s,.o, $(STARTSRC))))

#
# Linker Script
#
LDSCR = $(PROJ_BASE)/CMSIS\Device/ST/STM32F3xx/Source/Templates/gcc/linker/STM32F373XC_FLASH.ld
LDFLAGS = -Wl,--gc-sections

# HAL driver sources
$(HALOBJ): $(HALCSRC)
	@echo Compiling HAL: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# Config sources
$(CFGOBJ): $(CFGSRC)
	@echo Compiling config: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# Startup code
$(STARTOBJ): $(STARTSRC)
	@echo Compiling startup Code: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@

# User Code
$(COBJ): $(CSRC)
	@echo Compiling user Code: $(notdir $@)
	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) $(addprefix -I,$(UINC)) -c $< -o $@


all: $(HALOBJ) $(STARTOBJ) $(COBJ) $(CFGOBJ)
	@echo Linking..
	@echo Using linker script $(notdir $(LDSCR))
	@$(LD) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -T,$(LDSCR)) $(LDFLAGS) $(HALOBJ) $(CFGOBJ) $(STARTOBJ) $(COBJ) -o main.elf
	@echo GIT_HASH = $(GIT_HASH)
	@echo 
	@echo 
	@echo Linking succeeded

clean:
	rm $(PROJ_BASE)/target/*.o
	rm $(PROJ_BASE)/STM32F3xx_HAL_Driver/Inc/*.gch
	rm $(PROJ_BASE)/*.elf