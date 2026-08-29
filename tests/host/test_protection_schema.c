#include "bms_protection_ids.h"
#include <stdint.h>

int main(void)
{
    if (BMS_PROTECTION_SCHEMA_VERSION != UINT16_C(1)) {
        return 1;
    }
    if (BMS_PROTECTION_ID_CELL_OV != UINT16_C(257)) {
        return 2;
    }
    if (BMS_PROTECTION_ID_DISCHARGE_OC != UINT16_C(514)) {
        return 3;
    }
    if (BMS_PROTECTION_ID_MOS_OTP != UINT16_C(773)) {
        return 4;
    }
    if (BMS_PROTECTION_ID_AFE_FAULT != UINT16_C(1537)) {
        return 5;
    }
    return 0;
}
