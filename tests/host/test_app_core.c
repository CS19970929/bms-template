#include "bms_mos_policy.h"
#include "bms_nvm_record.h"
#include "bms_parameter.h"
#include "bms_protection_manager.h"
#include "bms_scheduler.h"
#include "bms_soc.h"
#include "bms_state_machine.h"
#include "bms_watchdog_supervisor.h"
#include <stdint.h>

int bms_test_app_core(void);
#define T(x) do { if (!(x)) return __LINE__; } while (0)

static void count_task(void *context, uint32_t now_ms)
{
    uint32_t *count = (uint32_t *)context;
    (void)now_ms;
    (*count)++;
}

static int cross_validate_limits(const bms_param_descriptor_t *descriptors,
                                 const bms_param_value_t *values,
                                 size_t count,
                                 void *context)
{
    (void)descriptors;
    (void)context;
    if (count < 2U) {
        return -1;
    }
    return (values[0].u32 < values[1].u32) ? 0 : -1;
}

static int test_state_machine(void)
{
    bms_state_machine_t state;
    bms_state_machine_init(&state);
    T(bms_state_machine_get(&state) == BMS_STATE_INIT);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_INIT_OK) == BMS_STATE_OK);
    T(state.current == BMS_STATE_IDLE);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_CHARGE_ACTIVITY) == BMS_STATE_OK);
    T(state.current == BMS_STATE_CHARGING);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_PROTECTION_ACTIVE) == BMS_STATE_OK);
    T(state.current == BMS_STATE_PROTECTED);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_CHARGE_ACTIVITY) == BMS_STATE_ERR_TRANSITION);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_PROTECTION_CLEARED) == BMS_STATE_OK);
    T(state.current == BMS_STATE_CHARGING);
    T(bms_state_machine_dispatch(&state, BMS_STATE_EVENT_FATAL_FAULT) == BMS_STATE_OK);
    T(state.current == BMS_STATE_FAULT);
    return 0;
}

static int test_parameters(void)
{
    bms_param_descriptor_t descriptors[2] = {0};
    bms_param_value_t active[2] = {0};
    bms_param_value_t staged[2] = {0};
    bms_param_value_t value = {0};
    bms_param_transaction_t transaction = {0};

    descriptors[0].id = 1U;
    descriptors[0].type = BMS_PARAM_U32;
    descriptors[0].write_access_mask = BMS_PARAM_ACCESS_USER;
    descriptors[0].default_value.u32 = 100U;
    descriptors[0].minimum.u32 = 50U;
    descriptors[0].maximum.u32 = 200U;
    descriptors[1].id = 2U;
    descriptors[1].type = BMS_PARAM_U32;
    descriptors[1].write_access_mask = BMS_PARAM_ACCESS_SERVICE;
    descriptors[1].default_value.u32 = 150U;
    descriptors[1].minimum.u32 = 100U;
    descriptors[1].maximum.u32 = 250U;

    bms_parameter_load_defaults(descriptors, active, 2U);
    T(active[0].u32 == 100U);
    T(active[1].u32 == 150U);
    T(bms_parameter_transaction_begin(&transaction, descriptors, active, staged, 2U) == BMS_PARAM_OK);
    value.u32 = 120U;
    T(bms_parameter_transaction_set(&transaction, 1U, BMS_PARAM_U32, value, BMS_PARAM_ACCESS_USER) == BMS_PARAM_OK);
    value.u32 = 90U;
    T(bms_parameter_transaction_set(&transaction, 2U, BMS_PARAM_U32, value, BMS_PARAM_ACCESS_SERVICE) == BMS_PARAM_ERR_RANGE);
    value.u32 = 180U;
    T(bms_parameter_transaction_set(&transaction, 2U, BMS_PARAM_U32, value, BMS_PARAM_ACCESS_USER) == BMS_PARAM_ERR_PERMISSION);
    T(bms_parameter_transaction_set(&transaction, 2U, BMS_PARAM_U32, value, BMS_PARAM_ACCESS_SERVICE) == BMS_PARAM_OK);
    T(bms_parameter_transaction_commit(&transaction, active, cross_validate_limits, NULL) == BMS_PARAM_OK);
    T(active[0].u32 == 120U);
    T(active[1].u32 == 180U);

    T(bms_parameter_transaction_begin(&transaction, descriptors, active, staged, 2U) == BMS_PARAM_OK);
    value.u32 = 190U;
    T(bms_parameter_transaction_set(&transaction, 1U, BMS_PARAM_U32, value, BMS_PARAM_ACCESS_USER) == BMS_PARAM_OK);
    T(bms_parameter_transaction_commit(&transaction, active, cross_validate_limits, NULL) == BMS_PARAM_ERR_CROSS_FIELD);
    T(active[0].u32 == 120U);
    bms_parameter_transaction_abort(&transaction);
    return 0;
}

