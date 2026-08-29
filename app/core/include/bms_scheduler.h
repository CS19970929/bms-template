#ifndef BMS_SCHEDULER_H
#define BMS_SCHEDULER_H

#include <stddef.h>
#include <stdint.h>

#define BMS_SCHEDULER_MAX_TASKS 16U

typedef void (*bms_task_fn)(void *context, uint32_t now_ms);

typedef struct {
    bms_task_fn fn;
    void *context;
    uint32_t period_ms;
    uint32_t next_due_ms;
    uint8_t enabled;
} bms_task_t;

typedef struct {
    bms_task_t tasks[BMS_SCHEDULER_MAX_TASKS];
    size_t count;
} bms_scheduler_t;

void bms_scheduler_init(bms_scheduler_t *scheduler);
int bms_scheduler_add(bms_scheduler_t *scheduler, bms_task_fn fn, void *context,
                      uint32_t period_ms, uint32_t first_due_ms);
void bms_scheduler_run_due(bms_scheduler_t *scheduler, uint32_t now_ms);

#endif
