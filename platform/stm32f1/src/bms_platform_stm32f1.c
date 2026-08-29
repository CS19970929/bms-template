#include "bms_platform.h"
#include "bms_target_config.h"
#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include <string.h>

#define BMS_UART_POLL_LIMIT 1000000UL
#define BMS_CLOCK_POLL_LIMIT 1000000UL

static int range_in_app(uint32_t address, size_t length)
{
    uint32_t end;
    if (length == 0U) return 1;
    if (address < BMS_TARGET_APP_START) return 0;
    if (length > (size_t)BMS_TARGET_APP_SIZE) return 0;
    end = address + (uint32_t)length;
    if (end < address) return 0;
    return (end <= BMS_TARGET_APP_END) ? 1 : 0;
}

static int erase_page(uint32_t address)
{
    return (FLASH_ErasePage(address) == FLASH_COMPLETE) ? 0 : -1;
}

static int program_bytes(uint32_t address, const uint8_t *data, size_t length)
{
    size_t i;
    if ((data == NULL) || ((address & 1U) != 0U)) return -1;
    for (i = 0U; i < length; i += 2U) {
        const uint8_t hi = ((i + 1U) < length) ? data[i + 1U] : 0xFFU;
        const uint16_t half = (uint16_t)data[i] | ((uint16_t)hi << 8U);
        if (FLASH_ProgramHalfWord(address + (uint32_t)i, half) != FLASH_COMPLETE) return -1;
        if (*(const volatile uint16_t *)(uintptr_t)(address + (uint32_t)i) != half) return -1;
    }
    return 0;
}

static void jump_with_msp(uint32_t msp, uint32_t reset)
{
#if defined(__GNUC__)
    __asm volatile(
        "msr msp, %0\n"
        "bx %1\n"
        :
        : "r"(msp), "r"(reset)
        : "memory"
    );
    __builtin_unreachable();
#else
    typedef void (*entry_fn_t)(void);
    const entry_fn_t entry = (entry_fn_t)(uintptr_t)reset;
    __set_MSP(msp);
    entry();
    for (;;) { }
#endif
}

void bms_platform_clock_init(void)
{
    if (RCC_GetSYSCLKSource() != 0x08U) {
        uint32_t polls;
        FLASH_SetLatency(FLASH_Latency_2);
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
        RCC_PLLCmd(ENABLE);
        polls = BMS_CLOCK_POLL_LIMIT;
        while ((RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) && (polls != 0U)) --polls;
        if (polls != 0U) {
            RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
            polls = BMS_CLOCK_POLL_LIMIT;
            while ((RCC_GetSYSCLKSource() != 0x08U) && (polls != 0U)) --polls;
        }
    }
    SystemCoreClockUpdate();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
}

void bms_platform_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;
    bms_platform_clock_init();
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
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

int bms_platform_uart_write(const uint8_t *data, size_t length)
{
    size_t i;
    if ((data == NULL) && (length != 0U)) return -1;
    for (i = 0U; i < length; ++i) {
        uint32_t polls = BMS_UART_POLL_LIMIT;
        while ((USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) && (polls != 0U)) --polls;
        if (polls == 0U) return -1;
        USART_SendData(USART1, data[i]);
    }
    {
        uint32_t polls = BMS_UART_POLL_LIMIT;
        while ((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) && (polls != 0U)) --polls;
        if (polls == 0U) return -1;
    }
    return 0;
}

void bms_platform_watchdog_start(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);
    IWDG_SetReload(625U);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void bms_platform_watchdog_reload(void)
{
    IWDG_ReloadCounter();
}

int bms_platform_flash_read(void *ctx, uint32_t address, uint8_t *dst, size_t length)
{
    (void)ctx;
    if ((dst == NULL) || (address < BMS_TARGET_BOOT_START) || (address >= BMS_TARGET_FLASH_END)) return -1;
    if (length > (size_t)(BMS_TARGET_FLASH_END - address)) return -1;
    (void)memcpy(dst, (const void *)(uintptr_t)address, length);
    return 0;
}

int bms_platform_flash_erase_app(void *ctx)
{
    uint32_t page;
    (void)ctx;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    for (page = BMS_TARGET_APP_START;
         page < BMS_TARGET_APP_END;
         page += (uint32_t)BMS_TARGET_FLASH_PAGE_SIZE) {
        if (erase_page(page) != 0) {
            FLASH_Lock();
            return -1;
        }
        bms_platform_watchdog_reload();
    }
    FLASH_Lock();
    return 0;
}

int bms_platform_flash_write(void *ctx, uint32_t address, const uint8_t *data, size_t length)
{
    int result;
    (void)ctx;
    if ((data == NULL) || (range_in_app(address, length) == 0)) return -1;
    FLASH_Unlock();
    result = program_bytes(address, data, length);
    FLASH_Lock();
    return result;
}

int bms_platform_metadata_store(void *ctx, bms_boot_meta_state_t state, const bms_image_manifest_t *image)
{
    const bms_boot_meta_record_t *const a = (const bms_boot_meta_record_t *)(uintptr_t)BMS_TARGET_METADATA_A;
    const bms_boot_meta_record_t *const b = (const bms_boot_meta_record_t *)(uintptr_t)BMS_TARGET_METADATA_B;
    const bms_boot_meta_record_t *current;
    bms_boot_meta_record_t next;
    uint32_t destination;
    int result;
    (void)ctx;
    current = bms_boot_metadata_select(a, b);
    destination = (current == a) ? BMS_TARGET_METADATA_B : BMS_TARGET_METADATA_A;
    bms_boot_metadata_prepare_next(&next, current, state, image);
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    result = erase_page(destination);
    if (result == 0) result = program_bytes(destination, (const uint8_t *)&next, sizeof(next));
    FLASH_Lock();
    if (result != 0) return -1;
    return bms_boot_metadata_is_valid((const bms_boot_meta_record_t *)(uintptr_t)destination) ? 0 : -1;
}

void bms_platform_jump_to_app(uint32_t app_start)
{
    const uint32_t msp = *(const volatile uint32_t *)(uintptr_t)app_start;
    const uint32_t reset = *(const volatile uint32_t *)(uintptr_t)(app_start + 4U);
    uint32_t i;
    __disable_irq();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;
    for (i = 0U; i < 8U; ++i) {
        NVIC->ICER[i] = 0xFFFFFFFFUL;
        NVIC->ICPR[i] = 0xFFFFFFFFUL;
    }
    jump_with_msp(msp, reset);
}

void bms_platform_app_vector_remap(void)
{
    SCB->VTOR = BMS_TARGET_APP_START;
    __DSB();
    __ISB();
}

void bms_platform_system_reset(void)
{
    NVIC_SystemReset();
}
