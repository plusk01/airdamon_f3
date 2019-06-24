#include "pwm.h"

namespace airdamon {

void PWM::init(const PWMConfig* config, uint16_t frequency, uint32_t min_us, uint32_t max_us, GPIO::gpio_write_t state)
{
  cfg_ = config;

  // initialize the GPIO for the output pwm pin
  pin_.init(config->GPIOx, config->pin, GPIO::PERIPH_OUT);
  pin_.write(state);

  // Set GPIO pins as alternate function
  GPIO_PinAFConfig(config->GPIOx, config->pin_source, config->GPIO_AF);

  // save min and max microseconds of PWM
  min_us_ = min_us;
  max_us_ = max_us;

  init_TIM(frequency);
}

// ----------------------------------------------------------------------------

void PWM::write(float value)
{
  // pwm usecs
  uint16_t us = min_us_ + static_cast<uint16_t>((max_us_ - min_us_) * value);
  write_us(us);
}

// ----------------------------------------------------------------------------

void PWM::write_us(uint16_t value)
{
  *CCRx_ = static_cast<uint16_t>(value * cntr_steps_per_us_);
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void PWM::init_TIM(uint16_t frequency)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef       TIM_OCInitStructure;
  
  /**
   * Choosing the period and prescaler for a desired PWM frequency can be tricky.
   * The formula for this is
   * 
   *      (PSC+1)*(ARR+1) = TIMClk / PWMFreq
   *      
   * where PSC is the 16-bit num that goes into TIM_Prescaler and ARR is the
   * 16-bit num that goes into TIM_Period (the auto-reload register). Note that
   * the right-hand side has the knowns, while the left hand side has two unknowns.
   * Thus, we need another constraint to find PSC and ARR.
   * 
   * A good strategy is to choose the smallest value for the prescaler, giving
   * more precision in the number of cycles in a PWM period. We do this by
   * first assuming the period to be 0xFFFF (the largest ARR+1 could ever be) and
   * solving for PSC+1. Then, we go back and solve for ARR+1 using that value.
   * 
   * c.f., https://electronics.stackexchange.com/a/179338/79646
   * c.f., https://stackoverflow.com/a/51911524/2392520
   */
  uint32_t period_cycles = static_cast<uint32_t>(get_timer_clock_freq() / frequency);
  uint16_t prescaler = static_cast<uint16_t>(period_cycles / 0xFFFF + 1);
  uint16_t period = static_cast<uint16_t>(period_cycles / prescaler + 1);

  // n.b. these names are not super expressive. Here, `period` is really the number
  // of cycles in until an event occurs, so `cycles` is conceivably a better name.
  cntr_steps_per_us_ = period / (1.0 / frequency) * 1E-6;

  /**
   * n.b. looking at the logic analyzer, there is a bit of slop in these numbers,
   * which is to be expected. I suppose this is why ESC calibration is important!
   */

  // init timer
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period        = period - 1;
  TIM_TimeBaseStructure.TIM_Prescaler     = prescaler - 1;
  // TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  // TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
  TIM_TimeBaseInit(cfg_->TIMx, &TIM_TimeBaseStructure);

  // init output compare
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode        = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState   = TIM_OutputState_Enable;
  // TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Disable;
  // TIM_OCInitStructure.TIM_Pulse         = 0;
  // TIM_OCInitStructure.TIM_OCPolarity    = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState   = TIM_OCIdleState_Set; // ?

  switch (cfg_->channel)
  {
  case TIM_Channel_1:
  default:
    TIM_OC1Init(cfg_->TIMx, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(cfg_->TIMx, TIM_OCPreload_Enable);
    CCRx_ = &cfg_->TIMx->CCR1;
    break;
  case TIM_Channel_2:
    TIM_OC2Init(cfg_->TIMx, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(cfg_->TIMx, TIM_OCPreload_Enable);
    CCRx_ = &cfg_->TIMx->CCR2;
    break;
  case TIM_Channel_3:
    TIM_OC3Init(cfg_->TIMx, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(cfg_->TIMx, TIM_OCPreload_Enable);
    CCRx_ = &cfg_->TIMx->CCR3;
    break;
  case TIM_Channel_4:
    TIM_OC4Init(cfg_->TIMx, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(cfg_->TIMx, TIM_OCPreload_Enable);
    CCRx_ = &cfg_->TIMx->CCR4;
    break;
  }

  // TIM_CtrlPWMOutputs(cfg_->TIMx, ENABLE);

  TIM_ARRPreloadConfig(cfg_->TIMx, ENABLE);
  TIM_Cmd(cfg_->TIMx, ENABLE);
}

// ----------------------------------------------------------------------------

uint32_t PWM::get_timer_clock_freq()
{
  // Retrieve the clock configuration. Originally set in `system_stm32fxxx.c`
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  // From the Clock Tree (see refman), we can see that interal clock used for
  // timers is the peripheral clock (PCLK). In particular, there is a separate
  // consideration based on the APBx prescaler. We can easily ascertain if there
  // is a prescaler in use by comparing the peripheral clock with the SYSCLK.
  // (c.f., http://www.micromouseonline.com/2016/02/03/tim3-arr-regular-interrupts-stm32f4/)
  uint8_t mult = (RCC_Clocks.PCLK1_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2;

  // Further, some timers are on APB1 (using PCLK1) and some are on APB2 (PCLK2).
  uint32_t freq = RCC_Clocks.PCLK1_Frequency;
  if (cfg_->TIMx == TIM1  ||
      cfg_->TIMx == TIM8  ||
      cfg_->TIMx == TIM9  ||
      cfg_->TIMx == TIM10 ||
      cfg_->TIMx == TIM11) {
    freq = RCC_Clocks.PCLK2_Frequency;
  }

  return mult * freq;
}

}