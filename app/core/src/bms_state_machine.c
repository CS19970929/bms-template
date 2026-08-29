#include "bms_state_machine.h"

static int bms_state_is_operational(bms_system_state_t state)
{
    return (state == BMS_STATE_NORMAL) || (state == BMS_STATE_CHARGING) ||
           (state == BMS_STATE_DISCHARGING) || (state == BMS_STATE_IDLE);
}

static void bms_state_set(bms_state_machine_t *machine, bms_system_state_t next)
{
    if (machine->current != next) {
        machine->current = next;
        machine->transition_count++;
    }
}

void bms_state_machine_init(bms_state_machine_t *machine)
{
    if (machine == NULL) {
        return;
    }
    machine->current = BMS_STATE_INIT;
    machine->resume_after_protection = BMS_STATE_IDLE;
    machine->transition_count = 0U;
}

bms_system_state_t bms_state_machine_get(const bms_state_machine_t *machine)
{
    if (machine == NULL) {
        return BMS_STATE_FAULT;
    }
    return machine->current;
}

bms_state_result_t bms_state_machine_dispatch(bms_state_machine_t *machine, bms_state_event_t event)
{
    if (machine == NULL) {
        return BMS_STATE_ERR_ARGUMENT;
    }

    switch (event) {
    case BMS_STATE_EVENT_INIT_OK:
        if (machine->current != BMS_STATE_INIT) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_IDLE);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_INIT_FAIL:
        if (machine->current != BMS_STATE_INIT) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_FAULT);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_NORMAL:
        if (!bms_state_is_operational(machine->current)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_NORMAL);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_CHARGE_ACTIVITY:
        if (!bms_state_is_operational(machine->current)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_CHARGING);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_DISCHARGE_ACTIVITY:
        if (!bms_state_is_operational(machine->current)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_DISCHARGING);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_IDLE:
        if (!bms_state_is_operational(machine->current)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_IDLE);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_PROTECTION_ACTIVE:
        if (bms_state_is_operational(machine->current)) {
            machine->resume_after_protection = machine->current;
            bms_state_set(machine, BMS_STATE_PROTECTED);
            return BMS_STATE_OK;
        }
        if (machine->current == BMS_STATE_PROTECTED) {
            return BMS_STATE_OK;
        }
        return BMS_STATE_ERR_TRANSITION;
    case BMS_STATE_EVENT_PROTECTION_CLEARED:
        if (machine->current != BMS_STATE_PROTECTED) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, machine->resume_after_protection);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_SLEEP_REQUEST:
        if ((machine->current != BMS_STATE_IDLE) && (machine->current != BMS_STATE_NORMAL)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_SLEEP);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_DEEP_SLEEP_REQUEST:
        if ((machine->current != BMS_STATE_IDLE) && (machine->current != BMS_STATE_SLEEP)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_DEEP_SLEEP);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_WAKE:
        if ((machine->current != BMS_STATE_SLEEP) && (machine->current != BMS_STATE_DEEP_SLEEP)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_IDLE);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_SHUTDOWN_REQUEST:
        if ((machine->current == BMS_STATE_FAULT) || (machine->current == BMS_STATE_UPGRADE_PENDING)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_SHUTDOWN);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_UPGRADE_REQUEST:
        if ((machine->current == BMS_STATE_FAULT) || (machine->current == BMS_STATE_SHUTDOWN)) {
            return BMS_STATE_ERR_TRANSITION;
        }
        bms_state_set(machine, BMS_STATE_UPGRADE_PENDING);
        return BMS_STATE_OK;
    case BMS_STATE_EVENT_FATAL_FAULT:
        bms_state_set(machine, BMS_STATE_FAULT);
        return BMS_STATE_OK;
    }
    return BMS_STATE_ERR_ARGUMENT;
}
