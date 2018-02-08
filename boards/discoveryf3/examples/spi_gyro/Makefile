###############################################################################
# Generic Makefile Template for C/C++ for use with STM32 Microcontrollers
#
# Adapted by PCL to allow chip and board level abstraction
#
# Copyright (c) 2018 - James Jackson and Parker Lusk
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

TARGET = spigyro

DEBUG ?= GDB

#################################
# Working directories
#################################

TARGET_DIR    = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
BOARD_DIR     = $(realpath $(TARGET_DIR)../..)
BIN_DIR       = build

#################################
# Board Files and Configuration
#################################

include $(BOARD_DIR)/board.mk

#################################
# Include Directories
#################################

INCLUDE_DIRS +=

#################################
# Source Files
#################################

# Search path and source files for the target CXX sources
VPATH     := $(VPATH):$(TARGET_DIR)
TARGET_SRC = $(notdir $(wildcard $(TARGET_DIR)/*.cpp))

# Append necessary C and CXX sources for this specific chip
ASOURCES   +=
CSOURCES   +=
CXXSOURCES += $(TARGET_SRC)

#################################
# Object List
#################################

OBJECTS  = $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(ASOURCES))))
OBJECTS += $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(CSOURCES))))
OBJECTS += $(addsuffix .o,$(addprefix $(BIN_DIR)/$(TARGET)/,$(basename $(CXXSOURCES))))

#################################
# Target Output Files
#################################

TARGET_ELF = $(BIN_DIR)/$(TARGET).elf
TARGET_HEX = $(BIN_DIR)/$(TARGET).hex
TARGET_BIN = $(BIN_DIR)/$(TARGET).bin

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

#CXX_STRICT_FLAGS += -std=c++11 -pedantic -pedantic-errors -Werror -Wall -Wextra \
#  -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Wlogical-op -Wmissing-include-dirs \
#  -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunused -Wvariadic-macros \
#  -Wctor-dtor-privacy -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel

CFLAGS    = $(CHIP_CFLAGS) $(BOARD_CFLAGS) $(LTO_FLAGS) $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS)
CFLAGS   += $(addprefix -I,$(INCLUDE_DIRS))

CXXFLAGS  = $(CHIP_CXXFLAGS) $(BOARD_CXXFLAGS) $(LTO_FLAGS) $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS)
CXXFLAGS += $(addprefix -I,$(INCLUDE_DIRS)) $(CXX_STRICT_FLAGS)

LDFLAGS   = $(CHIP_LDFLAGS)  $(BOARD_LDFLAGS)


#################################
# GNU ARM Embedded Toolchain
#################################

CC   = arm-none-eabi-gcc
CXX  = arm-none-eabi-g++
LD   = arm-none-eabi-ld
AR   = arm-none-eabi-ar
AS   = arm-none-eabi-as
CP   = arm-none-eabi-objcopy
OD   = arm-none-eabi-objdump
NM   = arm-none-eabi-nm
SIZE = arm-none-eabi-size
A2L  = arm-none-eabi-addr2line

#################################
# Build
#################################

$(TARGET_BIN): $(TARGET_HEX)
	$(CP) -I ihex -O binary $< $@

$(TARGET_HEX): $(TARGET_ELF)
	$(CP) -O ihex --set-start 0x8000000 $< $@

$(TARGET_ELF): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	$(SIZE) $(TARGET_ELF)

$(BIN_DIR)/$(TARGET)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CXX) -c -o $@ $(CXXFLAGS) $<

$(BIN_DIR)/$(TARGET)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CC) -c -o $@ $(CFLAGS) $<

$(BIN_DIR)/$(TARGET)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo %% $(notdir $<)
	@$(CC) -c -o $@ $(CFLAGS) $<


#################################
# Recipes
#################################

.PHONY: flash clean

clean:
	rm -rf $(BIN_DIR)

flash: $(TARGET_BIN)
	dfu-util -a 0 -s 0x08000000:leave -D $(TARGET_BIN)

print-%:
	@echo '$*=$($*)'