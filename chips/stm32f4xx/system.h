#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
// #include <ctype.h>
// #include <stdarg.h>

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

void board_init(void);
void systemInit(void);
void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);

uint64_t micros(void);
uint32_t millis(void);

// bootloader/IAP
void systemReset(void);
void systemResetToBootloader(void);

#ifdef __cplusplus
}
#endif

#endif //SYSTEM_H
