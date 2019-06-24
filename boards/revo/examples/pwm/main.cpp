#include <revo.h>


uint32_t get_timer_clock_frequency(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  uint32_t multiplier;
  if (RCC_Clocks.PCLK1_Frequency == RCC_Clocks.SYSCLK_Frequency) {
    multiplier = 1;
  } else {
    multiplier = 2;
  }
  printf("\tPCLK1: %d\tPCLK2: %d\tSYSCLK: %d\n", RCC_Clocks.PCLK1_Frequency,
                    RCC_Clocks.PCLK2_Frequency, RCC_Clocks.SYSCLK_Frequency);
  return multiplier * RCC_Clocks.PCLK1_Frequency;
}

int main()
{
  board_init();

  airdamon::LED info, warn;
  info.init(GPIOB, GPIO_Pin_5, true);
  warn.init(GPIOB, GPIO_Pin_4, true);

  airdamon::VCP vcp;
  vcp.init();
  vcp.connect_to_printf();

  airdamon::PWM pwm[NUM_PWMS];
  pwm[0].init(&pwm_config[0], 490, 1000, 2000);
  pwm[1].init(&pwm_config[1], 490, 1000, 2000);
  pwm[2].init(&pwm_config[2], 490, 1000, 2000);
  pwm[3].init(&pwm_config[3], 50, 1000, 8000);

  printf("\n**** PWM Tester ****\n\n");

  int i = 1000, i50 = 1000;

  while(1) {
    info.toggle();

    i += 100;
    i50 += 100;
    if (i>2000) { warn.toggle(); i = 1000; }
    if (i50>8000) i50 = 1000;
    pwm[0].write_us(i);
    pwm[1].write_us(i);
    pwm[2].write_us(i);
    pwm[3].write_us(i50);

    delay(50);

    printf("System Core Clock: %d\n", SystemCoreClock);
    printf("Timer Clock Frew: %d\n", get_timer_clock_frequency());
  }  
}
