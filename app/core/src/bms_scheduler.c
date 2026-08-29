#include "bms_scheduler.h"
#include <stddef.h>

static int time_due(uint32_t now, uint32_t due)
{
    return ((int32_t)(now - due) >= 0) ? 1 : 0;
}

void bms_scheduler_init(bms_scheduler_t *scheduler)
{
    size_t i;
    if (scheduler == NULL) return;
    scheduler->count = 0U;
    for (i = 0U; i < BMS_SCHEDULER_MAX_TASKS; ++i) scheduler->tasks[i].enabled = 0U;
}

int bms_scheduler_add(bms_scheduler_t *scheduler, bms_task_fn fn, void *context,
                      uint32_t period_ms, uint32_t first_due_ms)
{
    bms_task_t *task;
    if ((scheduler == NULL) || (fn == NULL) || (period_ms == 0U) || (scheduler->count >= BMS_SCHEDULER_MAX_TASKS)) return -1;
    task = &scheduler->tasks[scheduler->count++];
    task->fn = fn;
    task->context = context;
    task->period_ms = period_ms;
    task->next_due_ms = first_due_ms;
    task->enabled = 1U;
    return 0;
}

void bms_scheduler_run_due(bms_scheduler_t *scheduler, uint32_t now_ms)
{
    size_t i;
    if (scheduler == NULL) return;
    for (i = 0U; i < scheduler->count; ++i) {
        bms_task_t *task = &scheduler->tasks[i];
        if ((task->enabled != 0U) && (time_due(now_ms, task->next_due_ms) != 0)) {
            task->next_due_ms = now_ms + task->period_ms;
            task->fn(task->context, now_ms);
        }
    }
}
