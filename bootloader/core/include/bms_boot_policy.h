#ifndef BMS_BOOT_POLICY_H
#define BMS_BOOT_POLICY_H

#include <stdint.h>

typedef enum {
    BMS_RESET_UNKNOWN = 0,
    BMS_RESET_POWER_ON,
    BMS_RESET_SOFTWARE,
    BMS_RESET_WATCHDOG,
    BMS_RESET_BROWNOUT
} bms_reset_reason_t;

typedef struct {
    uint8_t app_valid;
    uint8_t upgrade_requested;
    uint8_t recovery_pin_active;
    uint8_t app_confirmed_healthy;
    uint8_t consecutive_boot_failures;
    uint8_t boot_failure_limit;
    bms_reset_reason_t reset_reason;
} bms_boot_policy_input_t;

typedef enum {
    BMS_BOOT_ENTER_RECOVERY = 0,
    BMS_BOOT_START_APP = 1
} bms_boot_action_t;

bms_boot_action_t bms_boot_policy_decide(const bms_boot_policy_input_t *input);

#endif
