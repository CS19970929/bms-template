#ifndef BMS_COMMANDS_H
#define BMS_COMMANDS_H

#include "bms_command_ids.h"
#include <stdint.h>

typedef enum {
    BMS_MSG_REQUEST = 1,
    BMS_MSG_RESPONSE = 2,
    BMS_MSG_EVENT = 3
} bms_message_type_t;

typedef enum {
    BMS_STATUS_OK = 0,
    BMS_STATUS_INVALID_ARGUMENT = 1,
    BMS_STATUS_INVALID_STATE = 2,
    BMS_STATUS_INVALID_IMAGE = 3,
    BMS_STATUS_RANGE = 4,
    BMS_STATUS_SEQUENCE = 5,
    BMS_STATUS_STORAGE = 6,
    BMS_STATUS_VERIFY = 7,
    BMS_STATUS_UNSUPPORTED = 8,
    BMS_STATUS_INTERNAL = 255
} bms_status_code_t;

#endif
