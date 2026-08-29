#include "bms_parameter_store.h"
#include <stdint.h>
#include <string.h>

#define SLOT_SIZE 128U
#define PAYLOAD_LIMIT 64U

typedef struct {
    uint8_t bytes[2][SLOT_SIZE];
} fake_flash_t;

static int fake_read(void *context, uint8_t slot, uint32_t offset, uint8_t *data, size_t length)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    if ((flash == NULL) || (data == NULL) || (slot >= 2U) ||
        (offset > SLOT_SIZE) || (length > (size_t)(SLOT_SIZE - offset))) {
        return -1;
    }
    (void)memcpy(data, &flash->bytes[slot][offset], length);
    return 0;
}

static int fake_erase(void *context, uint8_t slot)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    if ((flash == NULL) || (slot >= 2U)) {
        return -1;
    }
    (void)memset(flash->bytes[slot], 0xFF, SLOT_SIZE);
    return 0;
}

static int fake_program(void *context, uint8_t slot, uint32_t offset,
                        const uint8_t *data, size_t length)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    size_t i;
    if ((flash == NULL) || (data == NULL) || (slot >= 2U) ||
        (offset > SLOT_SIZE) || (length > (size_t)(SLOT_SIZE - offset))) {
        return -1;
    }
    for (i = 0U; i < length; ++i) {
        const uint8_t old_value = flash->bytes[slot][offset + i];
        if ((old_value & data[i]) != data[i]) {
            return -1;
        }
        flash->bytes[slot][offset + i] = data[i];
    }
    return 0;
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
    descriptors[1].default_value.u32 = 10U;
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
    fake_flash_t flash;
    bms_nvm_store_io_t io;
    bms_param_descriptor_t descriptors[3] = {0};
    bms_param_value_t values[3] = {0};
    bms_param_value_t staged[3] = {0};
    bms_nvm_store_info_t info;
    uint8_t payload[PAYLOAD_LIMIT];
    uint8_t scratch[PAYLOAD_LIMIT];

    (void)memset(&flash, 0xFF, sizeof(flash));
    io.context = &flash;
    io.slot_size = SLOT_SIZE;
    io.read = fake_read;
    io.erase = fake_erase;
    io.program = fake_program;
    init_descriptors(descriptors);
    bms_parameter_load_defaults(descriptors, values, 3U);

    if (bms_parameter_store_load(&io, PAYLOAD_LIMIT, descriptors, values, staged, 3U,
                                 payload, sizeof(payload), scratch, sizeof(scratch),
                                 NULL, NULL, &info) != BMS_PARAMETER_STORE_NOT_FOUND) {
        return 1;
    }

    values[0].i32 = -25;
    values[1].u32 = 500U;
    values[2].boolean = 1U;
    if (bms_parameter_store_commit(&io, PAYLOAD_LIMIT, descriptors, values, 3U,
                                   payload, sizeof(payload), scratch, sizeof(scratch),
                                   &info) != BMS_PARAMETER_STORE_OK) {
        return 2;
    }
    if ((info.valid == 0U) || (info.sequence != 1U)) {
        return 3;
    }

    values[0].i32 = 5;
    values[1].u32 = 6U;
    values[2].boolean = 0U;
    if (bms_parameter_store_load(&io, PAYLOAD_LIMIT, descriptors, values, staged, 3U,
                                 payload, sizeof(payload), scratch, sizeof(scratch),
                                 NULL, NULL, &info) != BMS_PARAMETER_STORE_OK) {
        return 4;
    }
    if ((values[0].i32 != -25) || (values[1].u32 != 500U) || (values[2].boolean != 0U)) {
        return 5;
    }

    values[0].i32 = 30;
    values[1].u32 = 600U;
    if (bms_parameter_store_commit(&io, PAYLOAD_LIMIT, descriptors, values, 3U,
                                   payload, sizeof(payload), scratch, sizeof(scratch),
                                   &info) != BMS_PARAMETER_STORE_OK) {
        return 6;
    }
    if (info.sequence != 2U) {
        return 7;
    }

    flash.bytes[info.slot][24U + 4U] ^= 0x01U;
    values[0].i32 = 31;
    values[1].u32 = 601U;
    if (bms_parameter_store_load(&io, PAYLOAD_LIMIT, descriptors, values, staged, 3U,
                                 payload, sizeof(payload), scratch, sizeof(scratch),
                                 NULL, NULL, &info) != BMS_PARAMETER_STORE_OK) {
        return 8;
    }
    if ((values[0].i32 != -25) || (values[1].u32 != 500U)) {
        return 9;
    }
    return 0;
}
