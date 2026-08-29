#ifndef BMS_AFE_MOCK_H
#define BMS_AFE_MOCK_H

#include "bms_afe.h"

typedef struct {
    bms_afe_sample_t sample;
    uint8_t charge_fet;
    uint8_t discharge_fet;
    uint32_t balance_mask;
} bms_afe_mock_context_t;

void bms_afe_mock_bind(bms_afe_t *afe, bms_afe_mock_context_t *context);

#endif
