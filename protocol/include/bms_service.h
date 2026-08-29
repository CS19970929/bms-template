#ifndef BMS_SERVICE_H
#define BMS_SERVICE_H

#include "bms_commands.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    BMS_SERVICE_DEVICE = 0,
    BMS_SERVICE_TELEMETRY,
    BMS_SERVICE_PROTECTION,
    BMS_SERVICE_PARAMETER,
    BMS_SERVICE_CONTROL,
    BMS_SERVICE_LOG,
    BMS_SERVICE_DIAGNOSTIC,
    BMS_SERVICE_COUNT,
    BMS_SERVICE_UNKNOWN = 255
} bms_service_kind_t;

typedef bms_status_code_t (*bms_service_handler_fn)(void *context,
                                                    uint16_t command,
                                                    const uint8_t *request,
                                                    uint16_t request_length,
                                                    uint8_t *response,
                                                    uint16_t response_capacity,
                                                    uint16_t *response_length);

typedef struct {
    bms_service_handler_fn handler;
    void *context;
} bms_service_slot_t;

typedef struct {
    bms_service_slot_t slots[BMS_SERVICE_COUNT];
} bms_service_router_t;

void bms_service_router_init(bms_service_router_t *router);
bms_service_kind_t bms_command_service(uint16_t command);
bms_status_code_t bms_service_router_bind(bms_service_router_t *router,
                                          bms_service_kind_t service,
                                          bms_service_handler_fn handler,
                                          void *context);
bms_status_code_t bms_service_dispatch(const bms_service_router_t *router,
                                       uint16_t command,
                                       const uint8_t *request,
                                       uint16_t request_length,
                                       uint8_t *response,
                                       uint16_t response_capacity,
                                       uint16_t *response_length);

#endif
