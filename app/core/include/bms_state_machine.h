#ifndef BMS_STATE_MACHINE_H
#define BMS_STATE_MACHINE_H

#include <stdint.h>

typedef enum {
    BMS_STATE_INIT = 0,
    BMS_STATE_NORMAL,
    BMS_STATE_CHARGING,
    BMS_STATE_DISCHARGING,
    BMS_STATE_IDLE,
    BMS_STATE_PROTECTED,
    BMS_STATE_SLEEP,
    BMS_STATE_DEEP_SLEEP,
    BMS_STATE_SHUTDOWN,
    BMS_STATE_UPGRADE_PENDING,
    BMS_STATE_FAULT
} bms_system_state_t;

typedef enum {
    BMS_STATE_EVENT_INIT_OK = 0,
    BMS_STATE_EVENT_INIT_FAIL,
    BMS_STATE_EVENT_NORMAL,
    BMS_STATE_EVENT_CHARGE_ACTIVITY,
    BMS_STATE_EVENT_DISCHARGE_ACTIVITY,
    BMS_STATE_EVENT_IDLE,
    BMS_STATE_EVENT_PROTECTION_ACTIVE,
    BMS_STATE_EVENT_PROTECTION_CLEARED,
    BMS_STATE_EVENT_SLEEP_REQUEST,
    BMS_STATE_EVENT_DEEP_SLEEP_REQUEST,
    BMS_STATE_EVENT_WAKE,
    BMS_STATE_EVENT_SHUTDOWN_REQUEST,
    BMS_STATE_EVENT_UPGRADE_REQUEST,
    BMS_STATE_EVENT_FATAL_FAULT
} bms_state_event_t;

typedef enum {
    BMS_STATE_OK = 0,
    BMS_STATE_ERR_ARGUMENT = -1,
    BMS_STATE_ERR_TRANSITION = -2
} bms_state_result_t;

typedef struct {
    bms_system_state_t current;
    bms_system_state_t resume_after_protection;
    uint32_t transition_count;
} bms_state_machine_t;

void bms_state_machine_init(bms_state_machine_t *machine);
bms_state_result_t bms_state_machine_dispatch(bms_state_machine_t *machine, bms_state_event_t event);
bms_system_state_t bms_state_machine_get(const bms_state_machine_t *machine);

#endif
