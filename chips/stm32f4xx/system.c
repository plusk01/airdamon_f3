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
    return ((uint64_t)sysTickUptime * 3125ul)/100ul;  // The conversion is 31.25, so doing fixed-point math to be exact
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
    SysTick_Config(SystemCoreClock / 32000 + 28);
    NVIC_SetPriority(SysTick_IRQn, 0);
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

