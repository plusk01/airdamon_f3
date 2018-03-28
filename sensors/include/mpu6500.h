#ifndef MPU6500_H
#define MPU6500_H

#include <stdint.h>
#include <type_traits> //for std::underlying_type

#include "system.h"

#include "spi.h"
#include "gpio.h"

namespace airdamon { namespace sensors {

  class MPU6500
  {
  private:

    // define the registers
    enum class RegAddr : uint8_t
    {
      SELF_TEST_X_GYRO = 0x00,
      SELF_TEST_Y_GYRO = 0x01,
      SELF_TEST_Z_GYRO = 0x02,
      SELF_TEST_X_ACCEL = 0x0D,
      SELF_TEST_Y_ACCEL = 0x0E,
      SELF_TEST_Z_ACCEL = 0x0F,
      XG_OFFSET_H = 0x13,
      XG_OFFSET_L = 0x14,
      YG_OFFSET_H = 0x15,
      YG_OFFSET_L = 0x16,
      ZG_OFFSET_H = 0x17,
      ZG_OFFSET_L = 0x18,
      SMPLRT_DIV = 0x19,
      CONFIG = 0x1A,
      GYRO_CONFIG = 0x1B,
      ACCEL_CONFIG = 0x1C,
      ACCEL_CONFIG2 = 0x1D,
      LP_ACCEL_ODR = 0x1E,
      WOM_THR = 0x1F,
      FIFO_EN = 0x23,
      // I2C registers (0x24 to 0x36) omitted for brevity
      INT_PIN_CFG = 0x37,
      INT_ENABLE = 0x38,
      INT_STATUS = 0x3A,
      ACCEL_XOUT_H = 0x3B,
      ACCEL_XOUT_L = 0x3C,
      ACCEL_YOUT_H = 0x3D,
      ACCEL_YOUT_L = 0x3E,
      ACCEL_ZOUT_H = 0x3F,
      ACCEL_ZOUT_L = 0x40,
      TEMP_OUT_H = 0x41,
      TEMP_OUT_L = 0x42,
      GYRO_XOUT_H = 0x43,
      GYRO_XOUT_L = 0x44,
      GYRO_YOUT_H = 0x45,
      GYRO_YOUT_L = 0x46,
      GYRO_ZOUT_H = 0x47,
      GYRO_ZOUT_L = 0x48,
      // External sensor data registers (0x49 to 0x60) omitted for brevity
      // Auxiliary I2C regs (0x61 to 0x67) omitted for brevity
      SIGNAL_PATH_RESET = 0x68,
      ACCEL_INTEL_CTRL = 0x69,
      USER_CTRL = 0x6A,
      PWR_MGMT_1 = 0x6B,
      PWR_MGMT_2 = 0x6C,
      FIFO_COUNT_H = 0x72,
      FIFO_COUNT_L = 0x73,
      FIFO_R_W = 0x74,
      WHO_AM_I = 0x75,
      XA_OFFSET_H = 0x77,
      XA_OFFSET_L = 0x78,
      YA_OFFSET_H = 0x7A,
      YA_OFFSET_L = 0x7B,
      ZA_OFFSET_H = 0x7D,
      ZA_OFFSET_L = 0x7E,
      // addr read/write config
      READ_CMD = 0x80,
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
    // register bits
    enum class PWR_MGMT_1: uint8_t { DEV_RST = 0x80, SLEEP = 0x40, CLKSEL_AUTO = 0x01 };
    enum class USER_CTRL: uint8_t { I2C_IF_DIS = 0x10, DMP_RST = 0x08, FIFO_RST = 0x04, I2C_MST_RST = 0x02, SIG_COND_RST = 0x01 };

    // Initialize the MPU device control and provide the SPI and chip select GPIO pin to use
    void init(SPI* spi, GPIO* cs);

    // Use async (DMA w/ ISR) SPI routine to read IMU data,
    // with the associated timestamp that IMU made the data available
    void read(float* accel, float* gyro, float* temp, uint64_t* time_us);

    // Use blocking (non-DMA and non-ISR) SPI routine to read IMU data
    void read_blocking(float* accel, float* gyro, float* temp);

    // Is there new data to asynchronously read?
    bool has_new_data() const { return new_data_; }

    // Let the IMU object know when MPU6500 triggered the EXTI
    void handle_exti_isr();

  private:
    SPI* spi_;
    GPIO* cs_;
    GPIO exti_;

    // the latest data from the MPU for async reads
    float accel_[3], gyro_[3], temp_;
    uint8_t buff_tx_[15] = { 0 };
    uint8_t buff_rx_[15] = { 0 };
    uint64_t imu_timestamp_us_ = 0;
    bool new_data_ = false;

    void init_EXTI();
    void write(RegAddr addr, uint8_t data);
    void chip_select(bool enable);
    void decode(uint8_t* buffer, float* accel, float* gyro, float* temp);
    void data_rx_cb();
  };

}}

#endif // MPU6500_H