#include "bms_command_ids.h"
#include <stdint.h>

int main(void)
{
    if (BMS_COMMAND_SCHEMA_VERSION != UINT16_C(1)) return 1;
    if (BMS_CMD_DEVICE_INFO != UINT16_C(1)) return 2;
    if (BMS_CMD_PROTECTION_SUMMARY != UINT16_C(513)) return 3;
    if (BMS_CMD_LOG_READ != UINT16_C(1282)) return 4;
    if (BMS_CMD_IAP_PROGRESS != UINT16_C(4103)) return 5;
    return 0;
}
