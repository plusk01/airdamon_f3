#ifndef L3GD20_H
#define L3GD20_H

#include <stdint.h>
#include <type_traits> //for std::underlying_type

#include "spi.h"
#include "gpio.h"

namespace sensors
{

  class L3GD20
  {
  private:

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
      INT1_DURATION = 0x38,
      // addr read/write config
      READWRITE_CMD = 0x80,
      MULTIPLEBYTE_CMD = 0x40,
      DUMMY_BYTE = 0x00
    };

    // This constexpr (compile-time) function extracts the underlying
    // value of an enumeration based on its underlying type
    template<typename E>
    static constexpr auto uv(E e) -> typename std::underlying_type<E>::type 
    {
       return static_cast<typename std::underlying_type<E>::type>(e);
    }

  public:
    // CTRL_REG1 bits
    enum class Power_Mode: uint8_t { POWERDOWN = 0x00, ACTIVE = 0x08 };
    enum class Output_DataRate: uint8_t { RATE1 = 0x00, RATE2 = 0x40, RATE3 = 0x80, RATE4 = 0xC0 };
    enum class Axes: uint8_t { X = 0x02, Y = 0x01, Z = 0x04, ENABLE = 0x07, DISABLE = 0x00 };
    enum class Bandwidth: uint8_t { BW1 = 0x00, BW2 = 0x10, BW3 = 0x20, BW4 = 0x30 };

    // CTRL_REG2 bits
    enum class HPF_Mode: uint8_t { NORMAL_RES = 0x00, REF_SIGNAL = 0x10, NORMAL = 0x20, AUTORESET_INT = 0x30 };
    enum class HPF_CF: uint8_t { F0=0x00,F1=0x01,F2=0x02,F3=0x03,F4=0x04,F5=0x05,F6=0x06,F7=0x07,F8=0x08,F9=0x09 };

    // CTRL_REG4 bits
    enum class BD_Update: uint8_t { CONTINUOUS = 0x00, SINGLE = 0x80 };
    enum class Endianness: uint8_t { LSB = 0x00, MSB = 0x40 };
    enum class FS_Selection: uint8_t { FS250 = 0x00, FS500 = 0x10, FS2000 = 0x20 };

    // CTRL_REG5 bits
    enum class Boot_Mode: uint8_t { NORMAL = 0x00, REBOOTMEMORY = 0x80 };
    enum class HPF_State: uint8_t { ENABLE = 0x10, DISABLE = 0x00 };

    struct DeviceParams
    {
      Power_Mode      power_mode;         // Power-down/Sleep/Normal mode
      Output_DataRate output_datarate;    // OUT datarate
      Axes            axes_enable;        // Axes enabled
      Bandwidth       bandwidth;          // Bandwidth selection
      BD_Update       blockdata_update;   // Block data update
      Endianness      endianness;         // Endianness of data
      FS_Selection    full_scale;         // Full-scale selection
    };


    struct FilterParams
    {
      HPF_Mode  hpf_mode;           // Internal filter mode
      HPF_CF    hpf_cutoff_freq;    // High-pass filter cut-off frequency
    };

    struct InterruptParams
    {
      uint8_t Latch_Request;      // Latch interrupt request into CLICK_SRC reg
      uint8_t Int_Axes;           // X, Y, Z axes interrupts
      uint8_t Int_ActiveEdge;     // Interrupt on active edge
    };

    void init(SPI* spi, GPIO* cs, DeviceParams* params);
    void enable_hpfilter(FilterParams* params);
    void read_rates(float& x, float& y, float& z);


  private:
    SPI* spi_;
    GPIO* cs_;

    void write(RegAddr addr, uint8_t* buffer, uint16_t len);
    void read(RegAddr addr, uint8_t* buffer, uint16_t len);
    void chip_select(bool enable);

    void reboot();
  };

}

#endif // L3GD20_H