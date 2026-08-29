#include "bms_afe_mock.h"
#include <stddef.h>

static int mock_init(bms_afe_t *afe)
{
    bms_afe_mock_context_t *context;
    if ((afe == NULL) || (afe->context == NULL)) return -1;
    context = (bms_afe_mock_context_t *)afe->context;
    context->charge_fet = 0U;
    context->discharge_fet = 0U;
    context->balance_mask = 0U;
    return 0;
}
static int mock_sample(bms_afe_t *afe, bms_afe_sample_t *sample)
{
    if ((afe == NULL) || (sample == NULL) || (afe->context == NULL)) return -1;
    *sample = ((bms_afe_mock_context_t *)afe->context)->sample;
    return 0;
}
static int mock_charge(bms_afe_t *afe, uint8_t enable)
{
    if ((afe == NULL) || (afe->context == NULL)) return -1;
    ((bms_afe_mock_context_t *)afe->context)->charge_fet = (enable != 0U) ? 1U : 0U;
    return 0;
}
static int mock_discharge(bms_afe_t *afe, uint8_t enable)
{
    if ((afe == NULL) || (afe->context == NULL)) return -1;
    ((bms_afe_mock_context_t *)afe->context)->discharge_fet = (enable != 0U) ? 1U : 0U;
    return 0;
}
static int mock_balance(bms_afe_t *afe, uint32_t mask)
{
    if ((afe == NULL) || (afe->context == NULL)) return -1;
    ((bms_afe_mock_context_t *)afe->context)->balance_mask = mask;
    return 0;
}
static uint32_t mock_faults(bms_afe_t *afe)
{
    if ((afe == NULL) || (afe->context == NULL)) return UINT32_MAX;
    return ((bms_afe_mock_context_t *)afe->context)->sample.fault_bits;
}
static const bms_afe_ops_t mock_ops = {mock_init, mock_sample, mock_charge, mock_discharge, mock_balance, mock_faults};

void bms_afe_mock_bind(bms_afe_t *afe, bms_afe_mock_context_t *context)
{
    if (afe != NULL) {
        afe->ops = &mock_ops;
        afe->context = context;
    }
}
