#pragma once

#include "gpio.h"

namespace airdamon {

  class LED : public GPIO
  {
  public:

    /**
     * @param gpio_port   GPIO port to use (ie GPIOA, GPIOB, etc)
     * @param pin         The pin within this port to use (ie: GPIO_Pin_5)
     * @param active_low  does GPIO::LOW turn the LED on?
     */
    void init(GPIO_TypeDef* gpio_port, uint16_t pin, bool active_low = false);

    void on();
    void off();

  private:
    bool active_low_ = false; ///< is the LED active low?
  };

} // ns airdamon
