#include "mpu6500.h"

namespace airdamon { namespace sensors {

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
    delay(100); // wait for device to reset, p. 42

    // reset gyro, accel, temp signal path
    write(RegAddr::SIGNAL_PATH_RESET, 0x07);
    delay(100); // wait for device to reset, p. 42


    // disable I2C interface and put in SPI only mode
    write(RegAddr::USER_CTRL, uv(USER_CTRL::I2C_IF_DIS) | uv(USER_CTRL::SIG_COND_RST));

    write(RegAddr::PWR_MGMT_1, uv(PWR_MGMT_1::CLKSEL_AUTO));
    write(RegAddr::SMPLRT_DIV, 0x00); // Sample Rate divisor == 0
    write(RegAddr::CONFIG, 1); // DLPF_CFG = 184Hz, 2.9ms delay, 1kHz Fs for Gyro
    // Default: 1kHz Accel sampling, 480Hz cutoff
    write(RegAddr::GYRO_CONFIG, 0x18); // +/-2000dps, FCHOICE_B == 2'b00
    write(RegAddr::ACCEL_CONFIG, 0x18); // +/-16g

    write(RegAddr::INT_PIN_CFG, 0x10); // INT_ANYRD_2CLEAR -- IRQ cleared on any read
    write(RegAddr::INT_ENABLE, 0x01); // data ready interrupt enabled
  }

  // ----------------------------------------------------------------------------

  void MPU6500::read(float* accel, float* gyro, float* temp)
  {
    // read the 14 registers containing x, y, z accel/gyro and temp (16 bits each)
    uint8_t buffer[14] = {0};
    read(RegAddr::ACCEL_XOUT_H, buffer, 14);

    // reconstruct high and low bits. Note that the value is stored
    // as two's complement, so make sure to cast to signed at the end.
    // Make sure to cast BEFORE shifting.
    int16_t raw[7] = {0};
    for (int i=0; i<7; i++)
    {
      const uint16_t high = static_cast<uint16_t>(buffer[2*i]) << 8;
      raw[i] = static_cast<int16_t>(high | buffer[2*i+1]);
    }

    // Accel sensitivity scale factor (g/lsb)
    // Accel measurements are 16 bit two's complement -- therefore, there
    // is half the positive range: 0x7FFF
    constexpr float accel_FS = 16.0; // depends on the reg values
    constexpr float g = 9.80665;
    constexpr float accel_scale = (accel_FS * g) / 0x7FFF;

    // gyro sensitivity scale factor (dps/lsb) (degrees per second == dps)
    // Gyro measurements are 16 bit two's complement -- therefore, there
    // is half the positive range: 0x7FFF
    constexpr float gyro_FS = 2000.0; // depends on the reg values
    constexpr float gyro_scale = gyro_FS / 0x7FFF;

    // temperature sensitivity
    constexpr float temp_scale = (1.0 / 333.87);

    // divide by the sensitivity
    accel[0] = static_cast<float>(raw[0])*accel_scale;
    accel[1] = static_cast<float>(raw[1])*accel_scale;
    accel[2] = static_cast<float>(raw[2])*accel_scale;

    *temp = static_cast<float>(raw[3])*temp_scale;

    gyro[0] = static_cast<float>(raw[4])*gyro_scale;
    gyro[1] = static_cast<float>(raw[5])*gyro_scale;
    gyro[2] = static_cast<float>(raw[6])*gyro_scale;
  }

  // --------------------------------------------------------------------------
  // Private Methods
  // --------------------------------------------------------------------------

  void MPU6500::read(RegAddr addr, uint8_t* buffer, uint16_t len)
  {
    // let the device know that we are reading
    // so that the initial reg addr will
    // automatically be incremented by device
    uint8_t read_addr = uv(addr) | uv(RegAddr::READ_CMD);

    // signal beginning of transmission to the MPU6500 device
    chip_select(true);

    // // send address of register to write to
    // spi_->transfer_byte(read_addr);

    // // Send the data to device (MSB first)
    // while (len > 0)
    // {
    //   *buffer = spi_->transfer_byte(uv(RegAddr::DUMMY_BYTE));
    //   len--;
    //   buffer++;
    // }

    uint8_t out[15] = { 0 };
    uint8_t in[15] = { 0 };

    out[0] = read_addr;
    spi_->transfer(out, 1+14, in, nullptr);
    while (spi_->is_busy());
    for (int i=0; i<14; i++)
      buffer[i] = in[i+1];

    // signal end of transmission to the MPU6500 device
    chip_select(false);
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

}}