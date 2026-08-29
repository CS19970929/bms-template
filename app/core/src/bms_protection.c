#include "bms_protection.h"
#include <stddef.h>

static int trip_condition(const bms_protection_config_t *config, int32_t value)
{
    return (config->mode == BMS_PROTECT_MODE_HIGH) ? (value >= config->trip_threshold) : (value <= config->trip_threshold);
}

static int release_condition(const bms_protection_config_t *config, int32_t value)
{
    return (config->mode == BMS_PROTECT_MODE_HIGH) ? (value <= config->release_threshold) : (value >= config->release_threshold);
}

static uint32_t add_sat_u32(uint32_t a, uint32_t b)
{
    return (UINT32_MAX - a < b) ? UINT32_MAX : (a + b);
}

void bms_protection_init(bms_protection_t *protection)
{
    if (protection != NULL) {
        protection->state = BMS_PROTECT_NORMAL;
        protection->elapsed_ms = 0U;
    }
}

bms_protect_state_t bms_protection_step(bms_protection_t *protection,
                                        const bms_protection_config_t *config,
                                        int32_t value, uint32_t elapsed_ms)
{
    if ((protection == NULL) || (config == NULL)) {
        return BMS_PROTECT_NORMAL;
    }
    if (config->enabled == 0U) {
        bms_protection_init(protection);
        return protection->state;
    }

    switch (protection->state) {
    case BMS_PROTECT_NORMAL:
        if (trip_condition(config, value) != 0) {
            protection->state = BMS_PROTECT_PENDING;
            protection->elapsed_ms = elapsed_ms;
            if (protection->elapsed_ms >= config->trip_delay_ms) {
                protection->state = BMS_PROTECT_ACTIVE;
                protection->elapsed_ms = 0U;
            }
        }
        break;
    case BMS_PROTECT_PENDING:
        if (trip_condition(config, value) == 0) {
            bms_protection_init(protection);
        } else {
            protection->elapsed_ms = add_sat_u32(protection->elapsed_ms, elapsed_ms);
            if (protection->elapsed_ms >= config->trip_delay_ms) {
                protection->state = BMS_PROTECT_ACTIVE;
                protection->elapsed_ms = 0U;
            }
        }
        break;
    case BMS_PROTECT_ACTIVE:
        if (release_condition(config, value) != 0) {
            protection->state = BMS_PROTECT_RECOVERING;
            protection->elapsed_ms = elapsed_ms;
            if (protection->elapsed_ms >= config->release_delay_ms) {
                bms_protection_init(protection);
            }
        }
        break;
    case BMS_PROTECT_RECOVERING:
        if (release_condition(config, value) == 0) {
            protection->state = BMS_PROTECT_ACTIVE;
            protection->elapsed_ms = 0U;
        } else {
            protection->elapsed_ms = add_sat_u32(protection->elapsed_ms, elapsed_ms);
            if (protection->elapsed_ms >= config->release_delay_ms) {
                bms_protection_init(protection);
            }
        }
        break;
    default:
        bms_protection_init(protection);
        break;
    }
    return protection->state;
}
