#include "led.h"

void LED::init(GPIO_TypeDef* gpio_port, uint16_t pin)
{
  GPIO::init(gpio_port, pin, GPIO::OUTPUT);
}

void LED::on()
{
  write(GPIO::HIGH);
}

void LED::off()
{
  write(GPIO::LOW);
}


