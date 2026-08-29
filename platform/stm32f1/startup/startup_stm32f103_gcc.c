#include <stdint.h>
#include "stm32f10x.h"
extern uint32_t _estack,_sidata,_sdata,_edata,_sbss,_ebss;extern int main(void);
void Reset_Handler(void);void Default_Handler(void);void NMI_Handler(void)__attribute__((weak,alias("Default_Handler")));void HardFault_Handler(void)__attribute__((weak,alias("Default_Handler")));void MemManage_Handler(void)__attribute__((weak,alias("Default_Handler")));void BusFault_Handler(void)__attribute__((weak,alias("Default_Handler")));void UsageFault_Handler(void)__attribute__((weak,alias("Default_Handler")));void SVC_Handler(void)__attribute__((weak,alias("Default_Handler")));void DebugMon_Handler(void)__attribute__((weak,alias("Default_Handler")));void PendSV_Handler(void)__attribute__((weak,alias("Default_Handler")));void SysTick_Handler(void)__attribute__((weak,alias("Default_Handler")));
__attribute__((used,section(".isr_vector"))) const uintptr_t g_vectors[59]={
(uintptr_t)&_estack,(uintptr_t)Reset_Handler,(uintptr_t)NMI_Handler,(uintptr_t)HardFault_Handler,(uintptr_t)MemManage_Handler,(uintptr_t)BusFault_Handler,(uintptr_t)UsageFault_Handler,0U,0U,0U,0U,(uintptr_t)SVC_Handler,(uintptr_t)DebugMon_Handler,0U,(uintptr_t)PendSV_Handler,(uintptr_t)SysTick_Handler,
[16 ... 58]=(uintptr_t)Default_Handler};
void Reset_Handler(void){uint32_t *src=&_sidata,*dst;for(dst=&_sdata;dst<&_edata;++dst,++src)*dst=*src;for(dst=&_sbss;dst<&_ebss;++dst)*dst=0U;SystemInit();(void)main();for(;;){}}
void Default_Handler(void){for(;;){}}
