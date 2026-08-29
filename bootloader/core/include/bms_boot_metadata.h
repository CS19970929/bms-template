#ifndef BMS_BOOT_METADATA_H
#define BMS_BOOT_METADATA_H

#include <stdint.h>
#include "bms_boot_image.h"

#define BMS_BOOT_META_MAGIC 0x424D534DUL

typedef enum {
    BMS_BOOT_META_EMPTY = 0,
    BMS_BOOT_META_RECEIVING = 1,
    BMS_BOOT_META_READY = 2,
    BMS_BOOT_META_CONFIRMED = 3,
    BMS_BOOT_META_INVALID = 4
} bms_boot_meta_state_t;

typedef struct {
    uint32_t magic;
    uint32_t sequence;
    uint32_t state;
    bms_image_manifest_t image;
    uint32_t record_crc32;
} bms_boot_meta_record_t;

uint32_t bms_boot_metadata_crc(const bms_boot_meta_record_t *record);
int bms_boot_metadata_is_valid(const bms_boot_meta_record_t *record);
const bms_boot_meta_record_t *bms_boot_metadata_select(const bms_boot_meta_record_t *a,
                                                       const bms_boot_meta_record_t *b);
void bms_boot_metadata_prepare_next(bms_boot_meta_record_t *dst,
                                    const bms_boot_meta_record_t *current,
                                    bms_boot_meta_state_t state,
                                    const bms_image_manifest_t *image);

#endif
