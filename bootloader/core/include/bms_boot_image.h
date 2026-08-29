#ifndef BMS_BOOT_IMAGE_H
#define BMS_BOOT_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#define BMS_IMAGE_MAGIC 0x424D5349UL
#define BMS_IMAGE_MANIFEST_VERSION 1U

typedef enum {
    BMS_MCU_UNKNOWN = 0,
    BMS_MCU_STM32F030C8 = 1,
    BMS_MCU_STM32F103C8 = 2
} bms_mcu_id_t;

typedef struct {
    uint32_t magic;
    uint16_t manifest_version;
    uint16_t mcu_id;
    uint32_t product_id;
    uint32_t image_size;
    uint32_t image_crc32;
    uint32_t firmware_version;
} bms_image_manifest_t;

typedef struct {
    uint32_t app_start;
    uint32_t app_end_exclusive;
    uint32_t ram_start;
    uint32_t ram_end_exclusive;
    uint16_t mcu_id;
    uint32_t product_id;
} bms_image_constraints_t;

typedef int (*bms_flash_read_fn)(void *ctx, uint32_t address, uint8_t *dst, size_t length);

typedef enum {
    BMS_IMAGE_OK = 0,
    BMS_IMAGE_ERR_ARGUMENT,
    BMS_IMAGE_ERR_MAGIC,
    BMS_IMAGE_ERR_MANIFEST_VERSION,
    BMS_IMAGE_ERR_TARGET,
    BMS_IMAGE_ERR_SIZE,
    BMS_IMAGE_ERR_READ,
    BMS_IMAGE_ERR_CRC,
    BMS_IMAGE_ERR_MSP,
    BMS_IMAGE_ERR_RESET_HANDLER
} bms_image_result_t;

bms_image_result_t bms_boot_image_validate(const bms_image_manifest_t *manifest,
                                           const bms_image_constraints_t *constraints,
                                           bms_flash_read_fn read_fn, void *read_ctx);

#endif
