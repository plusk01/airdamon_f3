#################################
# Working directories
#################################

CHIP_DIR     := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
CMSIS_DIR     = $(LIB_DIR)/CMSIS
STDPERIPH_DIR = $(LIB_DIR)/STM32F4/STM32F4xx_StdPeriph_Driver
USBCORE_DIR   = $(LIB_DIR)/STM32F4/STM32_USB_Device_Library/Core
USBCDC_DIR    = $(LIB_DIR)/STM32F4/STM32_USB_Device_Library/Class/cdc
USBOTG_DIR    = $(LIB_DIR)/STM32F4/STM32_USB_OTG_Driver
PRINTF_DIR    = $(LIB_DIR)/printf
STARTUP_DIR   = $(CHIP_DIR)/startup
VCP_DIR       = $(CHIP_DIR)/vcp

#################################
# Include Directories
#################################

INCLUDE_DIRS += $(CHIP_DIR) \
								$(CHIP_DIR)/include \
                $(STDPERIPH_DIR)/inc \
                $(USBCORE_DIR)/inc \
                $(USBCDC_DIR)/inc \
                $(USBOTG_DIR)/inc \
                $(CMSIS_DIR)/CM4/CoreSupport \
                $(CMSIS_DIR)/CM4/DeviceSupport/ST/STM32F4xx \
                $(PRINTF_DIR) \
                $(VCP_DIR)

#################################
# Source Files
#################################

# Start up files
VPATH      := $(VPATH):$(STARTUP_DIR)
STARTUP_SRC = $(notdir $(wildcard $(STARTUP_DIR)/*.S))
LDSCRIPT    = $(STARTUP_DIR)/stm32f405.ld

# Search path and source files for the ST stdperiph library.
# Exclude unused APIs for efficiency
VPATH         := $(VPATH):$(STDPERIPH_DIR)/src
STDPERIPH_SRC  = $(notdir $(wildcard $(STDPERIPH_DIR)/src/*.c))
EXCLUDES  		 = stm32f4xx_crc.c stm32f4xx_can.c stm32f4xx_fmc.c \
           				stm32f4xx_fsmc.c stm32f4xx_sai.c stm32f4xx_cec.c \
           				stm32f4xx_dsi.c stm32f4xx_flash_ramfunc.c \
           				stm32f4xx_fmpi2c.c stm32f4xx_lptim.c stm32f4xx_qspi.c
STDPERIPH_SRC := $(filter-out ${EXCLUDES}, $(STDPERIPH_SRC))

# Search path and source files for the USB Core library
VPATH      := $(VPATH):$(USBCORE_DIR)/src
USBCORE_SRC = $(notdir $(wildcard $(USBCORE_DIR)/src/*.c))

# Search path and source files for the USB CDC library
# Exclude unused APIs (e.g., templates)
VPATH      := $(VPATH):$(USBCDC_DIR)/src
USBCDC_SRC  = $(notdir $(wildcard $(USBCDC_DIR)/src/*.c))
EXCLUDES	  = usbd_cdc_if_template.c
USBCDC_SRC := $(filter-out ${EXCLUDES}, $(USBCDC_SRC))

# Search path and source files for the USB OTG library
VPATH      := $(VPATH):$(USBOTG_DIR)/src
USBOTG_SRC  = $(notdir $(wildcard $(USBOTG_DIR)/src/*.c))
EXCLUDES	  = usb_bsp_template.c usb_hcd_int.c usb_hcd.c usb_otg.c
USBOTG_SRC := $(filter-out ${EXCLUDES}, $(USBOTG_SRC))

# Search path and source files for the printf library
VPATH      := $(VPATH):$(PRINTF_DIR)
PRINTF_SRC  = $(notdir $(wildcard $(PRINTF_DIR)/*.c))

# Search path and source files for the USB VCP application source
VPATH  := $(VPATH):$(VCP_DIR)
VCP_SRC = $(notdir $(wildcard $(VCP_DIR)/*.c))

# Search path and source files for the chip peripheral CXX sources
VPATH   := $(VPATH):$(CHIP_DIR)/src
CHIP_SRC = $(notdir $(wildcard $(CHIP_DIR)/src/*.cpp))

# Search path and source files for the chip system C source
VPATH   := $(VPATH):$(CHIP_DIR)
SYS_SRC = $(CHIP_DIR)/system.c

# Append necessary C and CXX sources for this specific chip
ASOURCES   += $(STARTUP_SRC)
CSOURCES   += $(STDPERIPH_SRC) $(USBCORE_SRC) $(USBCDC_SRC) $(USBOTG_SRC) \
							$(VCP_SRC) $(PRINTF_SRC) $(SYS_SRC)
CXXSOURCES += $(CHIP_SRC)

#################################
# Flags
#################################

# Manufacturer name that shows up when plugged into USB
FIRMWARE_NAME_STRING ?= AIRDAMON

# Make the binary as small as possible
C_FILE_SIZE_FLAGS   = -ffunction-sections -fdata-sections -fno-exceptions
CXX_FILE_SIZE_FLAGS = -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti

# Microcontroller flags
MCFLAGS = -mcpu=cortex-m4 -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -Wdouble-promotion

# Chip-specific defines
DEFS = -DSTM32F40_41xxx -D__CORTEX_M4 -D__FPU_PRESENT -DARM_MATH_CM4 \
				-DWORDS_STACK_SIZE=200 -DUSE_STDPERIPH_DRIVER \
				-DFIRMWARE_NAME_STRING=\"$(FIRMWARE_NAME_STRING)\"

CHIP_CFLAGS   = -c -std=c99   $(MCFLAGS) $(DEFS) $(C_FILE_SIZE_FLAGS)
CHIP_CXXFLAGS = -c -std=c++11 $(MCFLAGS) $(DEFS) $(CXX_FILE_SIZE_FLAGS)
CHIP_LDFLAGS  = -T$(LDSCRIPT) $(MCFLAGS) -Wl,-L$(STARTUP_DIR) -lm -lc --specs=nano.specs --specs=rdimon.specs -static -Wl,-gc-sections