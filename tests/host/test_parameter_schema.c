#include "bms_parameter_ids.h"
#include <stdint.h>

int main(void)
{
    if (BMS_PARAMETER_SCHEMA_VERSION != UINT16_C(1)) {
        return 1;
    }
    if (BMS_PARAM_ID_SOC_CURRENT_FLOOR_MA != UINT16_C(257)) {
        return 2;
    }
    if (BMS_PARAM_ID_SOC_REST_REQUIRED_MS != UINT16_C(258)) {
        return 3;
    }
    if (BMS_PARAM_ID_SOC_FORBID_REST_UPWARD_CORRECTION != UINT16_C(259)) {
        return 4;
    }
    return 0;
}
