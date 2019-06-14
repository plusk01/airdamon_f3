#include "system.h"

void board_init(void)
{
  // Initialize the timing and other parts of the airdamon system
  systemInit();

  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  // GPIO EXTI (for imu)
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // Enable clock to DMA1, an AHB peripheral
  // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  //TODO: Should these be abstracted with the board-specific (ie revo_f4.h) file?
    // RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  // Enable clock to SPI1, an APB2 peripheral
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
}