#include "ms5611.h"

namespace airdamon { namespace sensors {

void MS5611::init(I2C * i2c, GPIO::gpio_write_t csb)
{
  i2c_ = i2c;

  // determine address
  addr_ = (csb == GPIO::HIGH) ? uv(I2CAddr::CSB_HIGH) : uv(I2CAddr::CSB_LOW);

  // check if device is present
  present_ = i2c_->write(addr_, uv(RegAddr::RESET));
  if (!present_) return;

  // wait for device to power up
  delay(10);

  // read the device PROM (128 bit)
  read_prom();
}

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void MS5611::read_prom()
{
  static constexpr uint8_t PROMADDRS = 8;

  for (uint8_t i=0; i<PROMADDRS; ++i) {
    // "The address of the PROM is embedded inside of the PROM read command
    //  using the a2, a1 and a0 bits." - datasheet, above Fig. 4.
    bool success = i2c_->write(addr_, uv(RegAddr::PROM_READ) + (i<<1));

    uint8_t buf[2] = { 0 };
    i2c_->read(addr_, buf, 2);

    prom_[i] = static_cast<uint16_t>(buf[0] << 8 | buf[1]);
  }
}

} // ns sensors
} // ns airdamon
