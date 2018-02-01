#ifndef UART_H
#define UART_H

// from serial.h
#define RX_BUFFER_SIZE 64
#define TX_BUFFER_SIZE 64
#include <functional>

#include "system.h"

//#include "serial.h"
#include "gpio.h"
#include "led.h"

class UART
{
public:
  UART(USART_TypeDef* _uart);
  void write(uint8_t*ch, uint8_t len);
  uint32_t rx_bytes_waiting();
  uint32_t tx_bytes_free();
  uint8_t read_byte();
  bool set_baud_rate(uint32_t baud);
  bool tx_buffer_empty();
  bool set_mode(uint8_t mode_);
  void put_byte(uint8_t ch);
  bool flush();
  void begin_write();
  void end_write();
  void register_rx_callback(std::function<void(uint8_t)> cb);
  void unregister_rx_callback();

  void toggleLED();

private:
  void init_UART(uint32_t baudrate_);
  void init_DMA();
  void init_NVIC();
  void startDMA();

  LED led_;

  uint32_t baudrate_;
  uint8_t rx_buffer_[RX_BUFFER_SIZE];
  uint8_t tx_buffer_[RX_BUFFER_SIZE];
  uint16_t rx_buffer_head_;
  uint16_t rx_buffer_tail_;
  uint16_t tx_buffer_head_;
  uint16_t tx_buffer_tail_;
  uint16_t rx_DMA_read_index_;
  uint32_t rx_DMA_pos_;
  GPIO rx_gpio_;
  GPIO tx_gpio_;
  DMA_Channel_TypeDef* Tx_DMA_Stream_;
  DMA_Channel_TypeDef* Rx_DMA_Stream_;
  uint32_t DMA_Channel_;
  USART_TypeDef* dev_;
  bool DMA_Tx_;
  bool DMA_Rx_;
  IRQn_Type TxDMAIRQ_;
  IRQn_Type RxDMAIRQ_;
  IRQn_Type UARTIRQ_;
  uint32_t DMA_TX_IT_BIT_;
  uint32_t DMA_RX_IT_BIT_;

  // from serial.h
  GPIO rx_pin_;
  GPIO tx_pin_;
  std::function<void(uint8_t)> receive_CB_;


};

#endif // UART_H
