/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "system.h"


// cycles per microsecond
static uint32_t usTicks = 0;
// current uptime for 32kHz systick timer. will rollover after 1.5 days. hopefully we won't care.
static volatile uint32_t sysTickUptime = 0;

// SysTick
void SysTick_Handler(void)
{
    sysTickUptime++;
}

// Return system uptime in microseconds (rollover in 1.5 days)
uint64_t micros(void)
{
    return ((uint64_t)sysTickUptime * 3125ul)/100ul;  // The convsersion is 31.25, so doing fixed-point math to be exact
}

// Return system uptime in milliseconds (rollover in 1.5 days)
uint32_t millis(void)
{
    return (uint32_t)(sysTickUptime >> 5);  // ( >> 5 is the same as divide by 32, but takes one operation)
}

void systemInit(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //2 bit preemption, 2 bit sub priority

    // Configure Systick
    SysTick_Config(SystemCoreClock / 32000 + 27);
    NVIC_SetPriority(SysTick_IRQn, 0);

    //TODO: Should these be abstracted with the board-specific (ie revo_f4.h) file?
    // RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, DISABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
    // RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);

    // RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

}

void delayMicroseconds(uint32_t us)
{
    uint32_t now = micros();
    while (micros() - now < us);
}

void delay(uint32_t ms)
{
    while (ms--)
        delayMicroseconds(1000);
}







// /*
//  * This file is part of Cleanflight.
//  *
//  * Cleanflight is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * Cleanflight is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
//  */

// #include <stdbool.h>
// #include <stdint.h>

// #include "platform.h"

// #include "gpio.h"
// #include "nvic.h"
// #include "system.h"

// #define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)
// void SetSysClock();

// void systemReset(void)
// {
//     // Generate system reset
//     SCB->AIRCR = AIRCR_VECTKEY_MASK | (uint32_t)0x04;
// }

// void systemResetToBootloader(void)
// {
//     // 1FFFF000 -> 20000200 -> SP
//     // 1FFFF004 -> 1FFFF021 -> PC

//     *((uint32_t *)0x20009FFC) = 0xDEADBEEF; // 40KB SRAM STM32F30X

//     systemReset();
// }


// void enableGPIOPowerUsageAndNoiseReductions(void)
// {
//     RCC_AHBPeriphClockCmd(
//         RCC_AHBPeriph_GPIOA |
//         RCC_AHBPeriph_GPIOB |
//         RCC_AHBPeriph_GPIOC |
//         RCC_AHBPeriph_GPIOD |
//         RCC_AHBPeriph_GPIOE |
//         RCC_AHBPeriph_GPIOF,
//         ENABLE
//     );

//     gpio_config_t gpio;

//     gpio.mode = Mode_AIN;

//     gpio.pin = Pin_All & ~(Pin_13 | Pin_14 | Pin_15);  // Leave JTAG pins alone
//     gpioInit(GPIOA, &gpio);

//     gpio.pin = Pin_All;
//     gpioInit(GPIOB, &gpio);
//     gpioInit(GPIOC, &gpio);
//     gpioInit(GPIOD, &gpio);
//     gpioInit(GPIOE, &gpio);
//     gpioInit(GPIOF, &gpio);
// }

// bool isMPUSoftReset(void)
// {
//     if (cachedRccCsrValue & RCC_CSR_SFTRSTF)
//         return true;
//     else
//         return false;
// }

// void systemInit(void)
// {
//     checkForBootLoaderRequest();

//     // Enable FPU
//     SCB->CPACR = (0x3 << (10 * 2)) | (0x3 << (11 * 2));
//     SetSysClock();

//     // Configure NVIC preempt/priority groups
//     NVIC_PriorityGroupConfig(NVIC_PRIORITY_GROUPING);

//     // cache RCC->CSR value to use it in isMPUSoftreset() and others
//     cachedRccCsrValue = RCC->CSR;
//     RCC_ClearFlag();

//     enableGPIOPowerUsageAndNoiseReductions();

//     // Init cycle counter
//     cycleCounterInit();

//     // SysTick
//     SysTick_Config(SystemCoreClock / 1000);
// }

// void checkForBootLoaderRequest(void)
// {
//     void(*bootJump)(void);

//     if (*((uint32_t *)0x20009FFC) == 0xDEADBEEF) {

//         *((uint32_t *)0x20009FFC) = 0x0;

//         __enable_irq();
//         __set_MSP(*((uint32_t *)0x1FFFD800));

//         bootJump = (void(*)(void))(*((uint32_t *) 0x1FFFD804));
//         bootJump();
//         while (1);
//     }
// }
