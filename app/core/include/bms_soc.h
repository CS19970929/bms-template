#ifndef BMS_SOC_H
#define BMS_SOC_H

#include <stdint.h>

typedef struct {
    uint32_t nominal_capacity_mah;
    uint32_t integration_period_ms;
    uint32_t current_floor_ma;
    uint32_t rest_current_threshold_ma;
    uint32_t rest_required_ms;
    uint16_t display_step_permille;
    uint16_t ocv_correction_step_permille;
    uint8_t forbid_rest_upward_correction;
} bms_soc_config_t;

typedef struct {
    int64_t capacity_mas;
    int64_t charge_mas;
    int64_t integration_remainder_mams;
    uint32_t rest_elapsed_ms;
    uint16_t soc_est_permille;
    uint16_t soc_display_permille;
    uint8_t valid;
} bms_soc_t;

typedef enum {
    BMS_SOC_OK = 0,
    BMS_SOC_OK_NO_CHANGE = 1,
    BMS_SOC_ERR_ARGUMENT = -1,
    BMS_SOC_ERR_CONFIG = -2,
    BMS_SOC_ERR_PERIOD = -3,
    BMS_SOC_ERR_NOT_RESTED = -4
} bms_soc_result_t;

bms_soc_result_t bms_soc_init(bms_soc_t *soc, const bms_soc_config_t *config, uint16_t initial_permille);
bms_soc_result_t bms_soc_step(bms_soc_t *soc, const bms_soc_config_t *config, int32_t current_ma,
                              uint32_t elapsed_ms);
bms_soc_result_t bms_soc_apply_ocv_target(bms_soc_t *soc, const bms_soc_config_t *config,
                                          uint16_t target_permille);
void bms_soc_anchor_full(bms_soc_t *soc);
void bms_soc_anchor_empty(bms_soc_t *soc);

#endif
