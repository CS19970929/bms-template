#ifndef BMS_IAP_SESSION_H
#define BMS_IAP_SESSION_H

#include "bms_boot_image.h"
#include "bms_boot_metadata.h"
#include <stddef.h>
#include <stdint.h>

#define BMS_IAP_MAX_CHUNK_SIZE 256U

typedef enum {
    BMS_IAP_STATE_IDLE = 0,
    BMS_IAP_STATE_ERASING,
    BMS_IAP_STATE_RECEIVING,
    BMS_IAP_STATE_VERIFYING,
    BMS_IAP_STATE_VERIFIED,
    BMS_IAP_STATE_READY,
    BMS_IAP_STATE_INVALID
} bms_iap_state_t;

typedef enum {
    BMS_IAP_OK = 0,
    BMS_IAP_OK_DUPLICATE,
    BMS_IAP_ERR_ARGUMENT,
    BMS_IAP_ERR_STATE,
    BMS_IAP_ERR_MANIFEST,
    BMS_IAP_ERR_RANGE,
    BMS_IAP_ERR_OFFSET,
    BMS_IAP_ERR_CHUNK,
    BMS_IAP_ERR_STORAGE,
    BMS_IAP_ERR_VERIFY,
    BMS_IAP_ERR_METADATA
} bms_iap_result_t;

typedef struct {
    int (*erase_app)(void *ctx);
    int (*write)(void *ctx, uint32_t address, const uint8_t *data, size_t length);
    int (*read)(void *ctx, uint32_t address, uint8_t *data, size_t length);
    int (*store_metadata)(void *ctx, bms_boot_meta_state_t state, const bms_image_manifest_t *image);
    void *ctx;
} bms_iap_storage_t;

typedef struct {
    bms_iap_state_t state;
    bms_image_constraints_t constraints;
    bms_image_manifest_t image;
    uint32_t next_offset;
    uint32_t last_offset;
    uint16_t last_length;
    uint32_t last_chunk_crc32;
    uint8_t has_last_chunk;
} bms_iap_session_t;

void bms_iap_session_init(bms_iap_session_t *session, const bms_image_constraints_t *constraints);
bms_iap_result_t bms_iap_start(bms_iap_session_t *session, const bms_iap_storage_t *storage,
                               const bms_image_manifest_t *image);
bms_iap_result_t bms_iap_write_chunk(bms_iap_session_t *session, const bms_iap_storage_t *storage,
                                     uint32_t offset, const uint8_t *data, uint16_t length);
bms_iap_result_t bms_iap_verify(bms_iap_session_t *session, const bms_iap_storage_t *storage);
bms_iap_result_t bms_iap_commit(bms_iap_session_t *session, const bms_iap_storage_t *storage);
bms_iap_result_t bms_iap_abort(bms_iap_session_t *session, const bms_iap_storage_t *storage);

#endif
