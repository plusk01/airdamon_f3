#ifndef GPIO_H
#define GPIO_H

#include "system.h"
#include "stm32f4xx_gpio.h"

class GPIO
{
public:
  typedef enum
  {
    HIGH,
    LOW
  } gpio_write_t;

  typedef enum
  {
    INPUT,
    OUTPUT,
    PERIPH_OUT,
    PERIPH_IN,
    PERIPH_IN_OUT,
    ANALOG,
    EXTERNAL_INTERRUPT
  } gpio_mode_t;

  void init(GPIO_TypeDef* BasePort, uint16_t pin, gpio_mode_t mode);
  void write(gpio_write_t state);
  void toggle(void);
  void set_mode(gpio_mode_t mode);
  bool read();

private:
  uint16_t pin_;
  GPIO_TypeDef* port_;
  gpio_mode_t mode_;

};

#endif // GPIO_H
