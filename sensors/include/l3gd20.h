#ifndef L3GD20_H
#define L3GD20_H

#include <stdint.h>

#include "spi.h"
#include "gpio.h"

#include <flags/flags.hpp>

enum class _Power_Mode: uint8_t { POWERDOWN = 0x00, ACTIVE = 0x08 };
enum class _Output_DataRate: uint8_t { RATE1 = 0x00, RATE2 = 0x40, RATE3 = 0x80, RATE4 = 0xC0 };
enum class _Axes: uint8_t { X = 0x02, Y = 0x01, Z = 0x04, ENABLE = 0x07, DISABLE = 0x00 };
enum class _Bandwidth: uint8_t { BW1 = 0x00, BW2 = 0x10, BW3 = 0x20, BW4 = 0x30 };
enum class _BD_Update: uint8_t { CONTINUOUS = 0x00, SINGLE = 0x80 };
enum class _Endianness: uint8_t { LSB = 0x00, MSB = 0x40 };
enum class _FS_Selection: uint8_t { FS250 = 0x00, FS500 = 0x10, FS2000 = 0x20 };

ALLOW_FLAGS_FOR_ENUM(_Power_Mode)
ALLOW_FLAGS_FOR_ENUM(_Output_DataRate)
ALLOW_FLAGS_FOR_ENUM(_Axes)
ALLOW_FLAGS_FOR_ENUM(_Bandwidth)
ALLOW_FLAGS_FOR_ENUM(_BD_Update)
ALLOW_FLAGS_FOR_ENUM(_Endianness)
ALLOW_FLAGS_FOR_ENUM(_FS_Selection)

enum class _HPF_Mode: uint8_t { NORMAL_RES = 0x00, REF_SIGNAL = 0x10, NORMAL = 0x20, AUTORESET_INT = 0x30 };
enum class _HPF_CF: uint8_t {F0=0x00,F1=0x01,F2=0x02,F3=0x03,F4=0x04,F5=0x05,F6=0x06,F7=0x07,F8=0x08,F9=0x09};

ALLOW_FLAGS_FOR_ENUM(_HPF_Mode)
ALLOW_FLAGS_FOR_ENUM(_HPF_CF)


namespace sensors
{

  class L3GD20
  {
  public:
    enum class Boot_Mode: uint8_t { NORMAL = 0x00, REBOOTMEMORY = 0x80 };
    
    using Power_Mode      = ::_Power_Mode;
    using Output_DataRate = ::_Output_DataRate;
    using Axes            = ::_Axes;
    using Bandwidth       = ::_Bandwidth;
    using BD_Update       = ::_BD_Update;
    using Endianness      = ::_Endianness;
    using FS_Selection    = ::_FS_Selection;

    using HPF_Mode        = ::_HPF_Mode;
    using HPF_CF          = ::_HPF_CF;

  private:
    using power_mode_t      = ::flags::flags<Power_Mode>;
    using output_datarate_t = ::flags::flags<Output_DataRate>;
    using axes_t            = ::flags::flags<Axes>;
    using bandwidth_t       = ::flags::flags<Bandwidth>;
    using bd_update_t       = ::flags::flags<BD_Update>;
    using endianess_t       = ::flags::flags<Endianness>;
    using fs_selection_t    = ::flags::flags<FS_Selection>;

    using hpf_mode_t        = ::flags::flags<HPF_Mode>;
    using hpf_cf_t          = ::flags::flags<HPF_CF>;

    // define the registers
    enum class RegAddr : uint8_t
    {
      WHO_AM_I = 0x0F,
      CTRL_REG1 = 0x20,
      CTRL_REG2 = 0x21,
      CTRL_REG3 = 0x22,
      CTRL_REG4 = 0x23,
      CTRL_REG5 = 0x24,
      REFERENCE = 0x25,
      OUT_TEMP = 0x26,
      STATUS = 0x27,
      OUT_X_L = 0x28,
      OUT_X_H = 0x29,
      OUT_Y_L = 0x2A,
      OUT_Y_H = 0x2B,
      OUT_Z_L = 0x2C,
      OUT_Z_H = 0x2D,
      FIFO_CTRL = 0x2E,
      FIFO_SRC = 0x2F,
      INT1_CFG = 0x30,
      INT1_SRC = 0x31,
      INT1_TSH_XH = 0x32,
      INT1_TSH_XL = 0x33,
      INT1_TSH_YH = 0x34,
      INT1_TSH_TL = 0x35,
      INT1_TSH_ZH = 0x36,
      INT1_TSH_ZL = 0x37,
      INT1_DURATION = 0x38
    };

  public:

    struct DeviceParams
    {
      power_mode_t      Power_Mode;         // Power-down/Sleep/Normal mode
      output_datarate_t Output_DataRate;    // OUT datarate
      axes_t            Axes_Enable;        // Axes enabled
      bandwidth_t       Bandwidth;          // Bandwidth selection
      bd_update_t       BlockData_Update;   // Block data update
      endianess_t       Endianness;         // Endianness of data
      fs_selection_t    Full_Scale;         // Full-scale selection
    };


    struct FilterParams
    {
      hpf_mode_t  HPF_Mode;           // Internal filter mode
      hpf_cf_t    HPF_Cutoff_Freq;    // High-pass filter cut-off frequency
    };

    struct InterruptParams
    {
      uint8_t Latch_Request;      // Latch interrupt request into CLICK_SRC reg
      uint8_t Int_Axes;           // X, Y, Z axes interrupts
      uint8_t Int_ActiveEdge;     // Interrupt on active edge
    };

    void init(SPI* spi, DeviceParams* params);


  private:
    SPI* spi_;

    void write(RegAddr addr, uint8_t* buffer, uint16_t len);
    void read(RegAddr addr, uint8_t* buffer, uint16_t len);
    void chip_select(bool enable);

    void reboot();
  };

}

#endif // L3GD20_H