static int test_nvm_records(void)
{
    const uint8_t payload_a[] = {1U, 2U, 3U};
    uint8_t payload_b[] = {4U, 5U, 6U};
    bms_nvm_record_header_t a;
    bms_nvm_record_header_t b;
    const bms_nvm_record_header_t *selected;

    T(bms_nvm_record_prepare(&a, 1U, 10U, payload_a, sizeof(payload_a)) == BMS_NVM_OK);
    T(bms_nvm_record_prepare(&b, 1U, 11U, payload_b, sizeof(payload_b)) == BMS_NVM_OK);
    selected = bms_nvm_record_select(&a, payload_a, sizeof(payload_a), &b, payload_b, sizeof(payload_b), 1U, 16U);
    T(selected == &b);
    payload_b[1] ^= 0x01U;
    selected = bms_nvm_record_select(&a, payload_a, sizeof(payload_a), &b, payload_b, sizeof(payload_b), 1U, 16U);
    T(selected == &a);
    T(bms_nvm_sequence_is_newer(0U, UINT32_MAX) != 0);
    return 0;
}

static int test_soc(void)
{
    bms_soc_config_t config = {0};
    bms_soc_t soc;
    config.nominal_capacity_mah = 10U;
    config.integration_period_ms = 200U;
    config.current_floor_ma = 200U;
    config.rest_current_threshold_ma = 100U;
    config.rest_required_ms = 400U;
    config.display_step_permille = 20U;
    config.ocv_correction_step_permille = 10U;
    config.forbid_rest_upward_correction = 1U;

    T(bms_soc_init(&soc, &config, 500U) == BMS_SOC_OK);
    T(bms_soc_step(&soc, &config, 36000, 200U) == BMS_SOC_OK);
    T(soc.soc_est_permille == 700U);
    T(soc.soc_display_permille == 520U);
    T(bms_soc_step(&soc, &config, 0, 200U) == BMS_SOC_OK);
    T(bms_soc_step(&soc, &config, 0, 200U) == BMS_SOC_OK);
    T(bms_soc_apply_ocv_target(&soc, &config, 800U) == BMS_SOC_OK_NO_CHANGE);
    T(bms_soc_apply_ocv_target(&soc, &config, 600U) == BMS_SOC_OK);
    T(soc.soc_est_permille == 690U);
    bms_soc_anchor_full(&soc);
    T((soc.soc_est_permille == 1000U) && (soc.soc_display_permille == 1000U));
    bms_soc_anchor_empty(&soc);
    T((soc.soc_est_permille == 0U) && (soc.soc_display_permille == 0U));
    return 0;
}

