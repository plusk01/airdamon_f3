#pragma once

#include <functional>
#include <type_traits>

#include "system.h"
#include "gpio.h"

namespace airdamon {

  struct I2CConfig {
    // The relevant I2C peripheral
    I2C_TypeDef* I2Cx;

    // GPIO port and pins for I2C
    GPIO_TypeDef* GPIOx;
    uint16_t scl_pin, sda_pin;

    // SPI alternate function (AF) and pins
    uint8_t GPIO_AF;
    uint8_t scl_pin_source, sda_pin_source;

    // I2C Rx Channel DMA IRQ number
    IRQn_Type Rx_DMA_IRQn;

    // I2C Event and Error IRQ numbers
    IRQn_Type I2Cx_EV_IRQn, I2Cx_ER_IRQn;

    // DMA Channels regs for SPI Rx and Tx
    DMA_Stream_TypeDef* Rx_DMA_Stream;
    // DMA_Stream_TypeDef* Tx_DMA_Stream;

    // peripheral request channel. Table 42/43 of refman
    uint32_t DMA_Channel;
  };



  class I2C
  {
  public:
    enum class ClockSpeed: uint32_t { FastMode400kHz = 400000, StandardMode100kHz = 100000 };

    void init(const I2CConfig * config, const ClockSpeed clock_speed = ClockSpeed::StandardMode100kHz);

    void unstick();
    void hardware_failure();
    int8_t read(uint8_t addr, uint8_t reg, uint8_t num_bytes, uint8_t* data, std::function<void(void)> callback, bool blocking = false);
    int8_t write(uint8_t addr, uint8_t reg, uint8_t data, std::function<void(void)> callback);

    bool write(uint8_t addr, uint8_t data);
    bool write(uint8_t addr, const uint8_t * data, uint8_t len);
    uint8_t read(uint8_t addr, uint8_t * data, uint8_t len);

    inline uint16_t num_errors() { return error_count_; }

    //interrupt handlers
    bool handle_error();
    bool handle_event();
    void transfer_complete_cb();


  private:
    // Initializers for low-level perhipherals and components
    void init_I2C(const ClockSpeed clock_speed);
    void init_DMA();
    void init_NVIC();

    std::function<void(void)> cb_;

    void handle_hardware_failure();

    // enum : int8_t
    // {
    //   ERROR = 0,
    //   SUCCESS = 1,
    //   BUSY = -1
    // };

    typedef enum
    {
      IDLE,
      READING,
      WRITING
    } current_status_t;

    // I2C GPIO pins
    GPIO scl_pin_;
    GPIO sda_pin_;

    // low-level hw configuration for this UART object
    const I2CConfig* cfg_;

    uint16_t error_count_ = 0;

    //Variables for current job:
    current_status_t current_status_;
    bool subaddress_sent_ = false;
    bool done_ = false;

    volatile uint8_t  addr_;
    volatile uint8_t  reg_;
    volatile uint8_t  len_;
    volatile uint8_t data_;

    DMA_InitTypeDef DMA_InitStructure_;

    // const i2c_hardware_struct_t *c_;

    // This constexpr (compile-time) function extracts the underlying
    // value of an enumeration based on its underlying type
    template<typename E>
    static constexpr auto uv(E e) -> typename std::underlying_type<E>::type 
    {
       return static_cast<typename std::underlying_type<E>::type>(e);
    }
  };

} // ns airdamon