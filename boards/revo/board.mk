
BOARD = revo
CHIP  = stm32f4xx

#################################
# Working directories
#################################

BOARD_DIR    := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
CHIP_DIR      = $(realpath $(BOARD_DIR)../../chips/$(CHIP))
LIB_DIR       = $(realpath $(BOARD_DIR)../../lib)
SENSOR_DIR    = $(realpath $(BOARD_DIR)../../sensors)

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

# Append necessary C and CXX sources for this specific board
ASOURCES   +=
CSOURCES   += $(BOARD_SRC)
CXXSOURCES += $(SENSOR_SRC)

#################################
# Flags
#################################

BOARD_CFLAGS   = 
BOARD_CXXFLAGS =
BOARD_LDFLAGS  =