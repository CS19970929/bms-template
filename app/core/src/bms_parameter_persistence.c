#include "bms_parameter_persistence.h"
#include <limits.h>

static void put16(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
}

static void put32(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
    dst[2] = (uint8_t)(value >> 16U);
    dst[3] = (uint8_t)(value >> 24U);
}

static uint16_t get16(const uint8_t *src)
{
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8U));
}

static uint32_t get32(const uint8_t *src)
{
    return (uint32_t)src[0] |
           ((uint32_t)src[1] << 8U) |
           ((uint32_t)src[2] << 16U) |
           ((uint32_t)src[3] << 24U);
}

static int type_valid(bms_param_type_t type)
{
    return (type <= BMS_PARAM_BOOL) ? 1 : 0;
}

static int descriptor_table_valid(const bms_param_descriptor_t *descriptors, size_t count)
{
    size_t i;
    size_t j;
    if ((descriptors == NULL) || (count == 0U)) {
        return 0;
    }
    for (i = 0U; i < count; ++i) {
        const bms_param_descriptor_t *descriptor = &descriptors[i];
        if ((descriptor->id == 0U) || (type_valid(descriptor->type) == 0) ||
            (descriptor->persistent > 1U)) {
            return 0;
        }
        if (bms_parameter_validate(descriptor, descriptor->type, descriptor->default_value) != BMS_PARAM_OK) {
            return 0;
        }
        for (j = i + 1U; j < count; ++j) {
            if (descriptor->id == descriptors[j].id) {
                return 0;
            }
        }
    }
    return 1;
}

static size_t descriptor_find(const bms_param_descriptor_t *descriptors, size_t count, uint16_t id)
{
    size_t i;
    for (i = 0U; i < count; ++i) {
        if (descriptors[i].id == id) {
            return i;
        }
    }
    return count;
}

static uint32_t value_to_raw(bms_param_type_t type, bms_param_value_t value)
{
    switch (type) {
    case BMS_PARAM_I32:
        return (uint32_t)value.i32;
    case BMS_PARAM_U32:
        return value.u32;
    case BMS_PARAM_BOOL:
        return (uint32_t)value.boolean;
    }
    return UINT32_C(0);
}

static int32_t raw_to_i32(uint32_t raw)
{
    if (raw <= (uint32_t)INT32_MAX) {
        return (int32_t)raw;
    }
    if (raw == UINT32_C(0x80000000)) {
        return INT32_MIN;
    }
    return -(int32_t)(UINT32_MAX - raw + UINT32_C(1));
}

static bms_param_value_t raw_to_value(bms_param_type_t type, uint32_t raw)
{
    bms_param_value_t value;
    value.u32 = UINT32_C(0);
    switch (type) {
    case BMS_PARAM_I32:
        value.i32 = raw_to_i32(raw);
        break;
    case BMS_PARAM_U32:
        value.u32 = raw;
        break;
    case BMS_PARAM_BOOL:
        value.boolean = (uint8_t)raw;
        break;
    }
    return value;
}

bms_parameter_persist_result_t bms_parameter_persist_encode(
    const bms_param_descriptor_t *descriptors,
    const bms_param_value_t *values,
    size_t count,
    uint8_t *encoded,
    size_t encoded_capacity,
    size_t *encoded_length)
{
    size_t i;
    size_t persistent_count = 0U;
    size_t offset = BMS_PARAMETER_PERSIST_HEADER_SIZE;
    size_t required;

    if ((values == NULL) || (encoded == NULL) || (encoded_length == NULL)) {
        return BMS_PARAMETER_PERSIST_ERR_ARGUMENT;
    }
    *encoded_length = 0U;
    if (descriptor_table_valid(descriptors, count) == 0) {
        return BMS_PARAMETER_PERSIST_ERR_DESCRIPTOR;
    }

    for (i = 0U; i < count; ++i) {
        if (descriptors[i].persistent != 0U) {
            if (bms_parameter_validate(&descriptors[i], descriptors[i].type, values[i]) != BMS_PARAM_OK) {
                return BMS_PARAMETER_PERSIST_ERR_VALUE;
            }
            persistent_count++;
        }
    }
    if (persistent_count > (size_t)UINT16_MAX) {
        return BMS_PARAMETER_PERSIST_ERR_CAPACITY;
    }
    required = BMS_PARAMETER_PERSIST_HEADER_SIZE +
               (persistent_count * BMS_PARAMETER_PERSIST_ENTRY_SIZE);
    if (required > encoded_capacity) {
        return BMS_PARAMETER_PERSIST_ERR_CAPACITY;
    }

    put32(&encoded[0], BMS_PARAMETER_PERSIST_MAGIC);
    put16(&encoded[4], BMS_PARAMETER_PERSIST_FORMAT_VERSION);
    put16(&encoded[6], (uint16_t)persistent_count);

    for (i = 0U; i < count; ++i) {
        if (descriptors[i].persistent != 0U) {
            put16(&encoded[offset], descriptors[i].id);
            encoded[offset + 2U] = (uint8_t)descriptors[i].type;
            encoded[offset + 3U] = 0U;
            put32(&encoded[offset + 4U], value_to_raw(descriptors[i].type, values[i]));
            offset += BMS_PARAMETER_PERSIST_ENTRY_SIZE;
        }
    }
    *encoded_length = required;
    return BMS_PARAMETER_PERSIST_OK;
}

