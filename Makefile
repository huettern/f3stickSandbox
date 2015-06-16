#
#   Copyright (C) Noah Huetter. All rights reserved.
#   Author: Noah Huetter <noah.huetter@gmail.com>
#

#
# PROJECT name
#
PROJECT = px4esc


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

CPPSRC = $(wildcard $(PROJ_SRC_DIR)/*.cpp)   \
         $(wildcard $(PROJ_SRC_DIR)/*/*.cpp)

UINC =   $(wildcard $(PROJ_SRC_DIR)/*.h)   \
         $(wildcard $(PROJ_SRC_DIR)/*/*.h) \
         $(wildcard $(PROJ_SRC_DIR)/Inc/*.h)        

#
# HAL Library
#

HALCSRC = $(wildcard $(PROJ_BASE)/STM32F3xx_HAL_Driver/Src/*.c)

HALINC = $(wildcard $(PROJ_BASE)/STM32F3xx_HAL_Driver/Inc/*.h)

UDEFS += -DUSE_HAL_DRIVER

#
# Compiler options
#

MCU  = cortex-m4

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
LD   = $(TRGT)g++
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


all:
	@echo GIT_HASH = $(GIT_HASH)


hal:
	@echo HALCSRC = $(HALCSRC)

