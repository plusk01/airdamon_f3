
#ifndef BEECORE_H
#define BEECORE_H

#include "system.h"

#include "led.h"
#include "uart.h"
#include "vcp.h"
#include "spi.h"
#include "pwm.h"

#include "mpu6500.h"
#include "rc_sbus.h"

///////////////////////////////////////////////////////////////////////////////
//                            UART Configuration                             //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_UARTS = 1;
constexpr int CFG_UART1 = 0;
const airdamon::UARTConfig uart_config[NUM_UARTS] = {
  // USARTx, GPIOx, rx_pin, tx_pin, rx_pin_source, tx_pin_source, GPIO_AF, USARTx_IRQn, Tx_DMA_IRQn, Rx_DMA_Channel, Tx_DMA_Channel
  // UART1 is exposed for debuging
  {USART1, GPIOA, GPIO_Pin_10, GPIO_Pin_9, GPIO_PinSource10, GPIO_PinSource9, GPIO_AF_7, USART1_IRQn, DMA1_Channel4_IRQn, DMA1_Channel5, DMA1_Channel4},
};

///////////////////////////////////////////////////////////////////////////////
//                            PWM Configuration                              //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_PWMS = 4;
const airdamon::PWMConfig pwm_config[NUM_PWMS] = {
  // TIMx, channel, GPIOx, pin, pin_source, GPIO_AF, TIMx_IRQn
  {TIM2, TIM_Channel_1, GPIOA, GPIO_Pin_0, GPIO_PinSource0, GPIO_AF_1, TIM2_IRQn},
  {TIM2, TIM_Channel_2, GPIOA, GPIO_Pin_1, GPIO_PinSource1, GPIO_AF_1, TIM2_IRQn},
  {TIM2, TIM_Channel_3, GPIOA, GPIO_Pin_2, GPIO_PinSource2, GPIO_AF_1, TIM2_IRQn},
  {TIM2, TIM_Channel_4, GPIOA, GPIO_Pin_3, GPIO_PinSource3, GPIO_AF_1, TIM2_IRQn},
};

#endif // BEECORE_H
