#include "bms_boot_image.h"
#include "bms_boot_metadata.h"
#include "bms_boot_policy.h"
#include "bms_platform_stm32f0.h"
#include <stdint.h>

#define PRODUCT_ID 42U

static bms_image_constraints_t constraints(void)
{
    bms_image_constraints_t c;
    c.app_start = BMS_F030_APP_START; c.app_end_exclusive = BMS_F030_APP_END;
    c.ram_start = BMS_F030_RAM_START; c.ram_end_exclusive = BMS_F030_RAM_END;
    c.mcu_id = BMS_MCU_STM32F030C8; c.product_id = PRODUCT_ID;
    return c;
}

int main(void)
{
    const bms_boot_meta_record_t *a = (const bms_boot_meta_record_t *)(uintptr_t)BMS_F030_META_A;
    const bms_boot_meta_record_t *b = (const bms_boot_meta_record_t *)(uintptr_t)BMS_F030_META_B;
    const bms_boot_meta_record_t *meta;
    bms_boot_policy_input_t policy = {0};
    bms_image_constraints_t c = constraints();
    static const uint8_t recovery_msg[] = "BMS-BOOT RECOVERY\r\n";

    bms_platform_uart_init(115200U);
    bms_platform_watchdog_start();
    meta = bms_boot_metadata_select(a, b);
    if ((meta != NULL) && ((meta->state == (uint32_t)BMS_BOOT_META_READY) || (meta->state == (uint32_t)BMS_BOOT_META_CONFIRMED))) {
        policy.app_valid = (bms_boot_image_validate(&meta->image, &c, bms_platform_flash_read, NULL) == BMS_IMAGE_OK) ? 1U : 0U;
    }
    policy.app_confirmed_healthy = (meta != NULL && meta->state == (uint32_t)BMS_BOOT_META_CONFIRMED) ? 1U : 0U;
    policy.boot_failure_limit = 3U;
    if (bms_boot_policy_decide(&policy) == BMS_BOOT_START_APP) bms_platform_jump_to_app(BMS_F030_APP_START);

    bms_platform_uart_write(recovery_msg, sizeof(recovery_msg) - 1U);
    for (;;) {
        bms_platform_watchdog_reload();
    }
}
