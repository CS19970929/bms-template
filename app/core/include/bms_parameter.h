#ifndef BMS_PARAMETER_H
#define BMS_PARAMETER_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    BMS_PARAM_I32 = 0,
    BMS_PARAM_U32,
    BMS_PARAM_BOOL
} bms_param_type_t;

typedef union {
    int32_t i32;
    uint32_t u32;
    uint8_t boolean;
} bms_param_value_t;

#define BMS_PARAM_ACCESS_USER    (1UL << 0U)
#define BMS_PARAM_ACCESS_SERVICE (1UL << 1U)
#define BMS_PARAM_ACCESS_FACTORY (1UL << 2U)

typedef struct {
    uint16_t id;
    bms_param_type_t type;
    uint32_t write_access_mask;
    uint8_t persistent;
    bms_param_value_t default_value;
    bms_param_value_t minimum;
    bms_param_value_t maximum;
} bms_param_descriptor_t;

typedef enum {
    BMS_PARAM_OK = 0,
    BMS_PARAM_ERR_ARGUMENT = -1,
    BMS_PARAM_ERR_NOT_FOUND = -2,
    BMS_PARAM_ERR_TYPE = -3,
    BMS_PARAM_ERR_RANGE = -4,
    BMS_PARAM_ERR_PERMISSION = -5,
    BMS_PARAM_ERR_TRANSACTION = -6,
    BMS_PARAM_ERR_CROSS_FIELD = -7
} bms_param_result_t;

typedef int (*bms_param_cross_validate_fn)(const bms_param_descriptor_t *descriptors,
                                           const bms_param_value_t *values,
                                           size_t count,
                                           void *context);

typedef struct {
    const bms_param_descriptor_t *descriptors;
    bms_param_value_t *staged_values;
    size_t count;
    uint8_t active;
} bms_param_transaction_t;

bms_param_result_t bms_parameter_validate(const bms_param_descriptor_t *descriptor,
                                          bms_param_type_t type,
                                          bms_param_value_t value);
void bms_parameter_load_defaults(const bms_param_descriptor_t *descriptors,
                                 bms_param_value_t *values,
                                 size_t count);
bms_param_result_t bms_parameter_transaction_begin(bms_param_transaction_t *transaction,
                                                    const bms_param_descriptor_t *descriptors,
                                                    const bms_param_value_t *active_values,
                                                    bms_param_value_t *staged_values,
                                                    size_t count);
bms_param_result_t bms_parameter_transaction_set(bms_param_transaction_t *transaction,
                                                  uint16_t id,
                                                  bms_param_type_t type,
                                                  bms_param_value_t value,
                                                  uint32_t caller_access_mask);
bms_param_result_t bms_parameter_transaction_commit(bms_param_transaction_t *transaction,
                                                     bms_param_value_t *active_values,
                                                     bms_param_cross_validate_fn cross_validate,
                                                     void *context);
void bms_parameter_transaction_abort(bms_param_transaction_t *transaction);

#endif
