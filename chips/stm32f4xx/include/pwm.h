#pragma once

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

    void write(float value);
    void write_us(uint16_t value);

    void enable();
    void disable();

  private:
    // capture/compare register for the relevant channel of the timer
    volatile uint32_t* CCRx_;
    float cntr_steps_per_us_;
    uint32_t min_us_, max_us_;

    // GPIO object for the PWM pin
    GPIO pin_;

    // low-level hw configuration for this PWM object
    const PWMConfig* cfg_;

    // low-level peripheral initialization
    void init_TIM(uint16_t frequency);

    uint32_t get_timer_clock_freq();
  };

}
