#include "bms_event_log.h"
#include <stdint.h>

int main(void)
{
    bms_event_record_t storage[3];
    bms_event_record_t record;
    bms_event_log_t log;
    uint32_t sequence = 0U;

    if (bms_event_log_init(&log, storage, 3U, 100U) != BMS_EVENT_LOG_OK) {
        return 1;
    }
    if (bms_event_log_get(&log, 0U, &record) != BMS_EVENT_LOG_ERR_EMPTY) {
        return 2;
    }
    if (bms_event_log_append(&log, 10U, 1U, BMS_EVENT_SEVERITY_INFO,
                             BMS_EVENT_SOURCE_BOOT, 11, 12, &sequence) != BMS_EVENT_LOG_OK) {
        return 3;
    }
    if (sequence != 100U) {
        return 4;
    }
    if (bms_event_log_append(&log, 20U, 2U, BMS_EVENT_SEVERITY_WARNING,
                             BMS_EVENT_SOURCE_COMM, 21, 22, NULL) != BMS_EVENT_LOG_OK) {
        return 5;
    }
    if (bms_event_log_append(&log, 30U, 3U, BMS_EVENT_SEVERITY_PROTECTION,
                             BMS_EVENT_SOURCE_PROTECTION, 31, 32, NULL) != BMS_EVENT_LOG_OK) {
        return 6;
    }
    if (bms_event_log_append(&log, 40U, 4U, BMS_EVENT_SEVERITY_FAULT,
                             BMS_EVENT_SOURCE_AFE, 41, 42, NULL) != BMS_EVENT_LOG_OK) {
        return 7;
    }
    if ((log.count != 3U) || (log.next_sequence != 104U)) {
        return 8;
    }
    if (bms_event_log_get(&log, 0U, &record) != BMS_EVENT_LOG_OK) {
        return 9;
    }
    if ((record.sequence != 101U) || (record.event_id != 2U) || (record.data0 != 21)) {
        return 10;
    }
    if (bms_event_log_get(&log, 2U, &record) != BMS_EVENT_LOG_OK) {
        return 11;
    }
    if ((record.sequence != 103U) || (record.event_id != 4U) ||
        (record.source != (uint8_t)BMS_EVENT_SOURCE_AFE)) {
        return 12;
    }
    if (bms_event_log_append(&log, 50U, 0U, BMS_EVENT_SEVERITY_INFO,
                             BMS_EVENT_SOURCE_SYSTEM, 0, 0, NULL) != BMS_EVENT_LOG_ERR_RANGE) {
        return 13;
    }
    if (log.next_sequence != 104U) {
        return 14;
    }
    bms_event_log_clear(&log);
    if ((log.count != 0U) || (log.next_sequence != 104U)) {
        return 15;
    }
    return 0;
}
