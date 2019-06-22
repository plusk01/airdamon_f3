#include "led.h"

namespace airdamon {

void LED::init(GPIO_TypeDef* gpio_port, uint16_t pin, bool active_low)
{
  active_low_ = active_low;
  GPIO::init(gpio_port, pin, GPIO::OUTPUT);

  // initialize the LED off
  off();
}

// ----------------------------------------------------------------------------

void LED::on()
{
  write( ((active_low_) ? GPIO::LOW : GPIO::HIGH) );
}

// ----------------------------------------------------------------------------

void LED::off()
{
 write( ((active_low_) ? GPIO::HIGH : GPIO::LOW) );
}

} // ns airdamon
