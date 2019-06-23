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
  *CCRx_ = (value - min_us_) * period_ / (max_us_ - min_us_);
}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void PWM::init_TIM(uint16_t frequency)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef       TIM_OCInitStructure;

  // calculate timer values
  constexpr uint32_t PWM_MHZ = 42;
  const uint32_t prescaler = SystemCoreClock / (PWM_MHZ * 1000000);

  // how many cycles is one period at the specified PWM frequency?
  period_ = (PWM_MHZ * 1000000) / frequency;

  //init timer
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period        = (period_ - 1) & 0xFFFF;
  TIM_TimeBaseStructure.TIM_Prescaler     = prescaler - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
  TIM_TimeBaseInit(cfg_->TIMx, &TIM_TimeBaseStructure);

  //init output compare
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode        = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState   = TIM_OutputState_Enable;
  // TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Disable;
  // TIM_OCInitStructure.TIM_Pulse         = min_cyc_ - 1;
  TIM_OCInitStructure.TIM_OCPolarity    = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCIdleState   = TIM_OCIdleState_Set;

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

}