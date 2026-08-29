#include "bms_service.h"

void bms_service_router_init(bms_service_router_t *router)
{
    size_t i;
    if (router == NULL) {
        return;
    }
    for (i = 0U; i < (size_t)BMS_SERVICE_COUNT; ++i) {
        router->slots[i].handler = NULL;
        router->slots[i].context = NULL;
    }
}

bms_service_kind_t bms_command_service(uint16_t command)
{
    if (command <= UINT16_C(0x00FF)) {
        return BMS_SERVICE_DEVICE;
    }
    if ((command >= UINT16_C(0x0100)) && (command <= UINT16_C(0x01FF))) {
        return BMS_SERVICE_TELEMETRY;
    }
    if ((command >= UINT16_C(0x0200)) && (command <= UINT16_C(0x02FF))) {
        return BMS_SERVICE_PROTECTION;
    }
    if ((command >= UINT16_C(0x0300)) && (command <= UINT16_C(0x03FF))) {
        return BMS_SERVICE_PARAMETER;
    }
    if ((command >= UINT16_C(0x0400)) && (command <= UINT16_C(0x04FF))) {
        return BMS_SERVICE_CONTROL;
    }
    if ((command >= UINT16_C(0x0500)) && (command <= UINT16_C(0x05FF))) {
        return BMS_SERVICE_LOG;
    }
    if ((command >= UINT16_C(0x0600)) && (command <= UINT16_C(0x06FF))) {
        return BMS_SERVICE_DIAGNOSTIC;
    }
    return BMS_SERVICE_UNKNOWN;
}

bms_status_code_t bms_service_router_bind(bms_service_router_t *router,
                                          bms_service_kind_t service,
                                          bms_service_handler_fn handler,
                                          void *context)
{
    size_t slot;
    if ((router == NULL) || (handler == NULL)) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if (service >= BMS_SERVICE_COUNT) {
        return BMS_STATUS_RANGE;
    }
    slot = (size_t)service;
    router->slots[slot].handler = handler;
    router->slots[slot].context = context;
    return BMS_STATUS_OK;
}

bms_status_code_t bms_service_dispatch(const bms_service_router_t *router,
                                       uint16_t command,
                                       const uint8_t *request,
                                       uint16_t request_length,
                                       uint8_t *response,
                                       uint16_t response_capacity,
                                       uint16_t *response_length)
{
    bms_service_kind_t service;
    size_t slot;
    const bms_service_slot_t *binding;

    if (response_length == NULL) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    *response_length = 0U;
    if (router == NULL) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if ((request_length != 0U) && (request == NULL)) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if ((response_capacity != 0U) && (response == NULL)) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    service = bms_command_service(command);
    if (service == BMS_SERVICE_UNKNOWN) {
        return BMS_STATUS_UNSUPPORTED;
    }
    slot = (size_t)service;
    binding = &router->slots[slot];
    if (binding->handler == NULL) {
        return BMS_STATUS_UNSUPPORTED;
    }
    return binding->handler(binding->context, command, request, request_length,
                            response, response_capacity, response_length);
}
