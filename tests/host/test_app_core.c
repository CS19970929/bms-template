#include "bms_mos_policy.h"
#include "bms_scheduler.h"
#include "bms_watchdog_supervisor.h"
#include <stdint.h>

int bms_test_app_core(void);
#define T(x) do { if (!(x)) return __LINE__; } while (0)

static void count_task(void *context, uint32_t now_ms)
{
    uint32_t *count = (uint32_t *)context;
    (void)now_ms;
    (*count)++;
}

int bms_test_app_core(void)
{
    bms_watchdog_supervisor_t w;
    bms_mos_policy_input_t mi = {0};
    bms_mos_policy_output_t mo;
    bms_scheduler_t scheduler;
    uint32_t count = 0U;

    bms_watchdog_supervisor_init(&w, 0x07U);
    bms_watchdog_supervisor_heartbeat(&w, 0x01U);
    bms_watchdog_supervisor_heartbeat(&w, 0x02U);
    T(bms_watchdog_supervisor_close_window(&w) == 0);
    T(w.failed_windows == 1U);
    bms_watchdog_supervisor_heartbeat(&w, 0x07U);
    T(bms_watchdog_supervisor_close_window(&w) == 1);

    mi.charge_requested = 1U; mi.discharge_requested = 1U; mi.topology = BMS_MOS_TOPOLOGY_COMMON_PORT;
    bms_mos_policy_evaluate(&mi, &mo); T(mo.charge_enable == 1U); T(mo.discharge_enable == 1U);
    mi.charge_block_mask = 0x02U; bms_mos_policy_evaluate(&mi, &mo); T(mo.charge_enable == 0U); T(mo.discharge_enable == 1U);
    mi.hardware_block_mask = 0x80U; bms_mos_policy_evaluate(&mi, &mo); T(mo.charge_enable == 0U); T(mo.discharge_enable == 0U);

    bms_scheduler_init(&scheduler); T(bms_scheduler_add(&scheduler, count_task, &count, 10U, 5U) == 0);
    bms_scheduler_run_due(&scheduler, 4U); T(count == 0U);
    bms_scheduler_run_due(&scheduler, 5U); T(count == 1U);
    bms_scheduler_run_due(&scheduler, 14U); T(count == 1U);
    bms_scheduler_run_due(&scheduler, 15U); T(count == 2U);
    return 0;
}
