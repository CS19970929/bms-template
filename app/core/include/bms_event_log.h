#ifndef BMS_EVENT_LOG_H
#define BMS_EVENT_LOG_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    BMS_EVENT_SEVERITY_INFO = 0,
    BMS_EVENT_SEVERITY_WARNING,
    BMS_EVENT_SEVERITY_PROTECTION,
    BMS_EVENT_SEVERITY_FAULT
} bms_event_severity_t;

typedef enum {
    BMS_EVENT_SOURCE_BOOT = 0,
    BMS_EVENT_SOURCE_SYSTEM,
    BMS_EVENT_SOURCE_PROTECTION,
    BMS_EVENT_SOURCE_PARAMETER,
    BMS_EVENT_SOURCE_NVM,
    BMS_EVENT_SOURCE_COMM,
    BMS_EVENT_SOURCE_AFE,
    BMS_EVENT_SOURCE_IAP
} bms_event_source_t;

typedef enum {
    BMS_EVENT_LOG_OK = 0,
    BMS_EVENT_LOG_ERR_ARGUMENT,
    BMS_EVENT_LOG_ERR_RANGE,
    BMS_EVENT_LOG_ERR_EMPTY
} bms_event_log_result_t;

typedef struct {
    uint32_t sequence;
    uint32_t timestamp_ms;
    uint16_t event_id;
    uint8_t severity;
    uint8_t source;
    int32_t data0;
    int32_t data1;
} bms_event_record_t;

typedef struct {
    bms_event_record_t *records;
    size_t capacity;
    size_t count;
    size_t head;
    uint32_t next_sequence;
} bms_event_log_t;

bms_event_log_result_t bms_event_log_init(bms_event_log_t *log,
                                          bms_event_record_t *storage,
                                          size_t capacity,
                                          uint32_t initial_sequence);
bms_event_log_result_t bms_event_log_append(bms_event_log_t *log,
                                            uint32_t timestamp_ms,
                                            uint16_t event_id,
                                            bms_event_severity_t severity,
                                            bms_event_source_t source,
                                            int32_t data0,
                                            int32_t data1,
                                            uint32_t *sequence_out);
bms_event_log_result_t bms_event_log_get(const bms_event_log_t *log,
                                         size_t oldest_index,
                                         bms_event_record_t *record_out);
void bms_event_log_clear(bms_event_log_t *log);

#endif
