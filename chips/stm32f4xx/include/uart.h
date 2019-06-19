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

namespace airdamon {


  struct UARTConfig {
    // The relevant USART peripheral
    USART_TypeDef* USARTx;
    
    // USART GPIO port and pins for RX and TX
    GPIO_TypeDef* GPIOx;
    uint16_t rx_pin, tx_pin;

    // USART alternate function (AF) and pins
    uint8_t rx_pin_source, tx_pin_source, GPIO_AF;

    // USART global interrupt IRQ number
    IRQn_Type USARTx_IRQn;

    // USART Tx Channel DMA IRQ number
    IRQn_Type Tx_DMA_IRQn;

    // DMA Stream regs for USART Rx and Tx
    DMA_Stream_TypeDef* Rx_DMA_Stream;
    DMA_Stream_TypeDef* Tx_DMA_Stream;

    // peripheral request channel. Table 42/43 of refman
    uint32_t DMA_Channel;
  };



  class UART
  {
  public:
    enum class Mode: uint8_t { m8N1, m8E2 };

    void init(const UARTConfig* config, uint32_t baudrate=115200, Mode mode=Mode::m8N1, bool inverted=false);

    // Use this object for printf
    void connect_to_printf();

    // Allow calling code to be sent bytes as they are received
    void register_rx_callback(std::function<void(uint8_t)> cb);
    void unregister_rx_callback();

    // Allow changing baudrate and UART mode
    void set_mode(uint32_t baudrate, Mode mode=Mode::m8N1, bool inverted=false);

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
    void init_UART(uint32_t baudrate, Mode mode, bool inverted);
    void init_DMA();
    void init_NVIC();

    // Buffers to hold data managed by DMA, from/to Rx/Tx
    static constexpr int RX_BUFFER_SIZE = 64;
    static constexpr int TX_BUFFER_SIZE = 64;
    uint8_t rx_buffer_[RX_BUFFER_SIZE];
    uint8_t tx_buffer_[TX_BUFFER_SIZE];
    uint16_t rx_buffer_head_, rx_buffer_tail_;
    uint16_t tx_buffer_head_, tx_buffer_tail_;
    uint16_t tx_old_DMA_pos_;

    // USART GPIO pins
    GPIO rx_pin_;
    GPIO tx_pin_;

    // low-level hw configuration for this UART object
    const UARTConfig* cfg_;

    // Allow calling code to be sent the byte when it is received
    std::function<void(uint8_t)> cb_rx_ = nullptr;

  };

}

#endif // UART_H