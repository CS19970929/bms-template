#include "bms_platform.h"
#include "bms_target_config.h"
#include "stm32f0xx.h"
#include <string.h>

static int nvm_address(uint8_t slot, uint32_t offset, size_t length, uint32_t *address_out)
{
    uint32_t base;
    if (address_out == NULL) {
        return -1;
    }
    if (slot == 0U) {
        base = BMS_TARGET_NVM_A;
    } else if (slot == 1U) {
        base = BMS_TARGET_NVM_B;
    } else {
        return -1;
    }
    if (offset > BMS_TARGET_NVM_SLOT_SIZE) {
        return -1;
    }
    if (length > (size_t)(BMS_TARGET_NVM_SLOT_SIZE - offset)) {
        return -1;
    }
    *address_out = base + offset;
    return 0;
}

int bms_platform_nvm_read(void *ctx, uint8_t slot, uint32_t offset, uint8_t *dst, size_t length)
{
    uint32_t address;
    (void)ctx;
    if (nvm_address(slot, offset, length, &address) != 0) {
        return -1;
    }
    if (length == 0U) {
        return 0;
    }
    if (dst == NULL) {
        return -1;
    }
    (void)memcpy(dst, (const void *)(uintptr_t)address, length);
    return 0;
}

int bms_platform_nvm_erase(void *ctx, uint8_t slot)
{
    uint32_t address;
    FLASH_Status status;
    (void)ctx;
    if (nvm_address(slot, 0U, (size_t)BMS_TARGET_NVM_SLOT_SIZE, &address) != 0) {
        return -1;
    }
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    status = FLASH_ErasePage(address);
    FLASH_Lock();
    if (status != FLASH_COMPLETE) {
        return -1;
    }
    bms_platform_watchdog_reload();
    return 0;
}

int bms_platform_nvm_program(void *ctx, uint8_t slot, uint32_t offset,
                             const uint8_t *data, size_t length)
{
    uint32_t address;
    size_t i;
    (void)ctx;
    if (nvm_address(slot, offset, length, &address) != 0) {
        return -1;
    }
    if (length == 0U) {
        return 0;
    }
    if ((data == NULL) || ((address & UINT32_C(1)) != 0U)) {
        return -1;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    for (i = 0U; i < length; i += 2U) {
        const uint8_t high = ((i + 1U) < length) ? data[i + 1U] : UINT8_C(0xFF);
        const uint16_t value = (uint16_t)data[i] | ((uint16_t)high << 8U);
        const uint32_t target = address + (uint32_t)i;
        if (FLASH_ProgramHalfWord(target, value) != FLASH_COMPLETE) {
            FLASH_Lock();
            return -1;
        }
        if (*(const volatile uint16_t *)(uintptr_t)target != value) {
            FLASH_Lock();
            return -1;
        }
        bms_platform_watchdog_reload();
    }
    FLASH_Lock();
    return 0;
}

uint32_t bms_platform_nvm_slot_size(void)
{
    return BMS_TARGET_NVM_SLOT_SIZE;
}
