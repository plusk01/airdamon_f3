#include "mpu6500.h"

namespace sensors {

  void MPU6500::init(SPI* spi, GPIO* cs)
  {
    spi_ = spi;
    cs_ = cs;

    // Configure STM32F3 EXTI here...

    // ensure that the SPI line is inactive
    chip_select(false);

    // for configuration, use a slow SPI clock (large divisor)
    spi_->set_divisor(128); //  72MHz / 128 = 0.5625 MHz

    // perform a device reset to restore default settings.
    write(RegAddr::PWR_MGMT_1, uv(PWR_MGMT_1::DEV_RST));
    delay(150); // wait for device to reset

    // disable I2C interface and put in SPI only mode
    write(RegAddr::USER_CTRL, uv(USER_CTRL::I2C_IF_DIS));

    write(RegAddr::PWR_MGMT_1, uv(PWR_MGMT_1::CLKSEL_AUTO));
    write(RegAddr::SMPLRT_DIV, 0x00); // Sample Rate divisor == 0
    write(RegAddr::CONFIG, 1); // DLPF_CFG = 184Hz, 2.9ms delay, 1kHz Fs for Gyro
    write(RegAddr::GYRO_CONFIG, 0x18); // +/-2000dps, FCHOICE_B == 2'b00
    write(RegAddr::ACCEL_CONFIG, 0x18); // +/-16g

    write(RegAddr::INT_PIN_CFG, 0x10); // INT_ANYRD_2CLEAR -- IRQ cleared on any read
    write(RegAddr::INT_ENABLE, 0x01); // data ready interrupt enabled
  }

  // ----------------------------------------------------------------------------

  void MPU6500::read(float& x, float& y, float& z)
  {

  }

  // --------------------------------------------------------------------------
  // Private Methods
  // --------------------------------------------------------------------------

  void MPU6500::read(RegAddr addr, uint8_t data)
  {

  }

  // --------------------------------------------------------------------------

  void MPU6500::write(RegAddr addr, uint8_t data)
  {
    chip_select(true);
    spi_->transfer_byte(uv(addr));
    spi_->transfer_byte(data);
    chip_select(false);
    // delayMicroseconds(1);
  }

  // --------------------------------------------------------------------------

  void MPU6500::chip_select(bool enable)
  {
    if (enable)
      cs_->write(GPIO::LOW);
    else
      cs_->write(GPIO::HIGH);
  }

  // --------------------------------------------------------------------------
}