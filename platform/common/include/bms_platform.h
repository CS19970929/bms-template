#ifndef BMS_PLATFORM_H
#define BMS_PLATFORM_H

#include "bms_boot_metadata.h"
#include <stddef.h>
#include <stdint.h>

void bms_platform_clock_init(void);
void bms_platform_uart_init(uint32_t baudrate);
int bms_platform_uart_try_read(uint8_t *byte);
int bms_platform_uart_write(const uint8_t *data, size_t length);
void bms_platform_watchdog_start(void);
void bms_platform_watchdog_reload(void);
int bms_platform_flash_read(void *ctx, uint32_t address, uint8_t *dst, size_t length);
int bms_platform_flash_erase_app(void *ctx);
int bms_platform_flash_write(void *ctx, uint32_t address, const uint8_t *data, size_t length);
int bms_platform_metadata_store(void *ctx, bms_boot_meta_state_t state, const bms_image_manifest_t *image);
int bms_platform_nvm_read(void *ctx, uint8_t slot, uint32_t offset, uint8_t *dst, size_t length);
int bms_platform_nvm_erase(void *ctx, uint8_t slot);
int bms_platform_nvm_program(void *ctx, uint8_t slot, uint32_t offset,
                             const uint8_t *data, size_t length);
uint32_t bms_platform_nvm_slot_size(void);
void bms_platform_jump_to_app(uint32_t app_start);
void bms_platform_app_vector_remap(void);
void bms_platform_system_reset(void);

#endif
