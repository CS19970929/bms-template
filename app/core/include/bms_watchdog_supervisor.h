#ifndef BMS_WATCHDOG_SUPERVISOR_H
#define BMS_WATCHDOG_SUPERVISOR_H

#include <stdint.h>

typedef uint32_t bms_health_mask_t;

typedef struct {
    bms_health_mask_t required_mask;
    bms_health_mask_t observed_mask;
    uint32_t completed_windows;
    uint32_t failed_windows;
} bms_watchdog_supervisor_t;

void bms_watchdog_supervisor_init(bms_watchdog_supervisor_t *supervisor, bms_health_mask_t required_mask);
void bms_watchdog_supervisor_heartbeat(bms_watchdog_supervisor_t *supervisor, bms_health_mask_t module_mask);
int bms_watchdog_supervisor_close_window(bms_watchdog_supervisor_t *supervisor);

#endif
