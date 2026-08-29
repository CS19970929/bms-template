#include "bms_protection_manager.h"

static int rule_config_valid(const bms_protection_rule_t *rule)
{
    if ((rule->detector.enabled > 1U) || (rule->latch_enabled > 1U)) {
        return 0;
    }
    switch (rule->detector.mode) {
    case BMS_PROTECT_MODE_HIGH:
        return (rule->detector.release_threshold <= rule->detector.trip_threshold) ? 1 : 0;
    case BMS_PROTECT_MODE_LOW:
        return (rule->detector.release_threshold >= rule->detector.trip_threshold) ? 1 : 0;
    }
    return 0;
}

static int detector_blocks(bms_protect_state_t state)
{
    return ((state == BMS_PROTECT_ACTIVE) || (state == BMS_PROTECT_RECOVERING)) ? 1 : 0;
}

static void output_reset(bms_protection_manager_output_t *output)
{
    output->charge_block_mask = 0U;
    output->discharge_block_mask = 0U;
    output->active_count = 0U;
    output->first_active_id = 0U;
    output->any_active = 0U;
}

void bms_protection_manager_init(bms_protection_runtime_t *runtime, size_t count)
{
    size_t i;
    if ((runtime == NULL) && (count != 0U)) {
        return;
    }
    for (i = 0U; i < count; ++i) {
        bms_protection_init(&runtime[i].detector);
        runtime[i].latched = 0U;
    }
}

bms_protection_manager_result_t bms_protection_manager_validate_rules(const bms_protection_rule_t *rules,
                                                                       size_t count)
{
    size_t i;
    size_t j;
    if ((rules == NULL) && (count != 0U)) {
        return BMS_PROTECTION_MANAGER_ERR_ARGUMENT;
    }
    for (i = 0U; i < count; ++i) {
        if (rule_config_valid(&rules[i]) == 0) {
            return BMS_PROTECTION_MANAGER_ERR_CONFIG;
        }
        for (j = i + 1U; j < count; ++j) {
            if (rules[i].id == rules[j].id) {
                return BMS_PROTECTION_MANAGER_ERR_DUPLICATE_ID;
            }
        }
    }
    return BMS_PROTECTION_MANAGER_OK;
}

bms_protection_manager_result_t bms_protection_manager_step(const bms_protection_rule_t *rules,
                                                            bms_protection_runtime_t *runtime,
                                                            const int32_t *values,
                                                            size_t count,
                                                            uint32_t elapsed_ms,
                                                            bms_protection_manager_output_t *output)
{
    size_t i;
    if (output == NULL) {
        return BMS_PROTECTION_MANAGER_ERR_ARGUMENT;
    }
    output_reset(output);
    if (count == 0U) {
        return BMS_PROTECTION_MANAGER_OK;
    }
    if ((rules == NULL) || (runtime == NULL) || (values == NULL)) {
        return BMS_PROTECTION_MANAGER_ERR_ARGUMENT;
    }

    for (i = 0U; i < count; ++i) {
        bms_protect_state_t state;
        int active;
        if (rule_config_valid(&rules[i]) == 0) {
            output_reset(output);
            return BMS_PROTECTION_MANAGER_ERR_CONFIG;
        }
        state = bms_protection_step(&runtime[i].detector, &rules[i].detector, values[i], elapsed_ms);
        if (rules[i].detector.enabled == 0U) {
            runtime[i].latched = 0U;
        } else if ((rules[i].latch_enabled != 0U) && (state == BMS_PROTECT_ACTIVE)) {
            runtime[i].latched = 1U;
        }

        active = (detector_blocks(state) != 0) || (runtime[i].latched != 0U);
        if (active != 0) {
            if (output->any_active == 0U) {
                output->first_active_id = rules[i].id;
                output->any_active = 1U;
            }
            output->active_count++;
            output->charge_block_mask |= rules[i].charge_block_mask;
            output->discharge_block_mask |= rules[i].discharge_block_mask;
        }
    }
    return BMS_PROTECTION_MANAGER_OK;
}

bms_protection_manager_result_t bms_protection_manager_clear_latch(const bms_protection_rule_t *rules,
                                                                   bms_protection_runtime_t *runtime,
                                                                   size_t count,
                                                                   uint16_t id)
{
    size_t i;
    if (((rules == NULL) || (runtime == NULL)) && (count != 0U)) {
        return BMS_PROTECTION_MANAGER_ERR_ARGUMENT;
    }
    for (i = 0U; i < count; ++i) {
        if (rules[i].id == id) {
            if (runtime[i].detector.state != BMS_PROTECT_NORMAL) {
                return BMS_PROTECTION_MANAGER_ERR_CONDITION_ACTIVE;
            }
            runtime[i].latched = 0U;
            return BMS_PROTECTION_MANAGER_OK;
        }
    }
    return BMS_PROTECTION_MANAGER_ERR_NOT_FOUND;
}
