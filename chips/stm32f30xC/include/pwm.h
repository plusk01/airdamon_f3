#ifndef PWM_H
#define PWM_H

#include "system.h"
#include "gpio.h"

namespace airdamon {


  struct PWMConfig {
    // The relevant timer peripheral
    TIM_TypeDef* TIMx;

    // TIM channel
    uint8_t channel;
    
    // GPIO port and pin for PWM
    GPIO_TypeDef* GPIOx;
    uint16_t pin;

    // TIM alternate function (AF) and pins
    uint8_t pin_source, GPIO_AF;

    // TIM global interrupt IRQ number
    IRQn_Type TIMx_IRQn;
  };



  class PWM
  {
  public:
    void init(const PWMConfig* config, uint16_t frequency, uint32_t min_us, uint32_t max_us);

    void enable();
    void disable();

    void write(float value);
    void write_us(uint16_t value);

  private:
    // capture/compare register for the relevant channel of the timer
    volatile uint32_t* CCRx_;
    uint16_t min_cyc_, max_cyc_;
    uint32_t cycles_per_us_;

    // GPIO object for the PWM pin
    GPIO pin_;

    // low-level hw configuration for this PWM object
    const PWMConfig* cfg_;

    // low-level peripheral initialization
    void init_TIM(uint16_t frequency, uint32_t min_us, uint32_t max_us);
  };

}

#endif // PWM_H