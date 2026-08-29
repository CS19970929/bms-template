#include "bms_nvm_store.h"
#include "bms_nvm_record.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

static int io_valid(const bms_nvm_store_io_t *io, uint32_t maximum_payload_length)
{
    if ((io == NULL) || (io->read == NULL) || (io->erase == NULL) || (io->program == NULL)) {
        return 0;
    }
    if ((io->slot_size < BMS_NVM_RECORD_HEADER_SIZE) ||
        (maximum_payload_length > (io->slot_size - BMS_NVM_RECORD_HEADER_SIZE))) {
        return 0;
    }
    return 1;
}

static bms_nvm_store_result_t scan_latest(const bms_nvm_store_io_t *io,
                                          uint16_t schema_version,
                                          uint32_t maximum_payload_length,
                                          uint8_t *scratch,
                                          size_t scratch_capacity,
                                          bms_nvm_store_info_t *info)
{
    uint8_t header_bytes[BMS_NVM_RECORD_HEADER_SIZE];
    uint8_t slot;
    info->valid = 0U;
    info->sequence = 0U;
    info->payload_length = 0U;
    info->slot = 0U;

    for (slot = 0U; slot < BMS_NVM_STORE_SLOT_COUNT; ++slot) {
        bms_nvm_record_header_t header;
        bms_nvm_result_t record_result;
        if (io->read(io->context, slot, 0U, header_bytes, sizeof(header_bytes)) != 0) {
            return BMS_NVM_STORE_ERR_IO;
        }
        record_result = bms_nvm_record_decode_header(header_bytes, sizeof(header_bytes), &header);
        if (record_result != BMS_NVM_OK) {
            return BMS_NVM_STORE_ERR_VERIFY;
        }
        record_result = bms_nvm_record_validate_header(&header, schema_version, maximum_payload_length);
        if (record_result != BMS_NVM_OK) {
            continue;
        }
        if ((size_t)header.payload_length > scratch_capacity) {
            return BMS_NVM_STORE_ERR_LAYOUT;
        }
        if ((header.payload_length != 0U) &&
            (io->read(io->context, slot, BMS_NVM_RECORD_HEADER_SIZE, scratch,
                      (size_t)header.payload_length) != 0)) {
            return BMS_NVM_STORE_ERR_IO;
        }
        record_result = bms_nvm_record_validate(&header, schema_version, scratch,
                                                (size_t)header.payload_length,
                                                maximum_payload_length);
        if (record_result != BMS_NVM_OK) {
            continue;
        }
        if ((info->valid == 0U) || (bms_nvm_sequence_is_newer(header.sequence, info->sequence) != 0)) {
            info->valid = 1U;
            info->sequence = header.sequence;
            info->payload_length = header.payload_length;
            info->slot = slot;
        }
    }
    return BMS_NVM_STORE_OK;
}

bms_nvm_store_result_t bms_nvm_store_load(const bms_nvm_store_io_t *io,
                                          uint16_t schema_version,
                                          uint32_t maximum_payload_length,
                                          uint8_t *payload_out,
                                          size_t payload_capacity,
                                          uint8_t *scratch,
                                          size_t scratch_capacity,
                                          bms_nvm_store_info_t *info_out)
{
    bms_nvm_store_info_t info;
    uint8_t header_bytes[BMS_NVM_RECORD_HEADER_SIZE];
    bms_nvm_record_header_t header;
    bms_nvm_store_result_t result;

    if ((payload_out == NULL) || (scratch == NULL) || (info_out == NULL)) {
        return BMS_NVM_STORE_ERR_ARGUMENT;
    }
    if ((payload_capacity < (size_t)maximum_payload_length) ||
        (scratch_capacity < (size_t)maximum_payload_length) ||
        (io_valid(io, maximum_payload_length) == 0)) {
        return BMS_NVM_STORE_ERR_LAYOUT;
    }

    result = scan_latest(io, schema_version, maximum_payload_length, scratch, scratch_capacity, &info);
    if (result != BMS_NVM_STORE_OK) {
        return result;
    }
    if (info.valid == 0U) {
        *info_out = info;
        return BMS_NVM_STORE_ERR_NOT_FOUND;
    }
    if (io->read(io->context, info.slot, 0U, header_bytes, sizeof(header_bytes)) != 0) {
        return BMS_NVM_STORE_ERR_IO;
    }
    if (bms_nvm_record_decode_header(header_bytes, sizeof(header_bytes), &header) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_VERIFY;
    }
    if ((header.payload_length != 0U) &&
        (io->read(io->context, info.slot, BMS_NVM_RECORD_HEADER_SIZE, payload_out,
                  (size_t)header.payload_length) != 0)) {
        return BMS_NVM_STORE_ERR_IO;
    }
    if (bms_nvm_record_validate(&header, schema_version, payload_out, (size_t)header.payload_length,
                                maximum_payload_length) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_VERIFY;
    }
    *info_out = info;
    return BMS_NVM_STORE_OK;
}

