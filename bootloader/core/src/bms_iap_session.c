#include "bms_iap_session.h"
#include "bms_crc32.h"
#include <stddef.h>
#include <string.h>

static int manifest_is_acceptable(const bms_image_manifest_t *image, const bms_image_constraints_t *constraints)
{
    uint32_t capacity;
    if ((image == NULL) || (constraints == NULL)) return 0;
    if ((image->magic != BMS_IMAGE_MAGIC) || (image->manifest_version != BMS_IMAGE_MANIFEST_VERSION)) return 0;
    if ((image->mcu_id != constraints->mcu_id) || (image->product_id != constraints->product_id)) return 0;
    if (constraints->app_end_exclusive <= constraints->app_start) return 0;
    capacity = constraints->app_end_exclusive - constraints->app_start;
    if ((image->image_size < 8U) || (image->image_size > capacity)) return 0;
    return 1;
}

static int metadata_store(const bms_iap_storage_t *storage, bms_boot_meta_state_t state,
                          const bms_image_manifest_t *image)
{
    if ((storage == NULL) || (storage->store_metadata == NULL)) return -1;
    return storage->store_metadata(storage->ctx, state, image);
}

static int duplicate_matches(const bms_iap_session_t *session, const bms_iap_storage_t *storage,
                             uint32_t offset, const uint8_t *data, uint16_t length)
{
    uint8_t existing[BMS_IAP_MAX_CHUNK_SIZE];
    uint32_t address;
    if ((session->has_last_chunk == 0U) || (offset != session->last_offset) ||
        (length != session->last_length) || (bms_crc32(data, length) != session->last_chunk_crc32)) return 0;
    if ((storage == NULL) || (storage->read == NULL)) return 0;
    address = session->constraints.app_start + offset;
    if (storage->read(storage->ctx, address, existing, length) != 0) return 0;
    return (memcmp(existing, data, length) == 0) ? 1 : 0;
}

void bms_iap_session_init(bms_iap_session_t *session, const bms_image_constraints_t *constraints)
{
    if (session == NULL) return;
    (void)memset(session, 0, sizeof(*session));
    session->state = BMS_IAP_STATE_IDLE;
    if (constraints != NULL) session->constraints = *constraints;
}

bms_iap_result_t bms_iap_start(bms_iap_session_t *session, const bms_iap_storage_t *storage,
                               const bms_image_manifest_t *image)
{
    if ((session == NULL) || (storage == NULL) || (image == NULL) || (storage->erase_app == NULL)) return BMS_IAP_ERR_ARGUMENT;
    if ((session->state != BMS_IAP_STATE_IDLE) && (session->state != BMS_IAP_STATE_INVALID)) return BMS_IAP_ERR_STATE;
    if (manifest_is_acceptable(image, &session->constraints) == 0) return BMS_IAP_ERR_MANIFEST;

    session->image = *image;
    session->next_offset = 0U;
    session->has_last_chunk = 0U;
    if (metadata_store(storage, BMS_BOOT_META_RECEIVING, image) != 0) {
        session->state = BMS_IAP_STATE_INVALID;
        return BMS_IAP_ERR_METADATA;
    }
    session->state = BMS_IAP_STATE_ERASING;
    if (storage->erase_app(storage->ctx) != 0) {
        session->state = BMS_IAP_STATE_INVALID;
        (void)metadata_store(storage, BMS_BOOT_META_INVALID, image);
        return BMS_IAP_ERR_STORAGE;
    }
    session->state = BMS_IAP_STATE_RECEIVING;
    return BMS_IAP_OK;
}

bms_iap_result_t bms_iap_write_chunk(bms_iap_session_t *session, const bms_iap_storage_t *storage,
                                     uint32_t offset, const uint8_t *data, uint16_t length)
{
    uint32_t end_offset;
    uint32_t address;
    if ((session == NULL) || (storage == NULL) || (data == NULL) || (storage->write == NULL)) return BMS_IAP_ERR_ARGUMENT;
    if (session->state != BMS_IAP_STATE_RECEIVING) return BMS_IAP_ERR_STATE;
    if ((length == 0U) || (length > BMS_IAP_MAX_CHUNK_SIZE)) return BMS_IAP_ERR_CHUNK;

    if (offset != session->next_offset) {
        if (duplicate_matches(session, storage, offset, data, length) != 0) return BMS_IAP_OK_DUPLICATE;
        return BMS_IAP_ERR_OFFSET;
    }
    if (offset > session->image.image_size) return BMS_IAP_ERR_RANGE;
    end_offset = offset + (uint32_t)length;
    if ((end_offset < offset) || (end_offset > session->image.image_size)) return BMS_IAP_ERR_RANGE;
    address = session->constraints.app_start + offset;
    if ((address < session->constraints.app_start) ||
        ((address + (uint32_t)length) < address) ||
        ((address + (uint32_t)length) > session->constraints.app_end_exclusive)) return BMS_IAP_ERR_RANGE;

    if (storage->write(storage->ctx, address, data, length) != 0) return BMS_IAP_ERR_STORAGE;
    session->last_offset = offset;
    session->last_length = length;
    session->last_chunk_crc32 = bms_crc32(data, length);
    session->has_last_chunk = 1U;
    session->next_offset = end_offset;
    return BMS_IAP_OK;
}

bms_iap_result_t bms_iap_verify(bms_iap_session_t *session, const bms_iap_storage_t *storage)
{
    bms_image_result_t result;
    if ((session == NULL) || (storage == NULL) || (storage->read == NULL)) return BMS_IAP_ERR_ARGUMENT;
    if (session->state != BMS_IAP_STATE_RECEIVING) return BMS_IAP_ERR_STATE;
    if (session->next_offset != session->image.image_size) return BMS_IAP_ERR_RANGE;
    session->state = BMS_IAP_STATE_VERIFYING;
    result = bms_boot_image_validate(&session->image, &session->constraints, storage->read, storage->ctx);
    if (result != BMS_IMAGE_OK) {
        session->state = BMS_IAP_STATE_INVALID;
        (void)metadata_store(storage, BMS_BOOT_META_INVALID, &session->image);
        return BMS_IAP_ERR_VERIFY;
    }
    session->state = BMS_IAP_STATE_VERIFIED;
    return BMS_IAP_OK;
}

bms_iap_result_t bms_iap_commit(bms_iap_session_t *session, const bms_iap_storage_t *storage)
{
    if ((session == NULL) || (storage == NULL)) return BMS_IAP_ERR_ARGUMENT;
    if (session->state != BMS_IAP_STATE_VERIFIED) return BMS_IAP_ERR_STATE;
    if (metadata_store(storage, BMS_BOOT_META_READY, &session->image) != 0) {
        session->state = BMS_IAP_STATE_INVALID;
        return BMS_IAP_ERR_METADATA;
    }
    session->state = BMS_IAP_STATE_READY;
    return BMS_IAP_OK;
}

bms_iap_result_t bms_iap_abort(bms_iap_session_t *session, const bms_iap_storage_t *storage)
{
    if ((session == NULL) || (storage == NULL)) return BMS_IAP_ERR_ARGUMENT;
    if (metadata_store(storage, BMS_BOOT_META_INVALID, &session->image) != 0) return BMS_IAP_ERR_METADATA;
    session->state = BMS_IAP_STATE_INVALID;
    return BMS_IAP_OK;
}
