/**
 * STM32F4xx I2C driver for OpenPilot REVO
 *
 * Adapted from https://github.com/jihlein/AQ32Plus/blob/master/src/drv/drv_i2c.h
 *          and https://github.com/superjax/airbourne/blob/master/include/i2c.h
 * @author len0rd
 * @since 2017-08-23
 */
#pragma once

#include <functional>

#include "system.h"
#include "gpio.h"

extern "C" {
#include "printf.h"
}

#define I2C_HIGHSPEED_TIMING  0x00500E30  // 1000 Khz, 72Mhz Clock, Analog Filter Delay ON, Setup 40, Hold 4.
#define I2C_STANDARD_TIMING   0x00E0257A  // 400 Khz, 72Mhz Clock, Analog Filter Delay ON, Rise 100, Fall 10.

class I2C
{
public:
  void init(I2C_TypeDef* I2Cx);

  std::function<void(void)> cb_;

  // void init(const i2c_hardware_struct_t *c);
  void unstick();
  void hardware_failure();
  int8_t read(uint8_t addr, uint8_t reg, uint8_t num_bytes, uint8_t* data, std::function<void(void)> callback, bool blocking = false);
  int8_t write(uint8_t addr, uint8_t reg, uint8_t data, std::function<void(void)> callback);

  bool write(uint8_t addr, uint8_t reg, uint8_t data);
  int8_t read(uint8_t addr, uint8_t reg, uint8_t *data);

  inline uint16_t num_errors() { return error_count_; }

  //interrupt handlers
  bool handle_error();
  bool handle_event();
  void transfer_complete_cb();


private:
  // Access to the registers for the I2C peripheral
  I2C_TypeDef* I2Cx_;

  void handle_hardware_failure();

  enum : int8_t
  {
    ERROR = 0,
    SUCCESS = 1,
    BUSY = -1
  };

  typedef enum
  {
    IDLE,
    READING,
    WRITING
  } current_status_t;

  // I2C GPIO pins
  GPIO scl_;
  GPIO sda_;

  uint16_t error_count_ = 0;

  //Variables for current job:
  current_status_t current_status_;
  bool subaddress_sent_ = false;
  bool done_ = false;

  volatile uint8_t  addr_;
  volatile uint8_t  reg_;
  volatile uint8_t  len_;
  volatile uint8_t data_;

  DMA_InitTypeDef  DMA_InitStructure_;

  // const i2c_hardware_struct_t *c_;

};