bms_parameter_persist_result_t bms_parameter_persist_restore(
    const bms_param_descriptor_t *descriptors,
    bms_param_value_t *active_values,
    bms_param_value_t *staged_values,
    size_t count,
    const uint8_t *encoded,
    size_t encoded_length,
    bms_param_cross_validate_fn cross_validate,
    void *context)
{
    uint16_t entry_count;
    size_t required;
    size_t i;

    if ((active_values == NULL) || (staged_values == NULL) || (encoded == NULL)) {
        return BMS_PARAMETER_PERSIST_ERR_ARGUMENT;
    }
    if (descriptor_table_valid(descriptors, count) == 0) {
        return BMS_PARAMETER_PERSIST_ERR_DESCRIPTOR;
    }
    if (encoded_length < BMS_PARAMETER_PERSIST_HEADER_SIZE) {
        return BMS_PARAMETER_PERSIST_ERR_FORMAT;
    }
    if ((get32(&encoded[0]) != BMS_PARAMETER_PERSIST_MAGIC) ||
        (get16(&encoded[4]) != BMS_PARAMETER_PERSIST_FORMAT_VERSION)) {
        return BMS_PARAMETER_PERSIST_ERR_FORMAT;
    }
    entry_count = get16(&encoded[6]);
    required = BMS_PARAMETER_PERSIST_HEADER_SIZE +
               ((size_t)entry_count * BMS_PARAMETER_PERSIST_ENTRY_SIZE);
    if (required != encoded_length) {
        return BMS_PARAMETER_PERSIST_ERR_FORMAT;
    }

    for (i = 0U; i < count; ++i) {
        staged_values[i] = active_values[i];
    }

    for (i = 0U; i < (size_t)entry_count; ++i) {
        const size_t offset = BMS_PARAMETER_PERSIST_HEADER_SIZE +
                              (i * BMS_PARAMETER_PERSIST_ENTRY_SIZE);
        const uint16_t id = get16(&encoded[offset]);
        const uint8_t type_byte = encoded[offset + 2U];
        const uint8_t reserved = encoded[offset + 3U];
        const uint32_t raw = get32(&encoded[offset + 4U]);
        size_t j;
        size_t descriptor_index;
        bms_param_type_t type;
        bms_param_value_t value;

        if ((id == 0U) || (reserved != 0U) || (type_byte > (uint8_t)BMS_PARAM_BOOL)) {
            return BMS_PARAMETER_PERSIST_ERR_FORMAT;
        }
        for (j = 0U; j < i; ++j) {
            const size_t previous = BMS_PARAMETER_PERSIST_HEADER_SIZE +
                                    (j * BMS_PARAMETER_PERSIST_ENTRY_SIZE);
            if (get16(&encoded[previous]) == id) {
                return BMS_PARAMETER_PERSIST_ERR_FORMAT;
            }
        }

        descriptor_index = descriptor_find(descriptors, count, id);
        if (descriptor_index == count) {
            continue;
        }
        if (descriptors[descriptor_index].persistent == 0U) {
            return BMS_PARAMETER_PERSIST_ERR_FORMAT;
        }
        type = (bms_param_type_t)type_byte;
        if (descriptors[descriptor_index].type != type) {
            return BMS_PARAMETER_PERSIST_ERR_FORMAT;
        }
        value = raw_to_value(type, raw);
        if (bms_parameter_validate(&descriptors[descriptor_index], type, value) != BMS_PARAM_OK) {
            return BMS_PARAMETER_PERSIST_ERR_VALUE;
        }
        staged_values[descriptor_index] = value;
    }

    if ((cross_validate != NULL) &&
        (cross_validate(descriptors, staged_values, count, context) != 0)) {
        return BMS_PARAMETER_PERSIST_ERR_CROSS_FIELD;
    }
    for (i = 0U; i < count; ++i) {
        active_values[i] = staged_values[i];
    }
    return BMS_PARAMETER_PERSIST_OK;
}
