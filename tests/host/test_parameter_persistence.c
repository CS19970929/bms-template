#include "bms_parameter_persistence.h"
#include <stdint.h>

static void put16_test(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
}

static void put32_test(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
    dst[2] = (uint8_t)(value >> 16U);
    dst[3] = (uint8_t)(value >> 24U);
}

static void init_descriptors(bms_param_descriptor_t *descriptors)
{
    descriptors[0].id = 1U;
    descriptors[0].type = BMS_PARAM_I32;
    descriptors[0].write_access_mask = BMS_PARAM_ACCESS_USER;
    descriptors[0].persistent = 1U;
    descriptors[0].default_value.i32 = 0;
    descriptors[0].minimum.i32 = -100;
    descriptors[0].maximum.i32 = 100;

    descriptors[1].id = 2U;
    descriptors[1].type = BMS_PARAM_U32;
    descriptors[1].write_access_mask = BMS_PARAM_ACCESS_SERVICE;
    descriptors[1].persistent = 1U;
    descriptors[1].default_value.u32 = 100U;
    descriptors[1].minimum.u32 = 0U;
    descriptors[1].maximum.u32 = 1000U;

    descriptors[2].id = 3U;
    descriptors[2].type = BMS_PARAM_BOOL;
    descriptors[2].write_access_mask = BMS_PARAM_ACCESS_FACTORY;
    descriptors[2].persistent = 0U;
    descriptors[2].default_value.boolean = 0U;
    descriptors[2].minimum.boolean = 0U;
    descriptors[2].maximum.boolean = 1U;
}

int main(void)
{
    bms_param_descriptor_t descriptors[3] = {0};
    bms_param_value_t values[3] = {0};
    bms_param_value_t staged[3] = {0};
    uint8_t encoded[64] = {0};
    size_t encoded_length = 0U;

    init_descriptors(descriptors);
    values[0].i32 = -20;
    values[1].u32 = 500U;
    values[2].boolean = 1U;

    if (bms_parameter_persist_encode(descriptors, values, 3U, encoded, sizeof(encoded),
                                     &encoded_length) != BMS_PARAMETER_PERSIST_OK) {
        return 1;
    }
    if (encoded_length != 24U) {
        return 2;
    }

    values[0].i32 = 7;
    values[1].u32 = 8U;
    values[2].boolean = 0U;
    if (bms_parameter_persist_restore(descriptors, values, staged, 3U, encoded, encoded_length,
                                      NULL, NULL) != BMS_PARAMETER_PERSIST_OK) {
        return 3;
    }
    if ((values[0].i32 != -20) || (values[1].u32 != 500U) || (values[2].boolean != 0U)) {
        return 4;
    }

    values[0].i32 = 11;
    values[1].u32 = 12U;
    put16_test(&encoded[8], 999U);
    if (bms_parameter_persist_restore(descriptors, values, staged, 3U, encoded, encoded_length,
                                      NULL, NULL) != BMS_PARAMETER_PERSIST_OK) {
        return 5;
    }
    if ((values[0].i32 != 11) || (values[1].u32 != 500U)) {
        return 6;
    }

    put16_test(&encoded[8], 1U);
    put32_test(&encoded[20], 2000U);
    values[0].i32 = 21;
    values[1].u32 = 22U;
    if (bms_parameter_persist_restore(descriptors, values, staged, 3U, encoded, encoded_length,
                                      NULL, NULL) != BMS_PARAMETER_PERSIST_ERR_VALUE) {
        return 7;
    }
    if ((values[0].i32 != 21) || (values[1].u32 != 22U)) {
        return 8;
    }

    put32_test(&encoded[20], 500U);
    put16_test(&encoded[16], 1U);
    if (bms_parameter_persist_restore(descriptors, values, staged, 3U, encoded, encoded_length,
                                      NULL, NULL) != BMS_PARAMETER_PERSIST_ERR_FORMAT) {
        return 9;
    }

    put16_test(&encoded[16], 2U);
    values[1].u32 = 2000U;
    if (bms_parameter_persist_encode(descriptors, values, 3U, encoded, sizeof(encoded),
                                     &encoded_length) != BMS_PARAMETER_PERSIST_ERR_VALUE) {
        return 10;
    }
    return 0;
}
