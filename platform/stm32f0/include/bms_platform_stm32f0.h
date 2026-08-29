#ifndef BMS_PLATFORM_STM32F0_H
#define BMS_PLATFORM_STM32F0_H

#include <stddef.h>
#include <stdint.h>

#define BMS_F030_FLASH_START 0x08000000UL
#define BMS_F030_BOOT_START  0x08000000UL
#define BMS_F030_BOOT_END    0x08003000UL
#define BMS_F030_APP_START   0x08003000UL
#define BMS_F030_APP_END     0x0800F800UL
#define BMS_F030_META_A      0x0800F800UL
#define BMS_F030_META_B      0x0800FC00UL
#define BMS_F030_FLASH_END   0x08010000UL
#define BMS_F030_RAM_START   0x20000000UL
#define BMS_F030_RAM_END     0x20002000UL
#define BMS_F030_VECTOR_WORDS 48U

void bms_platform_clock_init(void);
void bms_platform_uart_init(uint32_t baudrate);
int bms_platform_uart_try_read(uint8_t *byte);
void bms_platform_uart_write(const uint8_t *data, size_t length);
void bms_platform_watchdog_start(void);
void bms_platform_watchdog_reload(void);
int bms_platform_flash_read(void *ctx, uint32_t address, uint8_t *dst, size_t length);
int bms_platform_flash_erase_app(void);
int bms_platform_flash_write(uint32_t address, const uint8_t *data, size_t length);
void bms_platform_jump_to_app(uint32_t app_start);
void bms_platform_app_vector_remap(void);

#endif
