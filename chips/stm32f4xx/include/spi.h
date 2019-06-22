#ifndef SPI_H
#define SPI_H

#pragma GCC push_options
#pragma GCC optimize ("O0")
#include <functional>
#pragma GCC pop_options

#include "system.h"
#include "gpio.h"

namespace airdamon {

  struct SPIConfig {
    // The relevant SPI peripheral
    SPI_TypeDef* SPIx;
    
    // GPIO port and pins for SPI
    GPIO_TypeDef* GPIOx;
    uint16_t sck_pin, miso_pin, mosi_pin;

    // SPI alternate function (AF) and pins
    uint8_t sck_pin_source, miso_pin_source, mosi_pin_source, GPIO_AF;

    // SPI Rx Channel DMA IRQ number
    IRQn_Type Rx_DMA_IRQn;

    // DMA Channels regs for SPI Rx and Tx
    DMA_Stream_TypeDef* Rx_DMA_Channel;
    DMA_Stream_TypeDef* Tx_DMA_Channel;
  };

  class SPI
  {
  public:
    void init(const SPIConfig* config);

    // set the speed of the SPI peripheral
    void set_divisor(uint16_t new_divisor);

    // transfer a single byte (blocking call)
    uint8_t transfer_byte(uint8_t data);

    // transfer an array of bytes using DMA (asynchronous call). Can be made synchronous
    // by waiting for SPi to not be busy: while (spi->is_busy());
    void transfer(uint8_t *tx_data, uint32_t num_bytes, volatile uint8_t *rx_data, std::function<void(void)> cb);

    // is there a DMA (asynchronous) transfer happening?
    bool is_busy() const { return busy_; }

    // public so that ISR can call
    void transfer_complete_isr();

  private:
    SPI_TypeDef* SPIx_;
    GPIO sck_;
    GPIO mosi_;
    GPIO miso_;

    DMA_InitTypeDef DMA_InitStructure_;

    uint32_t errors_ = 0;
    volatile bool busy_ = false;

    // callback once DMA transfer is complete
    std::function<void(void)> cb_;

    // low-level hw configuration for this SPI object
    const SPIConfig* cfg_;

    void init_DMA();
    void init_SPI();
    void init_NVIC();
  };

}

#endif // SPI_H