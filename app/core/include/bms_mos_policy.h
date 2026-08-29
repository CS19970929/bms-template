#ifndef BMS_MOS_POLICY_H
#define BMS_MOS_POLICY_H

#include <stdint.h>

typedef enum {
    BMS_MOS_TOPOLOGY_COMMON_PORT = 0,
    BMS_MOS_TOPOLOGY_SEPARATE_PORT = 1
} bms_mos_topology_t;

typedef struct {
    uint8_t charge_requested;
    uint8_t discharge_requested;
    uint32_t charge_block_mask;
    uint32_t discharge_block_mask;
    uint32_t hardware_block_mask;
    bms_mos_topology_t topology;
} bms_mos_policy_input_t;

typedef struct {
    uint8_t charge_enable;
    uint8_t discharge_enable;
    uint32_t effective_charge_block;
    uint32_t effective_discharge_block;
} bms_mos_policy_output_t;

void bms_mos_policy_evaluate(const bms_mos_policy_input_t *input, bms_mos_policy_output_t *output);

#endif
