/**
 * TE Connectivity Measurement Specialties MS5611-01BA03
 * Barometric Pressure Sensor (U7 on Revo)
 */

#pragma once

#include <stdint.h>
#include <type_traits>

#include "i2c.h"
#include "gpio.h"

namespace airdamon { namespace sensors {

class MS5611
{
  private:

    // define the registers (LSB is always 0)
    enum class RegAddr : uint8_t
    {
      RESET = 0x1E,
      // Digital pressure value at different over-sampling rates
      CONV_D1_OSR_256 = 0x40,
      CONV_D1_OSR_512 = 0x42,
      CONV_D1_OSR_1024 = 0x44,
      CONV_D1_OSR_2048 = 0x46,
      CONV_D1_OSR_4096 = 0x48,
      // Digital pressure value at different over-sampling rates
      CONV_D2_OSR_256 = 0x50,
      CONV_D2_OSR_512 = 0x52,
      CONV_D2_OSR_1024 = 0x54,
      CONV_D2_OSR_2048 = 0x56,
      CONV_D2_OSR_4096 = 0x58,
      // Read ADC after requesting a conversion from measurements, D1 or D2
      ADC_READ = 0x00,
      // Read PROM (done after reset) for calibration params, serial num, CRC
      PROM_READ = 0xA0 // to 0xAE (8 PROM addresses)
    };

    // possible I2C device addresses
    enum class I2CAddr : uint8_t { CSB_HIGH = 0x76, CSB_LOW = 0x77 };

    // This constexpr (compile-time) function extracts the underlying
    // value of an enumeration based on its underlying type
    template<typename E>
    static constexpr auto uv(E e) -> typename std::underlying_type<E>::type 
    {
       return static_cast<typename std::underlying_type<E>::type>(e);
    }
public:
  MS5611() = default;
  ~MS5611() = default;

  /**
   * @brief      Initialization sequence for the MS5611
   *
   * @param      i2c   I2C device used for communication
   * @param[in]  csb   Chip Select pin. It's complement is
   *                   used as the LSB of its I2C address:
   *                   0x76 if pulled high, 0x77 if pulled low.
   */
  void init(I2C * i2c, GPIO::gpio_write_t csb = GPIO::LOW);

  bool present() const { return present_; }

private:
  I2C * i2c_; ///< comms dev obj
  bool present_ = false; ///< confirmed that this device is on the bus
  uint8_t addr_ = 0x00; ///< I2C address of the MS5611
  uint16_t prom_[8]; ///< 128 bit device programmable ROM

  /**
   * @brief      Read 128 bit device PROM
   * 
   *      8 addresses with 16 bit result
   *             1) factory data / setup
   *             2) C1 calibration coeff
   *             3) C2 calibration coeff
   *             4) C3 calibration coeff
   *             5) C4 calibration coeff
   *             6) C5 calibration coeff
   *             7) C6 calibration coeff
   *             8) serial code and CRC
   *
   * @return     True if successful
   */
  bool read_prom();

};

} // ns sensors
} // ns airdamon
