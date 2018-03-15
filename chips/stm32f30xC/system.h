#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
// #include <ctype.h>
// #include <stdarg.h>

#include "stm32f30x.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_i2c.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_usart.h"

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
