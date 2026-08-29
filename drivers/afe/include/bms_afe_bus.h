#ifndef BMS_AFE_BUS_H
#define BMS_AFE_BUS_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int (*write)(void *context, uint8_t device_address, const uint8_t *data, size_t length);
    int (*write_read)(void *context, uint8_t device_address,
                      const uint8_t *tx, size_t tx_length, uint8_t *rx, size_t rx_length);
    uint32_t (*time_ms)(void *context);
    void *context;
} bms_afe_bus_t;

#endif
