#ifndef DISCOVERYF3_H
#define DISCOVERYF3_H

#include "system.h"

#include "led.h"
#include "uart.h"
#include "vcp.h"
#include "spi.h"
#include "i2c.h"
#include "l3gd20.h"

///////////////////////////////////////////////////////////////////////////////
//                            UART Configuration                             //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_UARTS = 1;
constexpr int CFG_UART1 = 0;
const airdamon::UARTConfig uart_config[NUM_UARTS] = {
  // USARTx, GPIOx, rx_pin, tx_pin, rx_pin_source, tx_pin_source, GPIO_AF, USARTx_IRQn, Tx_DMA_IRQn, Rx_DMA_Channel, Tx_DMA_Channel
  {USART1, GPIOA, GPIO_Pin_10, GPIO_Pin_9, GPIO_PinSource10, GPIO_PinSource9, GPIO_AF_7, USART1_IRQn, DMA1_Channel4_IRQn, DMA1_Channel5, DMA1_Channel4}
};

///////////////////////////////////////////////////////////////////////////////
//                            SPI Configuration                              //
///////////////////////////////////////////////////////////////////////////////

constexpr int NUM_SPIS = 1;
const airdamon::SPIConfig spi_config[NUM_SPIS] = {
  // SPIx, GPIOx, sck_pin, miso_pin, mosi_pin, sck_pin_source, miso_pin_source, mosi_pin_source, GPIO_AF
  {SPI1, GPIOA, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, GPIO_PinSource5, GPIO_PinSource6, GPIO_PinSource7, GPIO_AF_5},
};

#endif // DISCOVERYF3_H