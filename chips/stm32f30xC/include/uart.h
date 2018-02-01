/**
 * C++ object that allows use of a USART with DMA capabilities.
 * USART with polling or interrupts not supported by the class,
 * though could easily be added.
 *
 **/

#ifndef UART_H
#define UART_H

#include "system.h"
#include "gpio.h"
#include "led.h"

extern "C" {
#include "printf.h"
}

#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64

class UART
{
public:
  UART(USART_TypeDef* uart, uint32_t baudrate=115200);

  //
  // Rx functions
  //

  // Get a single byte from buffer. Must call
  //  `rx_bytes_waiting()` beforehand.
  uint8_t read_byte();

  // Check to see if there are bytes to be read.
  uint32_t rx_bytes_waiting();


  //
  // Tx functions
  //

  // send a single byte through USARTx
  void write_byte(uint8_t* c, uint8_t len);

  // Is there any data in the buffer that needs to be sent?
  bool tx_buffer_empty();


  // Start a DMA transfer from buffer to Tx reg of USARTx.
  // The only reason this is public is for access from the IRQ handler.
  void start_DMA_transfer();

private:
  // Initializers for low-level perhipherals and components
  void init_UART(uint32_t baudrate);
  void init_DMA();
  void init_NVIC();

  // Buffers to hold data managed by DMA, from/to Rx/Tx
  uint8_t rx_buffer_[RX_BUFFER_SIZE];
  uint8_t tx_buffer_[RX_BUFFER_SIZE];
  uint16_t rx_buffer_head_, rx_buffer_tail_;
  uint16_t tx_buffer_head_, tx_buffer_tail_;

  // USART GPIO pins
  GPIO rx_pin_;
  GPIO tx_pin_;

  // DMA Channels regs for USART Rx and Tx
  DMA_Channel_TypeDef* Tx_DMA_Channel_;
  DMA_Channel_TypeDef* Rx_DMA_Channel_;

  // USART Tx Channel DMA IRQ number
  IRQn_Type Tx_DMA_IRQn_;

  // The specified USART perhipheral
  USART_TypeDef* USARTx_;

};

#endif // UART_H
