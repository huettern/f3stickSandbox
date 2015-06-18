#
#   Copyright (C) Noah Huetter. All rights reserved.
#   Author: Noah Huetter <noah.huetter@gmail.com>
#

#
# PROJECT name
#
PROJECT = stick
PROJ_BASE = .

#
# Directories
#
PROJ_OUT_DIR	= $(PROJ_BASE)/target
PROJ_SRC_DIR 	= $(PROJ_BASE)/src
HAL_DIR    		= $(PROJ_BASE)/STM32F3xx_HAL_Driver
CMSIS_DIR  		= $(PROJ_BASE)/CMSIS
CFG_DIR  		= $(PROJ_BASE)/config
DEV_DIR    		= $(CMSIS_DIR)/Device/ST/STM32F3xx

# MCU family and type in various capitalizations o_O
MCU_FAMILY = stm32f3xx
MCU_LC     = stm32f373xc
MCU_MC     = STM32F373xC
MCU_UC     = STM32F373XC

#
# User Source Files
#
CSRC = $(wildcard $(PROJ_SRC_DIR)/*.c)

#
# Include search paths (-I)
#
INCS       = -I$(PROJ_SRC_DIR)

#
# Defines
#
DEFS       =



###############################################################################
# Toolchain
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


###############################################################################
# Options

# Defines
DEFS      += -D$(MCU_MC) -DUSE_HAL_DRIVER

# Debug specific definitions for semihosting
#DEFS       += -DUSE_DBPRINTF

# HAL Sources
HALSRC 		= $(wildcard $(HAL_DIR)/Src/*.c)
# Config Sources
CFGSRC 		= $(wildcard $(CFG_DIR)/*.c)
# Startup Source
STRTSRC		= $(CMSIS_DIR)/Device/ST/STM32F3xx/Source/Templates/startup_$(MCU_LC).s

# Include search paths (-I)
INCS      += -I$(CMSIS_DIR)/Include
INCS      += -I$(DEV_DIR)/Include
INCS      += -I$(HAL_DIR)/Inc

# Library search paths
LIBS       = -L$(CMSIS_DIR)/Lib

# Compiler flags
CFLAGS     = -Wall -g -std=c99 -Os
CFLAGS    += -mlittle-endian -mcpu=$(MCU) -march=armv7e-m -mthumb
CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS    += -ffunction-sections -fdata-sections
CFLAGS    += $(INCS) $(DEFS)

# Linker Script
LDSCR = $(CMSIS_DIR)/Device/ST/STM32F3xx/Source/Templates/gcc/linker/STM32F373XC_FLASH.ld

# Linker flags
LDFLAGS    = -Wl,--gc-sections -Wl,-Map=$(PROJ_OUT_DIR)/$(PROJECT).map $(LIBS) -T$(LDSCR).ld

# Enable Semihosting
#LDFLAGS   += --specs=rdimon.specs -lc -lrdimon

# Source search paths
VPATH      = $(PROJ_SRC_DIR
VPATH     += $(HAL_DIR)/Src
VPATH     += $(DEV_DIR)/Source/

#
# Objects and Dependencies
#
COBJS       = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(CSRC:.c=.o)))
CDEPS       = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(CSRC:.c=.d))))

HALOBJS     = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(HALSRC:.c=.o)))
HALDEPS     = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(HALSRC:.c=.d)))

CFGOBJS     = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(CFGSRC:.c=.o)))
CFGDEPS     = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(CFGSRC:.c=.d)))

STRTOBJS    = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(STRTSRC:.s=.o)))
STRTDEPS    = $(addprefix $(PROJ_OUT_DIR)/,$(notdir $(STRTSRC:.s=.d)))

OBJS 		= $(COBJS) $(HALOBJS) $(CFGOBJS) $(STRTOBJS) 
###################################################
#-include $(CDEPS) $(HALDEPS) $(CFGDEPS) $(STRTDEPS)

.PHONY: all hal cfg user startup link debug clean

all: $(PROJECT).elf

$(PROJECT).elf: $(OBJS)
	@echo "[LD]      $(PROJECT).elf"
#	$Q$(CC) $(CFLAGS) $(LDFLAGS) src/startup_$(MCU_LC).s $^ -o $@
#	@echo "[OBJDUMP] $(TARGET).lst"
#	$Q$(OBJDUMP) -St $(TARGET).elf >$(TARGET).lst
#	@echo "[SIZE]    $(TARGET).elf"
#	$(SIZE) $(TARGET).elf

target/%.o : %.c
	@echo "[CC]      $(notdir $<)"
#	$Q$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF dep/$(*F).d


#$(COBJS): $(CSRC)
#	@echo COBJS = TGT=$@
#	@echo DEP=$<
#
#$(HALOBJS): $(addprefix $(HAL_DIR)/Src/, $(notdir $(HALOBJS:.o=.c)))
#	@echo HALOBJS = TGT=$@ DEP=$<
#
#$(CFGOBJS): $(CFGSRC)
#	@echo CFGOBJS = TGT=$@
#	@echo DEP=$<
#
#$(STRTOBJS): $(STRTSRC)
#	@echo STRTOBJS = TGT=$@
#	@echo DEP=$<


















## Output directory
#OUTPUT = $(PROJ_BASE)/target
#
##
## Global Defines
##
#UDEFS = -DSTM32F373xC
#
##PROJ_BASE = $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
#HAL_SRC_DIR = $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src
#CFG_SRC_DIR = $(PROJ_BASE)/config
#START_SRC_DIR = $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc
#
#
#
##
## User Sources
##
#CSRC = $(wildcard $(PROJ_SRC_DIR)/*.c)     \
#       $(wildcard $(PROJ_SRC_DIR)/*/*.c)   \
#       $(wildcard $(PROJ_SRC_DIR)/*/*/*.c)
#
#CPPSRC = $(wildcard $(CSRCDIR)/*.cpp)   \
#         $(wildcard $(CSRCDIR)/*/*.cpp)
#
#UINC =   $(CSRCDIR)/  		      
#
##
## Config sources
##
#CFGSRC = $(wildcard $(PROJ_BASE)/config/*.c)
#
##
## HAL Library
##
#HALSRC = $(wildcard $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src/*.c)
#HALINC = $(PROJ_BASE)/STM32F3xx_HAL_Driver/Inc/ \
#		 $(PROJ_BASE)/CMSIS/Include/			\
#		 $(PROJ_BASE)/config/					\
#		 $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Include/
#
#UDEFS += -DUSE_HAL_DRIVER
#
##
## Startup Code
##
#STARTSRC = $(PROJ_BASE)/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/startup_stm32f373xc.s
#
##
## Objects
##
#HALOBJ = $(patsubst %.c, %.o, $(HALSRC))
#
#COBJ = $(patsubst %.c, %.o, $(CSRC))
#
#CPPOBJ = $(patsubst %.cpp, %.o, $(CPPSRC))
#
#CFGOBJ = $(patsubst %.c, %.o, $(CFGSRC))
#
#STARTOBJ = $(patsubst %.s, %.o, $(STARTSRC))
#
#
#
##
## Git commit hash
##
#GIT_HASH := $(shell git rev-parse --short HEAD)
#ifneq ($(words $(GIT_HASH)),1)
#    GIT_HASH := "?"
#endif
#UDEFS += -DGIT_HASH=0x$(GIT_HASH)
#
#
## THUMB-specific options here
#TOPT = -mthumb -DTHUMB -mlittle-endian -Os
#
##
## Linker Script
##
#LDSCR = $(PROJ_BASE)/CMSIS\Device/ST/STM32F3xx/Source/Templates/gcc/linker/STM32F373XC_FLASH.ld
#LDFLAGS = -Wl,--gc-sections,-Map,$(OUTPUT)/$(PROJECT).map
#
#
#all:  $(HALOBJ) $(STARTOBJ) $(COBJ) $(CFGOBJ)
#	@echo Linking..
#	@echo Using linker script $(notdir $(LDSCR))
#	@$(LD) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -T,$(LDSCR)) $(LDFLAGS) $(HALOBJ) $(CFGOBJ) $(STARTOBJ) $(COBJ) -o $(OUTPUT)/$(PROJECT).elf
#	$(CP) -Oihex $(OUTPUT)/$(PROJECT).elf $(OUTPUT)/$(PROJECT).hex
#	@echo GIT_HASH = $(GIT_HASH)
#	@echo 
#	@echo 
#	@echo Linking successful
#	
## HAL driver sources
#$(HALOBJ): $(HAL_SRC_DIR)/%.o : $(HAL_SRC_DIR)/%.c
#	@echo Compiling HAL: $(notdir $@)
#	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@
#
## Config sources
#$(CFGOBJ): $(CFG_SRC_DIR)/%.o : $(CFG_SRC_DIR)/%.c
#	@echo Compiling config: $(notdir $@)
#	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@
#
## Startup code
#$(STARTOBJ): $(START_SRC_DIR)/%.o : $(START_SRC_DIR)/%.s
#	@echo Compiling startup Code: $(notdir $@)
#	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) -c $< -o $@
#
## User Code
#$(COBJ): $(PROJ_SRC_DIR)/%.o : $(PROJ_SRC_DIR)/%.c
#	@echo Compiling user Code: $(notdir $@)
#	@$(CC) $(TOPT) $(UDEFS) -mcpu=$(MCU) $(addprefix -I,$(HALINC)) $(addprefix -I,$(UINC)) -c $< -o $@
#
#
#clean:
#	rm -f $(PROJ_SRC_DIR)/*.o
#	rm -f $(HAL_SRC_DIR)/*.o
#	rm -f $(CFG_SRC_DIR)/*.o
#	rm -f $(START_SRC_DIR)/*.o
#	rm -f $(OUTPUT)/*.elf
#	rm -f $(OUTPUT)/*.map
#
#dbg:
#	@echo $(PROJ_BASE)
#	@echo $(HAL_DIR)