bms_nvm_store_result_t bms_nvm_store_commit(const bms_nvm_store_io_t *io,
                                            uint16_t schema_version,
                                            const uint8_t *payload,
                                            size_t payload_length,
                                            uint8_t *scratch,
                                            size_t scratch_capacity,
                                            bms_nvm_store_info_t *info_out)
{
    bms_nvm_store_info_t current;
    bms_nvm_record_header_t header;
    bms_nvm_record_header_t verify_header;
    uint8_t encoded_header[BMS_NVM_RECORD_HEADER_SIZE];
    uint8_t target_slot;
    uint32_t next_sequence;
    uint32_t maximum_payload_length;
    bms_nvm_store_result_t scan_result;

    if (((payload == NULL) && (payload_length != 0U)) || (scratch == NULL) || (info_out == NULL)) {
        return BMS_NVM_STORE_ERR_ARGUMENT;
    }
    if ((payload_length > (size_t)UINT32_MAX) || (scratch_capacity > (size_t)UINT32_MAX) ||
        (payload_length > scratch_capacity)) {
        return BMS_NVM_STORE_ERR_LAYOUT;
    }
    maximum_payload_length = (uint32_t)scratch_capacity;
    if (io_valid(io, maximum_payload_length) == 0) {
        return BMS_NVM_STORE_ERR_LAYOUT;
    }

    scan_result = scan_latest(io, schema_version, maximum_payload_length, scratch, scratch_capacity, &current);
    if (scan_result != BMS_NVM_STORE_OK) {
        return scan_result;
    }
    target_slot = (current.valid != 0U) ? (uint8_t)(1U - current.slot) : 0U;
    next_sequence = (current.valid != 0U) ? (current.sequence + 1U) : 1U;

    if (bms_nvm_record_prepare(&header, schema_version, next_sequence, payload, payload_length) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_ARGUMENT;
    }
    if (bms_nvm_record_encode_header(&header, encoded_header, sizeof(encoded_header)) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_VERIFY;
    }

    if (io->erase(io->context, target_slot) != 0) {
        return BMS_NVM_STORE_ERR_IO;
    }
    if ((payload_length != 0U) &&
        (io->program(io->context, target_slot, BMS_NVM_RECORD_HEADER_SIZE, payload, payload_length) != 0)) {
        return BMS_NVM_STORE_ERR_IO;
    }
    /* Header is the commit marker and is programmed last. */
    if (io->program(io->context, target_slot, 0U, encoded_header, sizeof(encoded_header)) != 0) {
        return BMS_NVM_STORE_ERR_IO;
    }

    if (io->read(io->context, target_slot, 0U, encoded_header, sizeof(encoded_header)) != 0) {
        return BMS_NVM_STORE_ERR_IO;
    }
    if (bms_nvm_record_decode_header(encoded_header, sizeof(encoded_header), &verify_header) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_VERIFY;
    }
    if ((payload_length != 0U) &&
        (io->read(io->context, target_slot, BMS_NVM_RECORD_HEADER_SIZE, scratch, payload_length) != 0)) {
        return BMS_NVM_STORE_ERR_IO;
    }
    if (bms_nvm_record_validate(&verify_header, schema_version, scratch, payload_length,
                                maximum_payload_length) != BMS_NVM_OK) {
        return BMS_NVM_STORE_ERR_VERIFY;
    }

    info_out->valid = 1U;
    info_out->slot = target_slot;
    info_out->sequence = next_sequence;
    info_out->payload_length = (uint32_t)payload_length;
    return BMS_NVM_STORE_OK;
}
