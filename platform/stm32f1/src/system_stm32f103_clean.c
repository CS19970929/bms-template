#include "stm32f10x.h"
#include "system_stm32f10x.h"

#define BMS_HSI_HZ 8000000UL
#define BMS_PLL_HZ 64000000UL
uint32_t SystemCoreClock=BMS_HSI_HZ;
void SystemInit(void){SystemCoreClock=BMS_HSI_HZ;}
void SystemCoreClockUpdate(void){SystemCoreClock=((RCC->CFGR&RCC_CFGR_SWS)==RCC_CFGR_SWS_PLL)?BMS_PLL_HZ:BMS_HSI_HZ;}
