#include "bms_event_ids.h"
#include <stdint.h>

int main(void)
{
    if (BMS_EVENT_SCHEMA_VERSION != UINT16_C(1)) {
        return 1;
    }
    if (BMS_EVENT_ID_BOOT_START != UINT16_C(257)) {
        return 2;
    }
    if (BMS_EVENT_ID_PROTECTION_TRIP != UINT16_C(769)) {
        return 3;
    }
    if (BMS_EVENT_ID_NVM_ERROR != UINT16_C(1281)) {
        return 4;
    }
    if (BMS_EVENT_ID_COMM_ERROR != UINT16_C(2049)) {
        return 5;
    }
    return 0;
}
