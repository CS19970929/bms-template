#include "bms_platform_stm32f0.h"
#include "stm32f0xx.h"
#include <string.h>

static int range_in_app(uint32_t address, size_t length)
{
    uint32_t end;
    if (length == 0U) return 1;
    if (address < BMS_F030_APP_START) return 0;
    if (length > (size_t)(BMS_F030_APP_END - BMS_F030_APP_START)) return 0;
    end = address + (uint32_t)length;
    if (end < address) return 0;
    return (end <= BMS_F030_APP_END) ? 1 : 0;
}

void bms_platform_clock_init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG | RCC_APB2Periph_USART1, ENABLE);
}

void bms_platform_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;
    bms_platform_clock_init();
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_Level_2;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);
    USART_StructInit(&uart);
    uart.USART_BaudRate = baudrate;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

int bms_platform_uart_try_read(uint8_t *byte)
{
    if (byte == NULL) return 0;
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) return 0;
    *byte = (uint8_t)USART_ReceiveData(USART1);
    return 1;
}

void bms_platform_uart_write(const uint8_t *data, size_t length)
{
    size_t i;
    if ((data == NULL) && (length != 0U)) return;
    for (i = 0U; i < length; ++i) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) { }
        USART_SendData(USART1, data[i]);
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { }
}

void bms_platform_watchdog_start(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(625U);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void bms_platform_watchdog_reload(void) { IWDG_ReloadCounter(); }

int bms_platform_flash_read(void *ctx, uint32_t address, uint8_t *dst, size_t length)
{
    (void)ctx;
    if ((dst == NULL) || (address < BMS_F030_FLASH_START) || (length > (size_t)(BMS_F030_FLASH_END - address))) return -1;
    (void)memcpy(dst, (const void *)(uintptr_t)address, length);
    return 0;
}

int bms_platform_flash_erase_app(void)
{
    uint32_t page;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    for (page = BMS_F030_APP_START; page < BMS_F030_APP_END; page += 1024U) {
        if (FLASH_ErasePage(page) != FLASH_COMPLETE) { FLASH_Lock(); return -1; }
        bms_platform_watchdog_reload();
    }
    FLASH_Lock();
    return 0;
}

int bms_platform_flash_write(uint32_t address, const uint8_t *data, size_t length)
{
    size_t i;
    if ((data == NULL) || ((address & 1U) != 0U) || ((length & 1U) != 0U) || (range_in_app(address, length) == 0)) return -1;
    FLASH_Unlock();
    for (i = 0U; i < length; i += 2U) {
        uint16_t half = (uint16_t)data[i] | ((uint16_t)data[i + 1U] << 8U);
        if (FLASH_ProgramHalfWord(address + (uint32_t)i, half) != FLASH_COMPLETE) { FLASH_Lock(); return -1; }
        if (*(const volatile uint16_t *)(uintptr_t)(address + (uint32_t)i) != half) { FLASH_Lock(); return -1; }
    }
    FLASH_Lock();
    return 0;
}

void bms_platform_jump_to_app(uint32_t app_start)
{
    typedef void (*entry_fn_t)(void);
    const uint32_t msp = *(const volatile uint32_t *)(uintptr_t)app_start;
    const uint32_t reset = *(const volatile uint32_t *)(uintptr_t)(app_start + 4U);
    entry_fn_t entry = (entry_fn_t)(uintptr_t)reset;
    __disable_irq();
    SysTick->CTRL = 0U; SysTick->LOAD = 0U; SysTick->VAL = 0U;
    NVIC->ICER[0] = 0xFFFFFFFFUL;
    NVIC->ICPR[0] = 0xFFFFFFFFUL;
    __set_MSP(msp);
    entry();
    for (;;) { }
}

void bms_platform_app_vector_remap(void)
{
    uint32_t i;
    volatile uint32_t *const sram_vectors = (volatile uint32_t *)(uintptr_t)BMS_F030_RAM_START;
    const uint32_t *const flash_vectors = (const uint32_t *)(uintptr_t)BMS_F030_APP_START;
    for (i = 0U; i < BMS_F030_VECTOR_WORDS; ++i) sram_vectors[i] = flash_vectors[i];
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
    __DSB(); __ISB();
}
