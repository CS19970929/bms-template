#include "bms_soc.h"
#include <limits.h>
#include <stddef.h>

#define BMS_SOC_PERMILLE_MAX 1000U
#define BMS_MILLISECONDS_PER_SECOND 1000LL
#define BMS_SECONDS_PER_HOUR 3600LL

static uint32_t magnitude_i32(int32_t value)
{
    if (value >= 0) {
        return (uint32_t)value;
    }
    return (uint32_t)(-(int64_t)value);
}

static uint16_t calculate_permille(const bms_soc_t *soc)
{
    int64_t scaled;
    if (soc->charge_mas <= 0LL) {
        return 0U;
    }
    if (soc->charge_mas >= soc->capacity_mas) {
        return BMS_SOC_PERMILLE_MAX;
    }
    scaled = (soc->charge_mas * (int64_t)BMS_SOC_PERMILLE_MAX) / soc->capacity_mas;
    return (uint16_t)scaled;
}

static int64_t charge_from_permille(const bms_soc_t *soc, uint16_t permille)
{
    return (soc->capacity_mas * (int64_t)permille) / (int64_t)BMS_SOC_PERMILLE_MAX;
}

static void update_display(bms_soc_t *soc, uint16_t maximum_step)
{
    uint16_t delta;
    if (soc->soc_display_permille < soc->soc_est_permille) {
        delta = (uint16_t)(soc->soc_est_permille - soc->soc_display_permille);
        if (delta > maximum_step) {
            delta = maximum_step;
        }
        soc->soc_display_permille = (uint16_t)(soc->soc_display_permille + delta);
    } else if (soc->soc_display_permille > soc->soc_est_permille) {
        delta = (uint16_t)(soc->soc_display_permille - soc->soc_est_permille);
        if (delta > maximum_step) {
            delta = maximum_step;
        }
        soc->soc_display_permille = (uint16_t)(soc->soc_display_permille - delta);
    }
}

static int config_valid(const bms_soc_config_t *config)
{
    if ((config == NULL) || (config->nominal_capacity_mah == 0U) ||
        (config->integration_period_ms == 0U) || (config->display_step_permille == 0U) ||
        (config->display_step_permille > BMS_SOC_PERMILLE_MAX) ||
        (config->ocv_correction_step_permille == 0U) ||
        (config->ocv_correction_step_permille > BMS_SOC_PERMILLE_MAX)) {
        return 0;
    }
    return 1;
}

bms_soc_result_t bms_soc_init(bms_soc_t *soc, const bms_soc_config_t *config, uint16_t initial_permille)
{
    if (soc == NULL) {
        return BMS_SOC_ERR_ARGUMENT;
    }
    if ((config_valid(config) == 0) || (initial_permille > BMS_SOC_PERMILLE_MAX)) {
        return BMS_SOC_ERR_CONFIG;
    }
    soc->capacity_mas = (int64_t)config->nominal_capacity_mah * BMS_SECONDS_PER_HOUR;
    soc->charge_mas = (soc->capacity_mas * (int64_t)initial_permille) / (int64_t)BMS_SOC_PERMILLE_MAX;
    soc->integration_remainder_mams = 0LL;
    soc->rest_elapsed_ms = 0U;
    soc->soc_est_permille = initial_permille;
    soc->soc_display_permille = initial_permille;
    soc->valid = 1U;
    return BMS_SOC_OK;
}

bms_soc_result_t bms_soc_step(bms_soc_t *soc, const bms_soc_config_t *config, int32_t current_ma,
                              uint32_t elapsed_ms)
{
    uint32_t magnitude;
    int64_t delta_mams;
    int64_t delta_mas;
    if ((soc == NULL) || (soc->valid == 0U)) {
        return BMS_SOC_ERR_ARGUMENT;
    }
    if (config_valid(config) == 0) {
        return BMS_SOC_ERR_CONFIG;
    }
    if (elapsed_ms != config->integration_period_ms) {
        return BMS_SOC_ERR_PERIOD;
    }

    magnitude = magnitude_i32(current_ma);
    if (magnitude <= config->rest_current_threshold_ma) {
        if (soc->rest_elapsed_ms <= (UINT32_MAX - elapsed_ms)) {
            soc->rest_elapsed_ms += elapsed_ms;
        } else {
            soc->rest_elapsed_ms = UINT32_MAX;
        }
    } else {
        soc->rest_elapsed_ms = 0U;
    }

    if (magnitude >= config->current_floor_ma) {
        delta_mams = ((int64_t)current_ma * (int64_t)elapsed_ms) + soc->integration_remainder_mams;
        delta_mas = delta_mams / BMS_MILLISECONDS_PER_SECOND;
        soc->integration_remainder_mams = delta_mams % BMS_MILLISECONDS_PER_SECOND;
        soc->charge_mas += delta_mas;
        if (soc->charge_mas < 0LL) {
            soc->charge_mas = 0LL;
            soc->integration_remainder_mams = 0LL;
        } else if (soc->charge_mas > soc->capacity_mas) {
            soc->charge_mas = soc->capacity_mas;
            soc->integration_remainder_mams = 0LL;
        }
        soc->soc_est_permille = calculate_permille(soc);
    }

    update_display(soc, config->display_step_permille);
    return BMS_SOC_OK;
}

bms_soc_result_t bms_soc_apply_ocv_target(bms_soc_t *soc, const bms_soc_config_t *config,
                                          uint16_t target_permille)
{
    uint16_t next;
    uint16_t delta;
    if ((soc == NULL) || (soc->valid == 0U)) {
        return BMS_SOC_ERR_ARGUMENT;
    }
    if ((config_valid(config) == 0) || (target_permille > BMS_SOC_PERMILLE_MAX)) {
        return BMS_SOC_ERR_CONFIG;
    }
    if (soc->rest_elapsed_ms < config->rest_required_ms) {
        return BMS_SOC_ERR_NOT_RESTED;
    }
    if ((config->forbid_rest_upward_correction != 0U) && (target_permille > soc->soc_est_permille)) {
        return BMS_SOC_OK_NO_CHANGE;
    }
    if (target_permille == soc->soc_est_permille) {
        return BMS_SOC_OK_NO_CHANGE;
    }

    next = soc->soc_est_permille;
    if (target_permille > next) {
        delta = (uint16_t)(target_permille - next);
        if (delta > config->ocv_correction_step_permille) {
            delta = config->ocv_correction_step_permille;
        }
        next = (uint16_t)(next + delta);
    } else {
        delta = (uint16_t)(next - target_permille);
        if (delta > config->ocv_correction_step_permille) {
            delta = config->ocv_correction_step_permille;
        }
        next = (uint16_t)(next - delta);
    }
    soc->soc_est_permille = next;
    soc->charge_mas = charge_from_permille(soc, next);
    soc->integration_remainder_mams = 0LL;
    update_display(soc, config->display_step_permille);
    return BMS_SOC_OK;
}

void bms_soc_anchor_full(bms_soc_t *soc)
{
    if ((soc != NULL) && (soc->valid != 0U)) {
        soc->charge_mas = soc->capacity_mas;
        soc->integration_remainder_mams = 0LL;
        soc->soc_est_permille = BMS_SOC_PERMILLE_MAX;
        soc->soc_display_permille = BMS_SOC_PERMILLE_MAX;
    }
}

void bms_soc_anchor_empty(bms_soc_t *soc)
{
    if ((soc != NULL) && (soc->valid != 0U)) {
        soc->charge_mas = 0LL;
        soc->integration_remainder_mams = 0LL;
        soc->soc_est_permille = 0U;
        soc->soc_display_permille = 0U;
    }
}
