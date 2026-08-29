#ifndef BMS_AFE_H
#define BMS_AFE_H

#include <stddef.h>
#include <stdint.h>

#define BMS_AFE_MAX_CELLS 32U
#define BMS_AFE_MAX_TEMPS 8U

typedef struct {
    uint16_t cell_mv[BMS_AFE_MAX_CELLS];
    int16_t temperature_decic[BMS_AFE_MAX_TEMPS];
    int32_t current_ma;
    uint32_t pack_mv;
    uint8_t cell_count;
    uint8_t temperature_count;
    uint32_t fault_bits;
} bms_afe_sample_t;

typedef struct bms_afe bms_afe_t;

typedef struct {
    int (*init)(bms_afe_t *afe);
    int (*sample)(bms_afe_t *afe, bms_afe_sample_t *sample);
    int (*set_charge_fet)(bms_afe_t *afe, uint8_t enable);
    int (*set_discharge_fet)(bms_afe_t *afe, uint8_t enable);
    int (*set_balance_mask)(bms_afe_t *afe, uint32_t mask);
    uint32_t (*get_faults)(bms_afe_t *afe);
} bms_afe_ops_t;

struct bms_afe {
    const bms_afe_ops_t *ops;
    void *context;
};

#endif
