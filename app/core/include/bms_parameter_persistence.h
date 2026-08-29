#ifndef BMS_PARAMETER_PERSISTENCE_H
#define BMS_PARAMETER_PERSISTENCE_H

#include "bms_parameter.h"
#include <stddef.h>
#include <stdint.h>

#define BMS_PARAMETER_PERSIST_MAGIC UINT32_C(0x52415042)
#define BMS_PARAMETER_PERSIST_FORMAT_VERSION UINT16_C(1)
#define BMS_PARAMETER_PERSIST_HEADER_SIZE 8U
#define BMS_PARAMETER_PERSIST_ENTRY_SIZE 8U

typedef enum {
    BMS_PARAMETER_PERSIST_OK = 0,
    BMS_PARAMETER_PERSIST_ERR_ARGUMENT = -1,
    BMS_PARAMETER_PERSIST_ERR_DESCRIPTOR = -2,
    BMS_PARAMETER_PERSIST_ERR_CAPACITY = -3,
    BMS_PARAMETER_PERSIST_ERR_FORMAT = -4,
    BMS_PARAMETER_PERSIST_ERR_VALUE = -5,
    BMS_PARAMETER_PERSIST_ERR_CROSS_FIELD = -6
} bms_parameter_persist_result_t;

bms_parameter_persist_result_t bms_parameter_persist_encode(
    const bms_param_descriptor_t *descriptors,
    const bms_param_value_t *values,
    size_t count,
    uint8_t *encoded,
    size_t encoded_capacity,
    size_t *encoded_length);

bms_parameter_persist_result_t bms_parameter_persist_restore(
    const bms_param_descriptor_t *descriptors,
    bms_param_value_t *active_values,
    bms_param_value_t *staged_values,
    size_t count,
    const uint8_t *encoded,
    size_t encoded_length,
    bms_param_cross_validate_fn cross_validate,
    void *context);

#endif
