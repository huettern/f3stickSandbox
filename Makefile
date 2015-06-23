# STM32 Makefile for GNU toolchain and openocd
#
# This Makefile fetches the Cube firmware package from ST's' website.
# This includes: CMSIS, STM32 HAL, BSPs, USB drivers and examples.
#
# Usage:
#	make cube		Download and unzip Cube firmware
#	make program		Flash the board with OpenOCD
#	make openocd		Start OpenOCD
#	make debug		Start GDB and attach to OpenOCD
#	make dirs		Create subdirs like obj, dep, ..
#	make template		Prepare a simple example project in this dir
#
# Copyright	2015 Steffen Vogel
# License	http://www.gnu.org/licenses/gpl.txt GNU Public License
# Author	Steffen Vogel <post@steffenvogel.de>
# Link		http://www.steffenvogel.de
# 
###############################################################################
#
# Modified by Noah Huetter to fit the f3stick plattform and personal needs
# Full courtesy of Steffen Vogel
#
# License	http://www.gnu.org/licenses/gpl.txt GNU Public License
# Author	Noah Huetter <noahhuetter@gmail.com>
# Link		github.com/noah95
#
# Changes: 
#	- Removed Board dependency
#	- Added all HAL sources to SRCS by wildcard
# 	- User can modify include paths
#	- Seperate source dir 'config' for configuration files
# 	- Output target files in new folder 'target'
#	- Create intel-hex file for flashing
#	- Removed target template
#	- Linker script and startup code taken directly from cube dir

# A name common to all output files (elf, map, hex, bin, lst)
TARGET     = stick

# Your C files from the /src directory
SRCS       = main.c

# Your header file locations (add -I before path!)
INCS       = -Isrc

#GDBFLAGS   = 

# MCU family and type in various capitalizations
MCU_FAMILY = stm32f3xx
MCU_LC     = stm32f373xc
MCU_MC     = STM32F373xC
MCU_UC     = STM32F373XC

# Directories
ifeq ($(OS),Windows_NT)
	OCD_DIR     = C:/OpenOCD-0.8.0/share/openocd/scripts
else
	OCD_DIR		= /usr/share/openocd/scripts
endif

CUBE_DIR   = cube

HAL_DIR    = $(CUBE_DIR)/Drivers/STM32F3xx_HAL_Driver
CFG_DIR    = config
CMSIS_DIR  = $(CUBE_DIR)/Drivers/CMSIS

DEV_DIR    = $(CMSIS_DIR)/Device/ST/STM32F3xx

CUBE_URL   = http://www.st.com/st-web-ui/static/active/en/st_prod_software_internet/resource/technical/software/firmware/stm32cubef3.zip

# that's it, no need to change anything below this line!

###############################################################################
# Toolchain

PREFIX     = arm-none-eabi
CC         = $(PREFIX)-gcc
AR         = $(PREFIX)-ar
OBJCOPY    = $(PREFIX)-objcopy
OBJDUMP    = $(PREFIX)-objdump
SIZE       = $(PREFIX)-size
GDB        = $(PREFIX)-gdb

ifeq ($(OS),Windows_NT)
	OCD     = C:/OpenOCD-0.8.0/bin/openocd.exe
else
	OCD		= openocd
endif

###############################################################################
# Files

# Basic HAL libraries
SRCS      += $(notdir $(wildcard $(HAL_DIR)/Src/*.c))
# Config sources
SRCS      += $(wildcard $(CFG_DIR)/*.c)

# Linker scipt
LDSCRIPT 	= $(DEV_DIR)/Source/Templates/gcc/linker/$(MCU_UC)_FLASH.ld
# Startup file
STARTUP 	= $(DEV_DIR)/Source/Templates/gcc/startup_$(MCU_LC).s

###############################################################################
# Options

# Defines
DEFS       = -D$(MCU_MC) -DUSE_HAL_DRIVER

# Debug specific definitions for semihosting
DEFS       += -DUSE_DBPRINTF

# Include search paths (-I)
INCS      += -I$(CFG_DIR)
INCS      += -I$(CMSIS_DIR)/Include
INCS      += -I$(DEV_DIR)/Include
INCS      += -I$(HAL_DIR)/Inc

# Library search paths
LIBS       = -L$(CMSIS_DIR)/Lib

# Compiler flags
CFLAGS     = -Wall -g -std=c99 -Os
CFLAGS    += -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb
CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS    += -ffunction-sections -fdata-sections
CFLAGS    += $(INCS) $(DEFS)

# Linker flags
LDFLAGS    = -Wl,--gc-sections -Wl,-Map=target/$(TARGET).map $(LIBS) -T$(LDSCRIPT)

# Enable Semihosting
LDFLAGS   += --specs=rdimon.specs -lc -lrdimon

# Source search paths
VPATH      = ./src
VPATH     += $(CFG_DIR)
VPATH     += $(HAL_DIR)/Src
VPATH     += $(DEV_DIR)/Source/

OBJS       = $(addprefix obj/,$(notdir $(SRCS:.c=.o)))
DEPS       = $(addprefix dep/,$(notdir $(SRCS:.c=.d)))

OCDFLAGS   = -f config/f3stick_openocd.cfg

# Prettify output
V = 0
ifeq ($V, 0)
	Q = @
	P = > /dev/null
endif

###################################################

.PHONY: all dirs program debug clean

all: target/$(TARGET).elf

-include $(DEPS)

dirs: dep obj cube target
dep obj src target:
	@echo "[MKDIR]   $@"
	$Qmkdir -p $@

obj/%.o : %.c | dirs
	@echo "[CC]      $(notdir $<)"
	$Q$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF dep/$(*F).d

target/$(TARGET).elf: $(OBJS)
	@echo "[LD]      target/$(TARGET).elf"
	$Q$(CC) $(CFLAGS) $(LDFLAGS) $(STARTUP) $^ -o $@
	@echo "[OBJDUMP] target/$(TARGET).lst"
	$Q$(OBJDUMP) -St target/$(TARGET).elf  >target/$(TARGET).lst
	@echo "[OBJCOPY] target/$(TARGET).hex"
	$Q$(OBJCOPY) -Oihex target/$(TARGET).elf  target/$(TARGET).hex
	@echo "[SIZE]    target/$(TARGET).elf"
	$(SIZE) target/$(TARGET).elf

openocd:
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS)

program: all
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) -c "program target/$(TARGET).elf verify reset"

debug:
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) &
	$(GDB)  -ex "target extended localhost:3333" \
		-ex "monitor arm semihosting enable" \
		-ex "monitor reset halt" \
		-ex "load" \
		-ex "monitor reset init" \
		$(GDBFLAGS) target/$(TARGET).elf; \

cube:
	rm -fr $(CUBE_DIR)
	wget -O /tmp/cube.zip $(CUBE_URL)
	unzip /tmp/cube.zip
	mv STM32Cube* $(CUBE_DIR)
	chmod -R u+w $(CUBE_DIR)
	rm -f /tmp/cube.zip

clean:
	@echo "[RMDIR]   dep"          ; rm -fr dep
	@echo "[RMDIR]   obj"          ; rm -fr obj
	@echo "[RMDIR]   target"       ; rm -fr target