#ifndef BMS_PARAMETER_STORE_H
#define BMS_PARAMETER_STORE_H

#include "bms_nvm_store.h"
#include "bms_parameter.h"
#include <stddef.h>
#include <stdint.h>

#define BMS_PARAMETER_STORE_NVM_SCHEMA_VERSION UINT16_C(1)

typedef enum {
    BMS_PARAMETER_STORE_OK = 0,
    BMS_PARAMETER_STORE_NOT_FOUND = 1,
    BMS_PARAMETER_STORE_ERR_ARGUMENT = -1,
    BMS_PARAMETER_STORE_ERR_CAPACITY = -2,
    BMS_PARAMETER_STORE_ERR_CODEC = -3,
    BMS_PARAMETER_STORE_ERR_NVM = -4
} bms_parameter_store_result_t;

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
    bms_nvm_store_info_t *info_out);

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
    bms_nvm_store_info_t *info_out);

#endif
