###############################################################################
# Generic Makefile Template for C/C++ for use with STM32 Microcontrollers
#
# Copyright (c) 2016 - James Jackson
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#    * Neither the name of the <organization> nor the
#      names of its contributors may be used to endorse or promote products
#      derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BOARD ?= betafpv
CHIP = stm32f30xC

DEBUG ?= GDB

#################################
# GNU ARM Embedded Toolchain
#################################
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++
LD=arm-none-eabi-ld
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump
NM=arm-none-eabi-nm
SIZE=arm-none-eabi-size
A2L=arm-none-eabi-addr2line

#################################
# Working directories
#################################
BOARD_DIR     = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
CHIP_DIR      = $(realpath $(BOARD_DIR)../../chips/$(CHIP))
LIB_DIR       = $(realpath $(BOARD_DIR)../../lib)
SENSOR_DIR    = $(realpath $(BOARD_DIR)../../sensors)
BIN_DIR       = obj

#################################
# Chip Files and Configuration
#################################

include $(CHIP_DIR)/chip.mk

#################################
# Include Directories
#################################

INCLUDE_DIRS += $(BOARD_DIR) $(SENSOR_DIR)/include

#################################
# Source Files
#################################

# Search path and source files for the board sources
VPATH    := $(VPATH):$(BOARD_DIR)
BOARD_SRC = $(notdir $(wildcard $(BOARD_DIR)/*.c))

# Search path and source files for the sensor/component CXX sources
VPATH     := $(VPATH):$(SENSOR_DIR)/src
SENSOR_SRC = $(notdir $(wildcard $(SENSOR_DIR)/src/*.cpp))

# Append necessary C and CXX sources for this specific chip
CSOURCES += $(BOARD_SRC)
CXXSOURCES += $(SENSOR_SRC)

#################################
# Object List
#################################
OBJECTS  = $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(ASOURCES))))
OBJECTS += $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(CSOURCES))))
OBJECTS += $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(CXXSOURCES))))

#################################
# Target Output Files
#################################
STATIC_LIB = $(BIN_DIR)/$(BOARD).a

#################################
# Debug Config
#################################
ifeq ($(DEBUG), GDB)
$(info ************  BUILDING DEBUG SYMBOLS ************)
DEBUG_FLAGS = -ggdb3
OPTIMIZE = -Og
else
$(info ************  BUILDING RELEASE ************)
DEBUG_FLAGS = -g0
OPTIMIZE = -O3
LTO_FLAGS = -flto -fuse-linker-plugin $(OPTIMIZE)
endif

#################################
# Flags
#################################

# #CXX_STRICT_FLAGS += -std=c++11 -pedantic -pedantic-errors -Werror -Wall -Wextra \
# #  -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wlogical-op -Wmissing-include-dirs \
# #  -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunused -Wvariadic-macros \
# #  -Wctor-dtor-privacy -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel
# FILE_SIZE_FLAGS += -ffunction-sections -fdata-sections -fno-exceptions
# CXX_FILE_SIZE_FLAGS = $(C_FILE_SIZE_FLAGS) -fno-rtti

# MCFLAGS=-mcpu=cortex-m4 -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion
# DEFS=-DSTM32F303xC -D__CORTEX_M4 -D__FPU_PRESENT -DWORDS_STACK_SIZE=200 -DUSE_STDPERIPH_DRIVER -DTARGET_$(BOARD)
# CFLAGS=-c $(MCFLAGS) $(DEFS) $(OPTIMIZE) $(DEBUG_FLAGS) $(FILE_SIZE_FLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -std=c99
# CXXFLAGS=-c $(MCFLAGS) $(DEFS) $(OPTIMIZE) $(DEBUG_FLAGS) $(CXX_FILE_SIZE_FLAGS) $(CXX_STRICT_FLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -std=c++11
# LDFLAGS =-T $(LDSCRIPT) -Wl,-L$(STARTUP_DIR) $(MCFLAGS) -lm -lc --specs=nano.specs --specs=rdimon.specs $(ARCH_FLAGS) $(LTO_FLAGS) $(DEBUG_FLAGS) -static  -Wl,-gc-sections

#################################
# Build Static Library
#################################
$(STATIC_LIB): $(OBJECTS)
	$(AR) rcs $@ $^
	$(SIZE) $(STATIC_LIB)

$(BIN_DIR)/$(TARGET)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CXX) -c -o $@ $(CXXFLAGS) $<

$(BIN_DIR)/$(TARGET)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(BIN_DIR)/$(TARGET)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CC) -c -o $@ $(CFLAGS) $<


#################################
# Recipes
#################################
.PHONY: clean

clean:
	rm -rf $(BIN_DIR)

print-%:
	@echo '$*=$($*)'