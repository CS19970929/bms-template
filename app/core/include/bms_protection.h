#ifndef BMS_PROTECTION_H
#define BMS_PROTECTION_H

#include <stdint.h>

typedef enum {
    BMS_PROTECT_MODE_HIGH = 0,
    BMS_PROTECT_MODE_LOW = 1
} bms_protect_mode_t;

typedef enum {
    BMS_PROTECT_NORMAL = 0,
    BMS_PROTECT_PENDING,
    BMS_PROTECT_ACTIVE,
    BMS_PROTECT_RECOVERING
} bms_protect_state_t;

typedef struct {
    int32_t trip_threshold;
    int32_t release_threshold;
    uint32_t trip_delay_ms;
    uint32_t release_delay_ms;
    bms_protect_mode_t mode;
    uint8_t enabled;
} bms_protection_config_t;

typedef struct {
    bms_protect_state_t state;
    uint32_t elapsed_ms;
} bms_protection_t;

void bms_protection_init(bms_protection_t *protection);
bms_protect_state_t bms_protection_step(bms_protection_t *protection,
                                        const bms_protection_config_t *config,
                                        int32_t value, uint32_t elapsed_ms);

#endif
