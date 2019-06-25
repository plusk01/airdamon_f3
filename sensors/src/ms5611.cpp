#include "ms5611.h"

namespace airdamon { namespace sensors {

void MS5611::init(I2C * i2c, GPIO::gpio_write_t csb)
{
  i2c_ = i2c;

  // determine address
  addr_ = (csb == GPIO::HIGH) ? uv(I2CAddr::CSB_HIGH) : uv(I2CAddr::CSB_LOW);

  //
  // Check to see that device exists and we can communicate with it
  //

  static constexpr uint8_t NUM_RETRIES = 3;
  for (uint8_t i=0; i<NUM_RETRIES; ++i) {
    // attempt to reset device
    i2c_->begin_tx(addr_);
    i2c_->write(uv(RegAddr::RESET));
    i2c_->end_tx();

    // wait for device to power up
    delay(10);

    // read the device PROM (128 bit)
    present_ = read_prom();
    if (present_) break;
  }
}

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

bool MS5611::read_prom()
{
  static constexpr uint8_t PROMADDRS = 8;

  for (uint8_t i=0; i<PROMADDRS; ++i) {
    // "The address of the PROM is embedded inside of the PROM read command
    //  using the a2, a1 and a0 bits." - datasheet, above Fig. 4.
    i2c_->begin_tx(addr_);
    i2c_->write(uv(RegAddr::PROM_READ) + (i<<1));
    bool success = i2c_->end_tx();
    if (!success) return false;

    uint8_t buf[2] = { 0 };
    uint8_t len = i2c_->request_from(addr_, buf, 2);
    if (len != 2) return false;

    prom_[i] = static_cast<uint16_t>(buf[0] << 8 | buf[1]);
  }

  return true;
}

} // ns sensors
} // ns airdamon
