#pragma once

#include "system.h"

#include "led.h"
#include "uart.h"
#include "vcp.h"
#include "spi.h"
#include "i2c.h"
#include "pwm.h"
// #include "flash.h"

#include "ms5611.h"
// #include "mpu6500.h"
// #include "rc_sbus.h"

///
/// This file defines the hardware configuration used in the C++ abstraction layer
/// All chip-specific configuration should happen here
///

///////////////////////////////////////////////////////////////////////////////
//                            UART Configuration                             //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_UARTS = 2;
constexpr int CFG_UART1 = 0;
constexpr int CFG_UART3 = 1;
const airdamon::UARTConfig uart_config[NUM_UARTS] = {
  // USARTx, GPIOx, rx_pin, tx_pin, rx_pin_source, tx_pin_source, GPIO_AF, USARTx_IRQn, Tx_DMA_IRQn, Rx_DMA_Stream, Tx_DMA_Stream, DMA_Channel
  // UART1 is CONN4/MainPort
  {USART1, GPIOA, GPIO_Pin_10, GPIO_Pin_9, GPIO_PinSource10, GPIO_PinSource9, GPIO_AF_USART1, USART1_IRQn, DMA2_Stream7_IRQn, DMA2_Stream5, DMA2_Stream7, DMA_Channel_4},
  // USART3 is CONN1/FlexiPort
  {USART3, GPIOB, GPIO_Pin_11, GPIO_Pin_10, GPIO_PinSource11, GPIO_PinSource10, GPIO_AF_USART3, USART3_IRQn, DMA1_Stream3_IRQn, DMA1_Stream1, DMA1_Stream3, DMA_Channel_4},
};

///////////////////////////////////////////////////////////////////////////////
//                            SPI Configuration                              //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_SPIS = 1;
constexpr int CFG_SPI1 = 0;
// const airdamon::SPIConfig spi_config[NUM_SPIS] = {
//   // SPIx, GPIOx, sck_pin, miso_pin, mosi_pin, sck_pin_source, miso_pin_source, mosi_pin_source, GPIO_AF
//   {SPI1, GPIOB, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_PinSource3, GPIO_PinSource4, GPIO_PinSource5, GPIO_AF_5, DMA1_Channel2_IRQn, DMA1_Channel2, DMA1_Channel3},
// };

///////////////////////////////////////////////////////////////////////////////
//                            I2C Configuration                              //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_I2CS = 2;
constexpr int CFG_I2C1 = 0;
constexpr int CFG_I2C2 = 1;
const airdamon::I2CConfig i2c_config[NUM_I2CS] = {
  // I2Cx, GPIOx, scl_pin, sda_pin, GPIO_AF, scl_pin_source, sda_pin_source, Rx_DMA_IRQn, I2Cx_EV_IRQn, I2Cx_ER_IRQn, Rx_DMA_Stream, DMA_Channel
  // I2C1 connects to onboard mag and pressure sensor
  {I2C1, GPIOB, GPIO_Pin_8, GPIO_Pin_9, GPIO_AF_I2C1, GPIO_PinSource8, GPIO_PinSource9, DMA1_Stream0_IRQn, I2C1_EV_IRQn, I2C1_ER_IRQn, DMA1_Stream0, DMA_Channel_1},
  // I2C2 is CONN1/FlexiPort
  {I2C2, GPIOB, GPIO_Pin_10, GPIO_Pin_11, GPIO_AF_I2C2, GPIO_PinSource10, GPIO_PinSource11, DMA1_Stream2_IRQn, I2C2_EV_IRQn, I2C2_ER_IRQn, DMA1_Stream2, DMA_Channel_7},
};

///////////////////////////////////////////////////////////////////////////////
//                            PWM Configuration                              //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_PWMS = 13;
const airdamon::PWMConfig pwm_config[NUM_PWMS] = {
  // TIMx, channel, GPIOx, pin, pin_source, GPIO_AF, TIMx_IRQn
  // Servo 1 (JP1)
  {TIM3, TIM_Channel_3, GPIOB, GPIO_Pin_0, GPIO_PinSource0, GPIO_AF_TIM3, TIM3_IRQn},
  // Servo 2 (JP2)
  {TIM3, TIM_Channel_4, GPIOB, GPIO_Pin_1, GPIO_PinSource1, GPIO_AF_TIM3, TIM3_IRQn},
  // Servo 3 (JP3)
  {TIM9, TIM_Channel_2, GPIOA, GPIO_Pin_3, GPIO_PinSource3, GPIO_AF_TIM9, TIM1_BRK_TIM9_IRQn},
  // Servo 4 (JP4)
  {TIM2, TIM_Channel_3, GPIOA, GPIO_Pin_2, GPIO_PinSource2, GPIO_AF_TIM2, TIM2_IRQn},
  // Servo 5 (JP5)
  {TIM5, TIM_Channel_2, GPIOA, GPIO_Pin_1, GPIO_PinSource1, GPIO_AF_TIM5, TIM5_IRQn},
  // Servo 6 (JP6)
  {TIM5, TIM_Channel_1, GPIOA, GPIO_Pin_0, GPIO_PinSource0, GPIO_AF_TIM5, TIM5_IRQn},

  // RC6 (Flexi-10)
  {TIM8, TIM_Channel_4, GPIOC, GPIO_Pin_9, GPIO_PinSource9, GPIO_AF_TIM8, TIM8_CC_IRQn},
  // RC5 (Flexi-9)
  {TIM8, TIM_Channel_3, GPIOC, GPIO_Pin_8, GPIO_PinSource8, GPIO_AF_TIM8, TIM8_CC_IRQn},
  // RC4 (Flexi-8)
  {TIM8, TIM_Channel_2, GPIOC, GPIO_Pin_7, GPIO_PinSource7, GPIO_AF_TIM8, TIM8_CC_IRQn},
  // RC3 (Flexi-7)
  {TIM8, TIM_Channel_1, GPIOC, GPIO_Pin_6, GPIO_PinSource6, GPIO_AF_TIM8, TIM8_CC_IRQn},
  // RC2 (Flexi-6)
  {TIM12, TIM_Channel_2, GPIOB, GPIO_Pin_15, GPIO_PinSource15, GPIO_AF_TIM12, TIM8_BRK_TIM12_IRQn},
  // RC2 (Flexi-6)
  {TIM12, TIM_Channel_1, GPIOB, GPIO_Pin_14, GPIO_PinSource14, GPIO_AF_TIM12, TIM8_BRK_TIM12_IRQn},

  // Buzzer?
  {TIM1, TIM_Channel_1, GPIOA, GPIO_Pin_8, GPIO_PinSource8, GPIO_AF_TIM1, TIM1_CC_IRQn},

};
