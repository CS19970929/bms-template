#include <stdint.h>
#include "stm32f0xx.h"
#include "bms_platform_stm32f0.h"

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
#define IRQ(name) void name(void) __attribute__((weak, alias("Default_Handler")))
IRQ(WWDG_IRQHandler); IRQ(PVD_IRQHandler); IRQ(RTC_IRQHandler); IRQ(FLASH_IRQHandler); IRQ(RCC_IRQHandler);
IRQ(EXTI0_1_IRQHandler); IRQ(EXTI2_3_IRQHandler); IRQ(EXTI4_15_IRQHandler); IRQ(TS_IRQHandler);
IRQ(DMA1_Channel1_IRQHandler); IRQ(DMA1_Channel2_3_IRQHandler); IRQ(DMA1_Channel4_5_IRQHandler);
IRQ(ADC1_COMP_IRQHandler); IRQ(TIM1_BRK_UP_TRG_COM_IRQHandler); IRQ(TIM1_CC_IRQHandler); IRQ(TIM2_IRQHandler);
IRQ(TIM3_IRQHandler); IRQ(TIM6_DAC_IRQHandler); IRQ(TIM14_IRQHandler); IRQ(TIM15_IRQHandler); IRQ(TIM16_IRQHandler);
IRQ(TIM17_IRQHandler); IRQ(I2C1_IRQHandler); IRQ(I2C2_IRQHandler); IRQ(SPI1_IRQHandler); IRQ(SPI2_IRQHandler);
IRQ(USART1_IRQHandler); IRQ(USART2_IRQHandler); IRQ(CEC_IRQHandler);

__attribute__((used, section(".isr_vector")))
const uintptr_t g_vectors[BMS_F030_VECTOR_WORDS] = {
    (uintptr_t)&_estack, (uintptr_t)Reset_Handler, (uintptr_t)NMI_Handler, (uintptr_t)HardFault_Handler,
    0U,0U,0U,0U,0U,0U,0U,(uintptr_t)SVC_Handler,0U,0U,(uintptr_t)PendSV_Handler,(uintptr_t)SysTick_Handler,
    (uintptr_t)WWDG_IRQHandler,(uintptr_t)PVD_IRQHandler,(uintptr_t)RTC_IRQHandler,(uintptr_t)FLASH_IRQHandler,
    (uintptr_t)RCC_IRQHandler,(uintptr_t)EXTI0_1_IRQHandler,(uintptr_t)EXTI2_3_IRQHandler,(uintptr_t)EXTI4_15_IRQHandler,
    (uintptr_t)TS_IRQHandler,(uintptr_t)DMA1_Channel1_IRQHandler,(uintptr_t)DMA1_Channel2_3_IRQHandler,
    (uintptr_t)DMA1_Channel4_5_IRQHandler,(uintptr_t)ADC1_COMP_IRQHandler,(uintptr_t)TIM1_BRK_UP_TRG_COM_IRQHandler,
    (uintptr_t)TIM1_CC_IRQHandler,(uintptr_t)TIM2_IRQHandler,(uintptr_t)TIM3_IRQHandler,(uintptr_t)TIM6_DAC_IRQHandler,
    0U,(uintptr_t)TIM14_IRQHandler,(uintptr_t)TIM15_IRQHandler,(uintptr_t)TIM16_IRQHandler,(uintptr_t)TIM17_IRQHandler,
    (uintptr_t)I2C1_IRQHandler,(uintptr_t)I2C2_IRQHandler,(uintptr_t)SPI1_IRQHandler,(uintptr_t)SPI2_IRQHandler,
    (uintptr_t)USART1_IRQHandler,(uintptr_t)USART2_IRQHandler,0U,(uintptr_t)CEC_IRQHandler
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst;
    for (dst = &_sdata; dst < &_edata; ++dst, ++src) *dst = *src;
    for (dst = &_sbss; dst < &_ebss; ++dst) *dst = 0U;
    SystemInit();
    (void)main();
    for (;;) { }
}

void Default_Handler(void) { for (;;) { } }
