#ifndef BMS_PROTECTION_MANAGER_H
#define BMS_PROTECTION_MANAGER_H

#include "bms_protection.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t id;
    bms_protection_config_t detector;
    uint32_t charge_block_mask;
    uint32_t discharge_block_mask;
    uint8_t latch_enabled;
} bms_protection_rule_t;

typedef struct {
    bms_protection_t detector;
    uint8_t latched;
} bms_protection_runtime_t;

typedef struct {
    uint32_t charge_block_mask;
    uint32_t discharge_block_mask;
    size_t active_count;
    uint16_t first_active_id;
    uint8_t any_active;
} bms_protection_manager_output_t;

typedef enum {
    BMS_PROTECTION_MANAGER_OK = 0,
    BMS_PROTECTION_MANAGER_ERR_ARGUMENT = -1,
    BMS_PROTECTION_MANAGER_ERR_CONFIG = -2,
    BMS_PROTECTION_MANAGER_ERR_DUPLICATE_ID = -3,
    BMS_PROTECTION_MANAGER_ERR_NOT_FOUND = -4,
    BMS_PROTECTION_MANAGER_ERR_CONDITION_ACTIVE = -5
} bms_protection_manager_result_t;

void bms_protection_manager_init(bms_protection_runtime_t *runtime, size_t count);
bms_protection_manager_result_t bms_protection_manager_validate_rules(const bms_protection_rule_t *rules,
                                                                       size_t count);
bms_protection_manager_result_t bms_protection_manager_step(const bms_protection_rule_t *rules,
                                                            bms_protection_runtime_t *runtime,
                                                            const int32_t *values,
                                                            size_t count,
                                                            uint32_t elapsed_ms,
                                                            bms_protection_manager_output_t *output);
bms_protection_manager_result_t bms_protection_manager_clear_latch(const bms_protection_rule_t *rules,
                                                                   bms_protection_runtime_t *runtime,
                                                                   size_t count,
                                                                   uint16_t id);

#endif
