#include "bms_parameter_store.h"
#include "bms_nvm_record.h"
#include "bms_parameter_persistence.h"

static int buffers_valid(const bms_nvm_store_io_t *io,
                         uint32_t payload_limit,
                         const uint8_t *payload_buffer,
                         size_t payload_capacity,
                         const uint8_t *nvm_scratch,
                         size_t nvm_scratch_capacity)
{
    if ((io == NULL) || (payload_buffer == NULL) || (nvm_scratch == NULL)) {
        return 0;
    }
    if (payload_limit < BMS_PARAMETER_PERSIST_HEADER_SIZE) {
        return 0;
    }
    if ((payload_capacity < (size_t)payload_limit) ||
        (nvm_scratch_capacity < (size_t)payload_limit)) {
        return 0;
    }
    if ((io->slot_size < BMS_NVM_RECORD_HEADER_SIZE) ||
        (payload_limit > (io->slot_size - BMS_NVM_RECORD_HEADER_SIZE))) {
        return 0;
    }
    return 1;
}

bms_parameter_store_result_t bms_parameter_store_load(
    const bms_nvm_store_io_t *io,
    uint32_t payload_limit,
    const bms_param_descriptor_t *descriptors,
    bms_param_value_t *active_values,
    bms_param_value_t *staged_values,
    size_t count,
    uint8_t *payload_buffer,
    size_t payload_capacity,
    uint8_t *nvm_scratch,
    size_t nvm_scratch_capacity,
    bms_param_cross_validate_fn cross_validate,
    void *cross_validate_context,
    bms_nvm_store_info_t *info_out)
{
    bms_nvm_store_result_t nvm_result;
    bms_parameter_persist_result_t codec_result;

    if ((descriptors == NULL) || (active_values == NULL) || (staged_values == NULL) ||
        (info_out == NULL) || (count == 0U)) {
        return BMS_PARAMETER_STORE_ERR_ARGUMENT;
    }
    if (buffers_valid(io, payload_limit, payload_buffer, payload_capacity,
                      nvm_scratch, nvm_scratch_capacity) == 0) {
        return BMS_PARAMETER_STORE_ERR_CAPACITY;
    }

    nvm_result = bms_nvm_store_load(io, BMS_PARAMETER_STORE_NVM_SCHEMA_VERSION,
                                    payload_limit, payload_buffer, payload_capacity,
                                    nvm_scratch, nvm_scratch_capacity, info_out);
    if (nvm_result == BMS_NVM_STORE_ERR_NOT_FOUND) {
        return BMS_PARAMETER_STORE_NOT_FOUND;
    }
    if (nvm_result != BMS_NVM_STORE_OK) {
        return BMS_PARAMETER_STORE_ERR_NVM;
    }

    codec_result = bms_parameter_persist_restore(
        descriptors, active_values, staged_values, count,
        payload_buffer, (size_t)info_out->payload_length,
        cross_validate, cross_validate_context);
    if (codec_result != BMS_PARAMETER_PERSIST_OK) {
        return BMS_PARAMETER_STORE_ERR_CODEC;
    }
    return BMS_PARAMETER_STORE_OK;
}

bms_parameter_store_result_t bms_parameter_store_commit(
    const bms_nvm_store_io_t *io,
    uint32_t payload_limit,
    const bms_param_descriptor_t *descriptors,
    const bms_param_value_t *active_values,
    size_t count,
    uint8_t *payload_buffer,
    size_t payload_capacity,
    uint8_t *nvm_scratch,
    size_t nvm_scratch_capacity,
    bms_nvm_store_info_t *info_out)
{
    bms_parameter_persist_result_t codec_result;
    bms_nvm_store_result_t nvm_result;
    size_t encoded_length = 0U;

    if ((descriptors == NULL) || (active_values == NULL) || (info_out == NULL) || (count == 0U)) {
        return BMS_PARAMETER_STORE_ERR_ARGUMENT;
    }
    if (buffers_valid(io, payload_limit, payload_buffer, payload_capacity,
                      nvm_scratch, nvm_scratch_capacity) == 0) {
        return BMS_PARAMETER_STORE_ERR_CAPACITY;
    }

    codec_result = bms_parameter_persist_encode(
        descriptors, active_values, count, payload_buffer,
        (size_t)payload_limit, &encoded_length);
    if (codec_result == BMS_PARAMETER_PERSIST_ERR_CAPACITY) {
        return BMS_PARAMETER_STORE_ERR_CAPACITY;
    }
    if (codec_result != BMS_PARAMETER_PERSIST_OK) {
        return BMS_PARAMETER_STORE_ERR_CODEC;
    }

    nvm_result = bms_nvm_store_commit(
        io, BMS_PARAMETER_STORE_NVM_SCHEMA_VERSION,
        payload_buffer, encoded_length, nvm_scratch,
        (size_t)payload_limit, info_out);
    if (nvm_result != BMS_NVM_STORE_OK) {
        return BMS_PARAMETER_STORE_ERR_NVM;
    }
    return BMS_PARAMETER_STORE_OK;
}
