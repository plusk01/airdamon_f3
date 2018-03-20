#include "pwm.h"

namespace airdamon {

void PWM::init(const PWMConfig* config, uint16_t frequency, uint32_t min_us, uint32_t max_us)
{
  cfg_ = config;

  // initialize the GPIO for the output pwm pin
  pin_.init(config->GPIOx, config->pin, GPIO::PERIPH_OUT);

  // Set GPIO pins as alternate function
  GPIO_PinAFConfig(config->GPIOx, config->pin_source, config->GPIO_AF);

  init_TIM(frequency, min_us, max_us);
}

// ----------------------------------------------------------------------------

void PWM::enable()
{

}

// ----------------------------------------------------------------------------

void PWM::disable()
{

}

// ----------------------------------------------------------------------------

void PWM::write(float value)
{

}

// ----------------------------------------------------------------------------

void PWM::write_us(uint16_t value)
{

}

// ----------------------------------------------------------------------------
// Private Methods
// ----------------------------------------------------------------------------

void PWM::init_TIM(uint16_t frequency, uint32_t min_us, uint32_t max_us)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef       TIM_OCInitStructure;



  //calculate timer values
  //This is dependent on how fast the SystemCoreClock is. (ie will change between stm32fX models)
  const uint16_t prescaler_default = 42;
  uint32_t freq_prescale = prescaler_default * 2;
  uint32_t tim_prescaler = prescaler_default;

  // if (TIMPtr == TIM9 || TIMPtr == TIM10 || TIMPtr == TIM11)
  // {
  //   //For F4's (possibly others) TIM9-11 have a max timer clk double that of all the other TIMs
  //   //compensate for this by doubling its prescaler
  //   tim_prescaler = tim_prescaler * 2;
  // }
  uint32_t timer_freq_hz = SystemCoreClock / freq_prescale;

  cycles_per_us_ = timer_freq_hz / 1000000;//E^6
  max_cyc_ = max_us * cycles_per_us_;
  min_cyc_ = min_us * cycles_per_us_;



  //init timer
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period        = (2000000 / frequency) - 1; // 0 indexed
  TIM_TimeBaseStructure.TIM_Prescaler     = tim_prescaler - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //0x0000
  TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
  TIM_TimeBaseInit(cfg_->TIMx, &TIM_TimeBaseStructure);

  //init output compare
  TIM_OCStructInit(&TIM_OCInitStructure);
  TIM_OCInitStructure.TIM_OCMode        = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState   = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState  = TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_Pulse         = min_cyc_ - 1;
  TIM_OCInitStructure.TIM_OCPolarity    = TIM_OCPolarity_Low;
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

  TIM_ARRPreloadConfig(cfg_->TIMx, ENABLE);
  TIM_Cmd(cfg_->TIMx, ENABLE);
}

}