#include "system.h"

void board_init(void)
{
  // Initialize the timing and other parts of the airdamon system
  systemInit();

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  // GPIO EXTI (for imu)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // Enable clock to DMA1, an AHB peripheral
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

  // Enable clock to SPI1, an APB2 peripheral
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
}