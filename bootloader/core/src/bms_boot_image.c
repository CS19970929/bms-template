#include "bms_boot_image.h"
#include "bms_crc32.h"

#define CRC_CHUNK 64U

static uint32_t u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8U) | ((uint32_t)p[2] << 16U) | ((uint32_t)p[3] << 24U);
}

bms_image_result_t bms_boot_image_validate(const bms_image_manifest_t *manifest,
                                           const bms_image_constraints_t *constraints,
                                           bms_flash_read_fn read_fn, void *read_ctx)
{
    uint8_t chunk[CRC_CHUNK];
    uint8_t vectors[8];
    uint32_t crc = 0xFFFFFFFFUL;
    uint32_t offset = 0U;
    uint32_t msp;
    uint32_t reset;

    if ((manifest == NULL) || (constraints == NULL) || (read_fn == NULL)) {
        return BMS_IMAGE_ERR_ARGUMENT;
    }
    if (manifest->magic != BMS_IMAGE_MAGIC) {
        return BMS_IMAGE_ERR_MAGIC;
    }
    if (manifest->manifest_version != BMS_IMAGE_MANIFEST_VERSION) {
        return BMS_IMAGE_ERR_MANIFEST_VERSION;
    }
    if ((manifest->mcu_id != constraints->mcu_id) || (manifest->product_id != constraints->product_id)) {
        return BMS_IMAGE_ERR_TARGET;
    }
    if ((manifest->image_size < 8U) || (manifest->image_size > (constraints->app_end_exclusive - constraints->app_start))) {
        return BMS_IMAGE_ERR_SIZE;
    }

    while (offset < manifest->image_size) {
        const uint32_t remaining = manifest->image_size - offset;
        const size_t take = (remaining > CRC_CHUNK) ? CRC_CHUNK : (size_t)remaining;
        if (read_fn(read_ctx, constraints->app_start + offset, chunk, take) != 0) {
            return BMS_IMAGE_ERR_READ;
        }
        crc = bms_crc32_update(crc, chunk, take);
        offset += (uint32_t)take;
    }
    crc ^= 0xFFFFFFFFUL;
    if (crc != manifest->image_crc32) {
        return BMS_IMAGE_ERR_CRC;
    }
    if (read_fn(read_ctx, constraints->app_start, vectors, sizeof(vectors)) != 0) {
        return BMS_IMAGE_ERR_READ;
    }
    msp = u32_le(&vectors[0]);
    reset = u32_le(&vectors[4]);
    if ((msp < constraints->ram_start) || (msp > constraints->ram_end_exclusive) || ((msp & 0x3U) != 0U)) {
        return BMS_IMAGE_ERR_MSP;
    }
    if (((reset & 1U) == 0U) || ((reset & ~1UL) < constraints->app_start) || ((reset & ~1UL) >= constraints->app_end_exclusive)) {
        return BMS_IMAGE_ERR_RESET_HANDLER;
    }
    return BMS_IMAGE_OK;
}
