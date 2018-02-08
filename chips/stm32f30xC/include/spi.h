#ifndef SPI_H
#define SPI_H

#include "system.h"
#include "gpio.h"

extern "C" {
#include "printf.h"
}

class SPI
{
public:

  void init(SPI_TypeDef* SPIx);
  void set_divisor(uint16_t new_divisor);

  bool transfer(uint8_t *out_data, uint32_t num_bytes, uint8_t* in_data, void (*cb)(void) = NULL);
  uint8_t transfer_byte(uint8_t data);

  void transfer_complete_cb();
  inline bool is_busy() {return busy_;}

private:
  SPI_TypeDef* SPIx_;
  GPIO sck_;
  GPIO mosi_;
  GPIO miso_;

  DMA_InitTypeDef DMA_InitStructure_;

  uint32_t errors_ = 0;
  bool busy_ = false;
  void (*transfer_cb_)(void) = NULL;
};

#endif // SPI_H