static int test_protection_manager(void)
{
    bms_protection_rule_t rules[2] = {0};
    bms_protection_runtime_t runtime[2];
    int32_t values[2] = {0};
    bms_protection_manager_output_t output;

    rules[0].id = 1U;
    rules[0].detector.trip_threshold = 4200;
    rules[0].detector.release_threshold = 4100;
    rules[0].detector.trip_delay_ms = 100U;
    rules[0].detector.release_delay_ms = 100U;
    rules[0].detector.mode = BMS_PROTECT_MODE_HIGH;
    rules[0].detector.enabled = 1U;
    rules[0].charge_block_mask = 0x01U;

    rules[1].id = 2U;
    rules[1].detector.trip_threshold = 100;
    rules[1].detector.release_threshold = 50;
    rules[1].detector.trip_delay_ms = 0U;
    rules[1].detector.release_delay_ms = 0U;
    rules[1].detector.mode = BMS_PROTECT_MODE_HIGH;
    rules[1].detector.enabled = 1U;
    rules[1].charge_block_mask = 0x02U;
    rules[1].discharge_block_mask = 0x02U;
    rules[1].latch_enabled = 1U;

    T(bms_protection_manager_validate_rules(rules, 2U) == BMS_PROTECTION_MANAGER_OK);
    bms_protection_manager_init(runtime, 2U);

    values[0] = 4300;
    values[1] = 0;
    T(bms_protection_manager_step(rules, runtime, values, 2U, 50U, &output) == BMS_PROTECTION_MANAGER_OK);
    T(output.any_active == 0U);
    T(bms_protection_manager_step(rules, runtime, values, 2U, 50U, &output) == BMS_PROTECTION_MANAGER_OK);
    T(output.charge_block_mask == 0x01U);
    T(output.discharge_block_mask == 0U);
    T((output.any_active == 1U) && (output.active_count == 1U) && (output.first_active_id == 1U));

    values[0] = 4000;
    T(bms_protection_manager_step(rules, runtime, values, 2U, 100U, &output) == BMS_PROTECTION_MANAGER_OK);
    T(output.any_active == 0U);

    values[1] = 200;
    T(bms_protection_manager_step(rules, runtime, values, 2U, 0U, &output) == BMS_PROTECTION_MANAGER_OK);
    T((output.charge_block_mask == 0x02U) && (output.discharge_block_mask == 0x02U));
    T(bms_protection_manager_clear_latch(rules, runtime, 2U, 2U) == BMS_PROTECTION_MANAGER_ERR_CONDITION_ACTIVE);

    values[1] = 0;
    T(bms_protection_manager_step(rules, runtime, values, 2U, 0U, &output) == BMS_PROTECTION_MANAGER_OK);
    T((output.charge_block_mask == 0x02U) && (runtime[1].detector.state == BMS_PROTECT_NORMAL));
    T(bms_protection_manager_clear_latch(rules, runtime, 2U, 2U) == BMS_PROTECTION_MANAGER_OK);
    T(bms_protection_manager_step(rules, runtime, values, 2U, 0U, &output) == BMS_PROTECTION_MANAGER_OK);
    T(output.any_active == 0U);

    rules[1].id = 1U;
    T(bms_protection_manager_validate_rules(rules, 2U) == BMS_PROTECTION_MANAGER_ERR_DUPLICATE_ID);
    rules[1].id = 2U;
    rules[0].detector.release_threshold = 4300;
    T(bms_protection_manager_validate_rules(rules, 2U) == BMS_PROTECTION_MANAGER_ERR_CONFIG);
    return 0;
}

int bms_test_app_core(void)
{
    bms_watchdog_supervisor_t w;
    bms_mos_policy_input_t mi = {0};
    bms_mos_policy_output_t mo;
    bms_scheduler_t scheduler;
    uint32_t count = 0U;

    bms_watchdog_supervisor_init(&w, 0x07U);
    bms_watchdog_supervisor_heartbeat(&w, 0x01U);
    bms_watchdog_supervisor_heartbeat(&w, 0x02U);
    T(bms_watchdog_supervisor_close_window(&w) == 0);
    T(w.failed_windows == 1U);
    bms_watchdog_supervisor_heartbeat(&w, 0x07U);
    T(bms_watchdog_supervisor_close_window(&w) == 1);

    mi.charge_requested = 1U;
    mi.discharge_requested = 1U;
    mi.topology = BMS_MOS_TOPOLOGY_COMMON_PORT;
    bms_mos_policy_evaluate(&mi, &mo);
    T(mo.charge_enable == 1U);
    T(mo.discharge_enable == 1U);
    mi.charge_block_mask = 0x02U;
    bms_mos_policy_evaluate(&mi, &mo);
    T(mo.charge_enable == 0U);
    T(mo.discharge_enable == 1U);
    mi.hardware_block_mask = 0x80U;
    bms_mos_policy_evaluate(&mi, &mo);
    T(mo.charge_enable == 0U);
    T(mo.discharge_enable == 0U);

    bms_scheduler_init(&scheduler);
    T(bms_scheduler_add(&scheduler, count_task, &count, 10U, 5U) == 0);
    bms_scheduler_run_due(&scheduler, 4U);
    T(count == 0U);
    bms_scheduler_run_due(&scheduler, 5U);
    T(count == 1U);
    bms_scheduler_run_due(&scheduler, 14U);
    T(count == 1U);
    bms_scheduler_run_due(&scheduler, 15U);
    T(count == 2U);

    T(test_state_machine() == 0);
    T(test_parameters() == 0);
    T(test_nvm_records() == 0);
    T(test_soc() == 0);
    T(test_protection_manager() == 0);
    return 0;
}
