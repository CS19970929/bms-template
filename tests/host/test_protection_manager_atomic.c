#include "bms_protection_manager.h"
#include <stdint.h>

int main(void)
{
    bms_protection_rule_t rules[2] = {0};
    bms_protection_runtime_t runtime[2];
    bms_protection_manager_output_t output;
    const int32_t values[2] = {4300, 100};

    rules[0].id = 1U;
    rules[0].detector.trip_threshold = 4200;
    rules[0].detector.release_threshold = 4100;
    rules[0].detector.trip_delay_ms = 100U;
    rules[0].detector.release_delay_ms = 100U;
    rules[0].detector.mode = BMS_PROTECT_MODE_HIGH;
    rules[0].detector.enabled = 1U;

    rules[1].id = 2U;
    rules[1].detector.trip_threshold = 100;
    rules[1].detector.release_threshold = 200;
    rules[1].detector.mode = BMS_PROTECT_MODE_HIGH;
    rules[1].detector.enabled = 1U;

    bms_protection_manager_init(runtime, 2U);
    if (bms_protection_manager_step(rules, runtime, values, 2U, 50U, &output) !=
        BMS_PROTECTION_MANAGER_ERR_CONFIG) {
        return 1;
    }
    if ((runtime[0].detector.state != BMS_PROTECT_NORMAL) ||
        (runtime[0].detector.elapsed_ms != 0U) ||
        (runtime[0].latched != 0U)) {
        return 2;
    }
    if ((output.any_active != 0U) || (output.charge_block_mask != 0U) ||
        (output.discharge_block_mask != 0U)) {
        return 3;
    }
    return 0;
}
