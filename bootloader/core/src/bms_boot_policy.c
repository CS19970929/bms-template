#include "bms_boot_policy.h"
#include <stddef.h>

bms_boot_action_t bms_boot_policy_decide(const bms_boot_policy_input_t *input)
{
    if (input == NULL) {
        return BMS_BOOT_ENTER_RECOVERY;
    }
    if ((input->app_valid == 0U) || (input->upgrade_requested != 0U) || (input->recovery_pin_active != 0U)) {
        return BMS_BOOT_ENTER_RECOVERY;
    }
    if ((input->boot_failure_limit != 0U) && (input->consecutive_boot_failures >= input->boot_failure_limit)) {
        return BMS_BOOT_ENTER_RECOVERY;
    }
    if ((input->reset_reason == BMS_RESET_WATCHDOG) && (input->app_confirmed_healthy == 0U)) {
        return BMS_BOOT_ENTER_RECOVERY;
    }
    return BMS_BOOT_START_APP;
}
