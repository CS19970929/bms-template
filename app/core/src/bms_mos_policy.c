#include "bms_mos_policy.h"
#include <stddef.h>

void bms_mos_policy_evaluate(const bms_mos_policy_input_t *input, bms_mos_policy_output_t *output)
{
    if (output == NULL) return;
    output->charge_enable = 0U;
    output->discharge_enable = 0U;
    output->effective_charge_block = UINT32_MAX;
    output->effective_discharge_block = UINT32_MAX;
    if (input == NULL) return;

    output->effective_charge_block = input->charge_block_mask | input->hardware_block_mask;
    output->effective_discharge_block = input->discharge_block_mask | input->hardware_block_mask;
    output->charge_enable = ((input->charge_requested != 0U) && (output->effective_charge_block == 0U)) ? 1U : 0U;
    output->discharge_enable = ((input->discharge_requested != 0U) && (output->effective_discharge_block == 0U)) ? 1U : 0U;

    if (input->topology == BMS_MOS_TOPOLOGY_COMMON_PORT) {
        /* Common-port hardware can still expose two logical FET commands. The policy does not
           infer board wiring; board output mapping owns any coupled-gate behavior. */
    }
}
