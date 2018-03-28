#include "mpu6500.h"

// to be used in the ISR at the bottom of this file
airdamon::sensors::MPU6500* IMUPtr = nullptr;

namespace airdamon { namespace sensors {

void MPU6500::init(SPI* spi, GPIO* cs)
{
  spi_ = spi;
  cs_ = cs;

  // setup the Tx / Rx buffers
  // let the device know that we are reading so that the
  // initial reg addr will automatically be incremented by device
  static constexpr uint8_t read_addr = uv(RegAddr::ACCEL_XOUT_H) | uv(RegAddr::READ_CMD);
  buff_tx_[0] = read_addr;

  // ensure that the SPI line is inactive
  chip_select(false);

  // for configuration, use a slow SPI clock (large divisor)
  constexpr float SLOW_HZ = 0.5625*1e6; // 72MHz / 128 = 0.5625 MHz
  spi_->set_divisor(SystemCoreClock/SLOW_HZ);


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


  // now that config is over, use a faster SPI clock for data transfer
  constexpr float FAST_HZ = 36*1e6; // 72MHz / 2 = 36 MHz
  spi_->set_divisor(SystemCoreClock/FAST_HZ);

  // connect the EXTI with this object
  IMUPtr = this;

  // Setup the EXTI pin for interrupts from the MPU6500, when data is ready
  init_EXTI();
}

// ----------------------------------------------------------------------------

void MPU6500::read(float* accel, float* gyro, float* temp, uint64_t* time_us)
{

  // read_blocking(accel, gyro, temp);
  // *time_us = imu_timestamp_us_;
  // return;

  // deep copy data
  accel[0] = accel_[0];
  accel[1] = accel_[1];
  accel[2] = accel_[2];
  gyro[0] = gyro_[0];
  gyro[1] = gyro_[1];
  gyro[2] = gyro_[2];
  *temp = temp_;
  *time_us = imu_timestamp_us_;

  // the newest data has been read
  new_data_ = false;
}

// ----------------------------------------------------------------------------

void MPU6500::read_blocking(float* accel, float* gyro, float* temp)
{
  // read the 14 registers containing x, y, z accel/gyro and temp (16 bits each)
  uint8_t buffer[14] = {0};
  
  // let the device know that we are reading
  // so that the initial reg addr will
  // automatically be incremented by device
  uint8_t read_addr = uv(RegAddr::ACCEL_XOUT_H) | uv(RegAddr::READ_CMD);

  // signal beginning of transmission to the MPU6500 device
  chip_select(true);

#if 0 // no DMA
  // send address of register to write to
  spi_->transfer_byte(read_addr);

  // Send the data to device (MSB first)
  while (len > 0)
  {
    *buffer = spi_->transfer_byte(uv(RegAddr::DUMMY_BYTE));
    len--;
    buffer++;
  }
#else // use DMA, but blocking
  uint8_t out[15] = { 0 };
  uint8_t in[15] = { 0 };

  out[0] = read_addr;
  spi_->transfer(out, 1+14, in, nullptr);
  while (spi_->is_busy());
  for (int i=0; i<14; i++)
    buffer[i] = in[i+1];
#endif

  // signal end of transmission to the MPU6500 device
  chip_select(false);

  // decode the buffer into accel, gyro, temp values
  decode(buffer, accel, gyro, temp);
}

// ----------------------------------------------------------------------------

void MPU6500::handle_exti_isr()
{
  // There is data ready to be read from the MPU via SPI.

  // Timestamp the IMU data that will be coming in over SPI
  imu_timestamp_us_ = micros();

    // perform the transmission to the MPU6500 device
  chip_select(true);
  spi_->transfer(buff_tx_, 1+14, buff_rx_, std::bind(&MPU6500::data_rx_cb, this));
  chip_select(false);

  // Note: We don't care about the first received bytes, only the 14 data bytes
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void MPU6500::init_EXTI()
{
  // Set up the EXTI pin
  exti_.init(GPIOC, GPIO_Pin_13, GPIO::INPUT);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);

  EXTI_InitTypeDef EXTI_InitStruct;
  EXTI_InitStruct.EXTI_Line = EXTI_Line13;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x03;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x03;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

// ----------------------------------------------------------------------------

void MPU6500::write(RegAddr addr, uint8_t data)
{
  chip_select(true);
  spi_->transfer_byte(uv(addr));
  spi_->transfer_byte(data);
  chip_select(false);
  // delayMicroseconds(1);
}

// ----------------------------------------------------------------------------

void MPU6500::chip_select(bool enable)
{
  if (enable)
    cs_->write(GPIO::LOW);
  else
    cs_->write(GPIO::HIGH);
}

// ----------------------------------------------------------------------------

void MPU6500::decode(uint8_t* buffer, float* accel, float* gyro, float* temp)
{
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

// ----------------------------------------------------------------------------

void MPU6500::data_rx_cb()
{
  // Throw away the first byte that we received
  decode(buff_rx_+1, accel_, gyro_, &temp_);

  // we have new data to read
  new_data_ = true;
}

}} // ns airdamon::sensors

// ----------------------------------------------------------------------------
// IRQ Handlers associated with external interrupts (EXTI) from MPU
// ----------------------------------------------------------------------------

// DMA SPI Rx: SPI1 to receive buffer complete
extern "C" void EXTI15_10_IRQHandler()
{
  EXTI_ClearITPendingBit(EXTI_Line13);
  IMUPtr->handle_exti_isr();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------