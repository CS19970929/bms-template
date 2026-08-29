#include "bms_parameter.h"

static size_t bms_parameter_find(const bms_param_descriptor_t *descriptors, size_t count, uint16_t id)
{
    size_t i;
    for (i = 0U; i < count; ++i) {
        if (descriptors[i].id == id) {
            return i;
        }
    }
    return count;
}

bms_param_result_t bms_parameter_validate(const bms_param_descriptor_t *descriptor,
                                          bms_param_type_t type,
                                          bms_param_value_t value)
{
    if (descriptor == NULL) {
        return BMS_PARAM_ERR_ARGUMENT;
    }
    if (descriptor->type != type) {
        return BMS_PARAM_ERR_TYPE;
    }
    switch (type) {
    case BMS_PARAM_I32:
        if ((value.i32 < descriptor->minimum.i32) || (value.i32 > descriptor->maximum.i32)) {
            return BMS_PARAM_ERR_RANGE;
        }
        break;
    case BMS_PARAM_U32:
        if ((value.u32 < descriptor->minimum.u32) || (value.u32 > descriptor->maximum.u32)) {
            return BMS_PARAM_ERR_RANGE;
        }
        break;
    case BMS_PARAM_BOOL:
        if ((value.boolean > 1U) || (descriptor->minimum.boolean > descriptor->maximum.boolean) ||
            (value.boolean < descriptor->minimum.boolean) || (value.boolean > descriptor->maximum.boolean)) {
            return BMS_PARAM_ERR_RANGE;
        }
        break;
    }
    return BMS_PARAM_OK;
}

void bms_parameter_load_defaults(const bms_param_descriptor_t *descriptors,
                                 bms_param_value_t *values,
                                 size_t count)
{
    size_t i;
    if ((descriptors == NULL) || (values == NULL)) {
        return;
    }
    for (i = 0U; i < count; ++i) {
        values[i] = descriptors[i].default_value;
    }
}

bms_param_result_t bms_parameter_transaction_begin(bms_param_transaction_t *transaction,
                                                    const bms_param_descriptor_t *descriptors,
                                                    const bms_param_value_t *active_values,
                                                    bms_param_value_t *staged_values,
                                                    size_t count)
{
    size_t i;
    if ((transaction == NULL) || (descriptors == NULL) || (active_values == NULL) ||
        (staged_values == NULL) || (count == 0U)) {
        return BMS_PARAM_ERR_ARGUMENT;
    }
    for (i = 0U; i < count; ++i) {
        staged_values[i] = active_values[i];
    }
    transaction->descriptors = descriptors;
    transaction->staged_values = staged_values;
    transaction->count = count;
    transaction->active = 1U;
    return BMS_PARAM_OK;
}

bms_param_result_t bms_parameter_transaction_set(bms_param_transaction_t *transaction,
                                                  uint16_t id,
                                                  bms_param_type_t type,
                                                  bms_param_value_t value,
                                                  uint32_t caller_access_mask)
{
    size_t index;
    bms_param_result_t result;
    if ((transaction == NULL) || (transaction->active == 0U) ||
        (transaction->descriptors == NULL) || (transaction->staged_values == NULL)) {
        return BMS_PARAM_ERR_TRANSACTION;
    }
    index = bms_parameter_find(transaction->descriptors, transaction->count, id);
    if (index == transaction->count) {
        return BMS_PARAM_ERR_NOT_FOUND;
    }
    if ((transaction->descriptors[index].write_access_mask & caller_access_mask) == 0U) {
        return BMS_PARAM_ERR_PERMISSION;
    }
    result = bms_parameter_validate(&transaction->descriptors[index], type, value);
    if (result != BMS_PARAM_OK) {
        return result;
    }
    transaction->staged_values[index] = value;
    return BMS_PARAM_OK;
}

bms_param_result_t bms_parameter_transaction_commit(bms_param_transaction_t *transaction,
                                                     bms_param_value_t *active_values,
                                                     bms_param_cross_validate_fn cross_validate,
                                                     void *context)
{
    size_t i;
    if ((transaction == NULL) || (transaction->active == 0U) || (active_values == NULL) ||
        (transaction->descriptors == NULL) || (transaction->staged_values == NULL)) {
        return BMS_PARAM_ERR_TRANSACTION;
    }
    if ((cross_validate != NULL) &&
        (cross_validate(transaction->descriptors, transaction->staged_values, transaction->count, context) != 0)) {
        return BMS_PARAM_ERR_CROSS_FIELD;
    }
    for (i = 0U; i < transaction->count; ++i) {
        active_values[i] = transaction->staged_values[i];
    }
    transaction->active = 0U;
    return BMS_PARAM_OK;
}

void bms_parameter_transaction_abort(bms_param_transaction_t *transaction)
{
    if (transaction != NULL) {
        transaction->active = 0U;
    }
}
