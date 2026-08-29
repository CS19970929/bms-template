#include "bms_nvm_store.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define TEST_SLOT_SIZE 128U
#define TEST_PAYLOAD_MAX 32U

typedef struct {
    uint8_t bytes[BMS_NVM_STORE_SLOT_COUNT][TEST_SLOT_SIZE];
    uint32_t mutation_count;
    uint32_t fail_after_mutation;
} fake_flash_t;

static int fake_read(void *context, uint8_t slot, uint32_t offset, uint8_t *data, size_t length)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    if ((slot >= BMS_NVM_STORE_SLOT_COUNT) || (offset > TEST_SLOT_SIZE) ||
        (length > (size_t)(TEST_SLOT_SIZE - offset))) {
        return -1;
    }
    memcpy(data, &flash->bytes[slot][offset], length);
    return 0;
}

static int fake_erase(void *context, uint8_t slot)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    if (slot >= BMS_NVM_STORE_SLOT_COUNT) {
        return -1;
    }
    memset(flash->bytes[slot], 0xFF, TEST_SLOT_SIZE);
    flash->mutation_count++;
    return (flash->mutation_count == flash->fail_after_mutation) ? -1 : 0;
}

static int fake_program(void *context, uint8_t slot, uint32_t offset, const uint8_t *data, size_t length)
{
    fake_flash_t *flash = (fake_flash_t *)context;
    size_t i;
    if ((slot >= BMS_NVM_STORE_SLOT_COUNT) || (data == NULL) || (offset > TEST_SLOT_SIZE) ||
        (length > (size_t)(TEST_SLOT_SIZE - offset))) {
        return -1;
    }
    for (i = 0U; i < length; ++i) {
        const uint32_t index = offset + (uint32_t)i;
        if ((uint8_t)(flash->bytes[slot][index] & data[i]) != data[i]) {
            return -1;
        }
    }
    for (i = 0U; i < length; ++i) {
        const uint32_t index = offset + (uint32_t)i;
        flash->bytes[slot][index] &= data[i];
    }
    flash->mutation_count++;
    return (flash->mutation_count == flash->fail_after_mutation) ? -1 : 0;
}

static void fake_init(fake_flash_t *flash)
{
    memset(flash, 0xFF, sizeof(*flash));
    flash->mutation_count = 0U;
    flash->fail_after_mutation = 0U;
}

static int payload_is(const uint8_t *actual, const uint8_t *expected, size_t length)
{
    return (memcmp(actual, expected, length) == 0) ? 1 : 0;
}

int main(void)
{
    const uint8_t old_payload[] = {1U, 2U, 3U, 4U, 5U, 6U};
    const uint8_t new_payload[] = {9U, 8U, 7U, 6U};
    fake_flash_t flash;
    fake_flash_t baseline;
    bms_nvm_store_io_t io;
    bms_nvm_store_info_t info;
    uint8_t scratch[TEST_PAYLOAD_MAX];
    uint8_t loaded[TEST_PAYLOAD_MAX];
    uint32_t fault_step;

    fake_init(&flash);
    io.context = &flash;
    io.slot_size = TEST_SLOT_SIZE;
    io.read = fake_read;
    io.erase = fake_erase;
    io.program = fake_program;

    if (bms_nvm_store_commit(&io, 1U, old_payload, sizeof(old_payload), scratch, sizeof(scratch), &info) !=
        BMS_NVM_STORE_OK) {
        return 1;
    }
    if (bms_nvm_store_load(&io, 1U, TEST_PAYLOAD_MAX, loaded, sizeof(loaded), scratch,
                           sizeof(scratch), &info) != BMS_NVM_STORE_OK) {
        return 2;
    }
    if ((info.sequence != 1U) || (info.payload_length != sizeof(old_payload)) ||
        (payload_is(loaded, old_payload, sizeof(old_payload)) == 0)) {
        return 3;
    }
    baseline = flash;

    for (fault_step = 1U; fault_step <= 3U; ++fault_step) {
        flash = baseline;
        flash.mutation_count = 0U;
        flash.fail_after_mutation = fault_step;
        if (bms_nvm_store_commit(&io, 1U, new_payload, sizeof(new_payload), scratch, sizeof(scratch), &info) !=
            BMS_NVM_STORE_ERR_IO) {
            return 10 + (int)fault_step;
        }
        flash.fail_after_mutation = 0U;
        if (bms_nvm_store_load(&io, 1U, TEST_PAYLOAD_MAX, loaded, sizeof(loaded), scratch,
                               sizeof(scratch), &info) != BMS_NVM_STORE_OK) {
            return 20 + (int)fault_step;
        }
        if ((info.payload_length == sizeof(old_payload)) &&
            (payload_is(loaded, old_payload, sizeof(old_payload)) != 0)) {
            continue;
        }
        if ((info.payload_length == sizeof(new_payload)) &&
            (payload_is(loaded, new_payload, sizeof(new_payload)) != 0)) {
            continue;
        }
        return 30 + (int)fault_step;
    }

    flash = baseline;
    flash.mutation_count = 0U;
    flash.fail_after_mutation = 0U;
    if (bms_nvm_store_commit(&io, 1U, new_payload, sizeof(new_payload), scratch, sizeof(scratch), &info) !=
        BMS_NVM_STORE_OK) {
        return 40;
    }
    if (bms_nvm_store_load(&io, 1U, TEST_PAYLOAD_MAX, loaded, sizeof(loaded), scratch,
                           sizeof(scratch), &info) != BMS_NVM_STORE_OK) {
        return 41;
    }
    if ((info.sequence != 2U) || (info.payload_length != sizeof(new_payload)) ||
        (payload_is(loaded, new_payload, sizeof(new_payload)) == 0)) {
        return 42;
    }
    return 0;
}
