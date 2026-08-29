#include "stm32f0xx.h"
#include "system_stm32f0xx.h"

#define BMS_HSI_HZ 8000000UL
#define BMS_PLL_HZ 48000000UL

uint32_t SystemCoreClock = BMS_HSI_HZ;

void SystemInit(void)
{
    /* Reset guarantees HSI is enabled. Keep startup deterministic and independent
       from any legacy project's clock macros; platform clock setup happens later. */
    SystemCoreClock = BMS_HSI_HZ;
}

void SystemCoreClockUpdate(void)
{
    const uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;
    if (sws == RCC_CFGR_SWS_PLL) SystemCoreClock = BMS_PLL_HZ;
    else SystemCoreClock = BMS_HSI_HZ;
}
