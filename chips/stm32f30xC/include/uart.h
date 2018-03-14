/**
 * C++ object that allows use of a USART with DMA capabilities.
 * USART with polling or interrupts not supported by the class,
 * though could easily be added.
 *
 * Except for the `printf` function, there is no logic preventing
 * calling code to overwrite data in the Tx buffer. It is up to
 * the caller to check the size of the data to write with the
 * TX_BUFFER_SIZE and then split the data up accordingly.
 *
 **/

#ifndef UART_H
#define UART_H

#include <functional>

#include "system.h"
#include "gpio.h"

extern "C" {
#include "printf.h"
}

#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64

class UART
{
public:
  enum class Mode: uint8_t { m8N1, m8E2 };

  void init(USART_TypeDef* uart, uint32_t baudrate=115200, Mode mode=Mode::m8N1);

  // Use this object for printf
  void connect_to_printf();

  // Allow calling code to be sent bytes as they are received
  void register_rx_callback(std::function<void(uint8_t)> cb);
  void unregister_rx_callback();

  // Allow changing baudrate and UART mode
  void set_mode(uint32_t baudrate, Mode mode);

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

  // send a byte array through to the USARTx
  void write(uint8_t* c, uint8_t len);

  // Is there any data in the buffer that needs to be sent?
  bool tx_buffer_empty();

  // Would adding another byte of data stomp on data that
  // the DMA could possibly still need for a transfer?
  bool would_stomp_dma_data();

  //
  // IRQ functions
  //

  // Start a DMA transfer from buffer to Tx reg of USARTx.
  // The only reason this is public is for access from the IRQ handler.
  void start_DMA_transfer();

  // The USART peripheral recieved a byte (IT_RXNE). Your move.
  void handle_usart_irq();

private:
  // Initializers for low-level perhipherals and components
  void init_UART(uint32_t baudrate, Mode mode);
  void init_DMA();
  void init_NVIC();

  // Buffers to hold data managed by DMA, from/to Rx/Tx
  uint8_t rx_buffer_[RX_BUFFER_SIZE];
  uint8_t tx_buffer_[TX_BUFFER_SIZE];
  uint16_t rx_buffer_head_, rx_buffer_tail_;
  uint16_t tx_buffer_head_, tx_buffer_tail_;
  uint16_t tx_old_DMA_pos_;

  // USART GPIO pins
  GPIO rx_pin_;
  GPIO tx_pin_;

  // DMA Channels regs for USART Rx and Tx
  DMA_Channel_TypeDef* Tx_DMA_Channel_;
  DMA_Channel_TypeDef* Rx_DMA_Channel_;

  // USART Tx Channel DMA IRQ number
  IRQn_Type Tx_DMA_IRQn_;

  // USART global interrupt IRQ number
  IRQn_Type USARTx_IRQn_;

  // The specified USART perhipheral
  USART_TypeDef* USARTx_;

  // Allow calling code to be sent the byte when it is received
  std::function<void(uint8_t)> cb_rx_ = nullptr;

};

#endif // UART_H
