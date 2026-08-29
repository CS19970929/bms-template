#include "bms_event_log.h"

static int severity_valid(bms_event_severity_t severity)
{
    return ((severity >= BMS_EVENT_SEVERITY_INFO) &&
            (severity <= BMS_EVENT_SEVERITY_FAULT)) ? 1 : 0;
}

static int source_valid(bms_event_source_t source)
{
    return ((source >= BMS_EVENT_SOURCE_BOOT) &&
            (source <= BMS_EVENT_SOURCE_IAP)) ? 1 : 0;
}

bms_event_log_result_t bms_event_log_init(bms_event_log_t *log,
                                          bms_event_record_t *storage,
                                          size_t capacity,
                                          uint32_t initial_sequence)
{
    if ((log == NULL) || (storage == NULL) || (capacity == 0U)) {
        return BMS_EVENT_LOG_ERR_ARGUMENT;
    }
    log->records = storage;
    log->capacity = capacity;
    log->count = 0U;
    log->head = 0U;
    log->next_sequence = initial_sequence;
    return BMS_EVENT_LOG_OK;
}

bms_event_log_result_t bms_event_log_append(bms_event_log_t *log,
                                            uint32_t timestamp_ms,
                                            uint16_t event_id,
                                            bms_event_severity_t severity,
                                            bms_event_source_t source,
                                            int32_t data0,
                                            int32_t data1,
                                            uint32_t *sequence_out)
{
    bms_event_record_t *record;
    uint32_t sequence;

    if ((log == NULL) || (log->records == NULL) || (log->capacity == 0U)) {
        return BMS_EVENT_LOG_ERR_ARGUMENT;
    }
    if ((event_id == 0U) || (severity_valid(severity) == 0) || (source_valid(source) == 0)) {
        return BMS_EVENT_LOG_ERR_RANGE;
    }

    sequence = log->next_sequence;
    record = &log->records[log->head];
    record->sequence = sequence;
    record->timestamp_ms = timestamp_ms;
    record->event_id = event_id;
    record->severity = (uint8_t)severity;
    record->source = (uint8_t)source;
    record->data0 = data0;
    record->data1 = data1;

    log->head++;
    if (log->head == log->capacity) {
        log->head = 0U;
    }
    if (log->count < log->capacity) {
        log->count++;
    }
    log->next_sequence = sequence + UINT32_C(1);
    if (sequence_out != NULL) {
        *sequence_out = sequence;
    }
    return BMS_EVENT_LOG_OK;
}

bms_event_log_result_t bms_event_log_get(const bms_event_log_t *log,
                                         size_t oldest_index,
                                         bms_event_record_t *record_out)
{
    size_t oldest;
    size_t physical;

    if ((log == NULL) || (record_out == NULL) || (log->records == NULL) || (log->capacity == 0U)) {
        return BMS_EVENT_LOG_ERR_ARGUMENT;
    }
    if (log->count == 0U) {
        return BMS_EVENT_LOG_ERR_EMPTY;
    }
    if (oldest_index >= log->count) {
        return BMS_EVENT_LOG_ERR_RANGE;
    }

    oldest = (log->count == log->capacity) ? log->head : 0U;
    physical = oldest + oldest_index;
    if (physical >= log->capacity) {
        physical -= log->capacity;
    }
    *record_out = log->records[physical];
    return BMS_EVENT_LOG_OK;
}

void bms_event_log_clear(bms_event_log_t *log)
{
    if (log == NULL) {
        return;
    }
    log->count = 0U;
    log->head = 0U;
}
