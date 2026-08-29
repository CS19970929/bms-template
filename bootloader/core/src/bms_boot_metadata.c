#include "bms_boot_metadata.h"
#include "bms_crc32.h"
#include <stddef.h>
#include <string.h>

uint32_t bms_boot_metadata_crc(const bms_boot_meta_record_t *record)
{
    if (record == NULL) {
        return 0U;
    }
    return bms_crc32((const uint8_t *)record, offsetof(bms_boot_meta_record_t, record_crc32));
}

int bms_boot_metadata_is_valid(const bms_boot_meta_record_t *record)
{
    if ((record == NULL) || (record->magic != BMS_BOOT_META_MAGIC)) {
        return 0;
    }
    if (record->state > (uint32_t)BMS_BOOT_META_INVALID) {
        return 0;
    }
    return (record->record_crc32 == bms_boot_metadata_crc(record)) ? 1 : 0;
}

const bms_boot_meta_record_t *bms_boot_metadata_select(const bms_boot_meta_record_t *a,
                                                       const bms_boot_meta_record_t *b)
{
    const int a_valid = bms_boot_metadata_is_valid(a);
    const int b_valid = bms_boot_metadata_is_valid(b);
    if ((a_valid == 0) && (b_valid == 0)) {
        return NULL;
    }
    if (a_valid == 0) {
        return b;
    }
    if (b_valid == 0) {
        return a;
    }
    return (a->sequence >= b->sequence) ? a : b;
}

void bms_boot_metadata_prepare_next(bms_boot_meta_record_t *dst,
                                    const bms_boot_meta_record_t *current,
                                    bms_boot_meta_state_t state,
                                    const bms_image_manifest_t *image)
{
    if (dst == NULL) {
        return;
    }
    (void)memset(dst, 0, sizeof(*dst));
    dst->magic = BMS_BOOT_META_MAGIC;
    dst->sequence = (current == NULL) ? 1U : (current->sequence + 1U);
    dst->state = (uint32_t)state;
    if (image != NULL) {
        dst->image = *image;
    } else if (current != NULL) {
        dst->image = current->image;
    }
    dst->record_crc32 = bms_boot_metadata_crc(dst);
}
