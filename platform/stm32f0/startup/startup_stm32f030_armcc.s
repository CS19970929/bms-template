; Clean-room ARMCC5 startup for STM32F030 reference target.
; Keil is a debug view; production source/layout remains CMake/config driven.

Stack_Size      EQU     0x00000400
                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp
                DCD     Reset_Handler
                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     0,0,0,0,0,0,0
                DCD     SVC_Handler
                DCD     0,0
                DCD     PendSV_Handler
                DCD     SysTick_Handler
                DCD     WWDG_IRQHandler
                DCD     PVD_IRQHandler
                DCD     RTC_IRQHandler
                DCD     FLASH_IRQHandler
                DCD     RCC_IRQHandler
                DCD     EXTI0_1_IRQHandler
                DCD     EXTI2_3_IRQHandler
                DCD     EXTI4_15_IRQHandler
                DCD     TS_IRQHandler
                DCD     DMA1_Channel1_IRQHandler
                DCD     DMA1_Channel2_3_IRQHandler
                DCD     DMA1_Channel4_5_IRQHandler
                DCD     ADC1_COMP_IRQHandler
                DCD     TIM1_BRK_UP_TRG_COM_IRQHandler
                DCD     TIM1_CC_IRQHandler
                DCD     TIM2_IRQHandler
                DCD     TIM3_IRQHandler
                DCD     TIM6_DAC_IRQHandler
                DCD     0
                DCD     TIM14_IRQHandler
                DCD     TIM15_IRQHandler
                DCD     TIM16_IRQHandler
                DCD     TIM17_IRQHandler
                DCD     I2C1_IRQHandler
                DCD     I2C2_IRQHandler
                DCD     SPI1_IRQHandler
                DCD     SPI2_IRQHandler
                DCD     USART1_IRQHandler
                DCD     USART2_IRQHandler
                DCD     0
                DCD     CEC_IRQHandler
__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY
Reset_Handler   PROC
                EXPORT  Reset_Handler [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                BL      SystemInit
                B       __main
                ENDP

NMI_Handler     PROC
                EXPORT  NMI_Handler [WEAK]
                B       .
                ENDP
HardFault_Handler PROC
                EXPORT  HardFault_Handler [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT WWDG_IRQHandler [WEAK]
                EXPORT PVD_IRQHandler [WEAK]
                EXPORT RTC_IRQHandler [WEAK]
                EXPORT FLASH_IRQHandler [WEAK]
                EXPORT RCC_IRQHandler [WEAK]
                EXPORT EXTI0_1_IRQHandler [WEAK]
                EXPORT EXTI2_3_IRQHandler [WEAK]
                EXPORT EXTI4_15_IRQHandler [WEAK]
                EXPORT TS_IRQHandler [WEAK]
                EXPORT DMA1_Channel1_IRQHandler [WEAK]
                EXPORT DMA1_Channel2_3_IRQHandler [WEAK]
                EXPORT DMA1_Channel4_5_IRQHandler [WEAK]
                EXPORT ADC1_COMP_IRQHandler [WEAK]
                EXPORT TIM1_BRK_UP_TRG_COM_IRQHandler [WEAK]
                EXPORT TIM1_CC_IRQHandler [WEAK]
                EXPORT TIM2_IRQHandler [WEAK]
                EXPORT TIM3_IRQHandler [WEAK]
                EXPORT TIM6_DAC_IRQHandler [WEAK]
                EXPORT TIM14_IRQHandler [WEAK]
                EXPORT TIM15_IRQHandler [WEAK]
                EXPORT TIM16_IRQHandler [WEAK]
                EXPORT TIM17_IRQHandler [WEAK]
                EXPORT I2C1_IRQHandler [WEAK]
                EXPORT I2C2_IRQHandler [WEAK]
                EXPORT SPI1_IRQHandler [WEAK]
                EXPORT SPI2_IRQHandler [WEAK]
                EXPORT USART1_IRQHandler [WEAK]
                EXPORT USART2_IRQHandler [WEAK]
                EXPORT CEC_IRQHandler [WEAK]
WWDG_IRQHandler
PVD_IRQHandler
RTC_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_1_IRQHandler
EXTI2_3_IRQHandler
EXTI4_15_IRQHandler
TS_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_3_IRQHandler
DMA1_Channel4_5_IRQHandler
ADC1_COMP_IRQHandler
TIM1_BRK_UP_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM6_DAC_IRQHandler
TIM14_IRQHandler
TIM15_IRQHandler
TIM16_IRQHandler
TIM17_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
CEC_IRQHandler
                B       .
                ENDP

                EXPORT  __initial_sp
                END
