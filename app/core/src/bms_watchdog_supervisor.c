#include "bms_watchdog_supervisor.h"
#include <stddef.h>

void bms_watchdog_supervisor_init(bms_watchdog_supervisor_t *supervisor, bms_health_mask_t required_mask)
{
    if (supervisor == NULL) return;
    supervisor->required_mask = required_mask;
    supervisor->observed_mask = 0U;
    supervisor->completed_windows = 0U;
    supervisor->failed_windows = 0U;
}

void bms_watchdog_supervisor_heartbeat(bms_watchdog_supervisor_t *supervisor, bms_health_mask_t module_mask)
{
    if (supervisor != NULL) supervisor->observed_mask |= (module_mask & supervisor->required_mask);
}

int bms_watchdog_supervisor_close_window(bms_watchdog_supervisor_t *supervisor)
{
    int healthy;
    if ((supervisor == NULL) || (supervisor->required_mask == 0U)) return 0;
    healthy = ((supervisor->observed_mask & supervisor->required_mask) == supervisor->required_mask) ? 1 : 0;
    if (healthy != 0) supervisor->completed_windows++;
    else supervisor->failed_windows++;
    supervisor->observed_mask = 0U;
    return healthy;
}
