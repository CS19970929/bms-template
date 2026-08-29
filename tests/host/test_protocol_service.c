#include "bms_service.h"
#include <stdint.h>

typedef struct {
    uint16_t calls;
    uint16_t last_command;
} handler_context_t;

static bms_status_code_t echo_handler(void *context,
                                      uint16_t command,
                                      const uint8_t *request,
                                      uint16_t request_length,
                                      uint8_t *response,
                                      uint16_t response_capacity,
                                      uint16_t *response_length)
{
    handler_context_t *state = (handler_context_t *)context;
    (void)request;
    (void)request_length;
    if ((state == NULL) || (response == NULL) || (response_length == NULL)) {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if (response_capacity < 2U) {
        return BMS_STATUS_RANGE;
    }
    state->calls++;
    state->last_command = command;
    response[0] = (uint8_t)(command & UINT16_C(0x00FF));
    response[1] = (uint8_t)(command >> 8U);
    *response_length = 2U;
    return BMS_STATUS_OK;
}

int main(void)
{
    bms_service_router_t router;
    handler_context_t device = {0};
    handler_context_t protection = {0};
    uint8_t response[4] = {0};
    uint16_t response_length = 99U;

    bms_service_router_init(&router);
    if (bms_command_service(BMS_CMD_DEVICE_INFO) != BMS_SERVICE_DEVICE) {
        return 1;
    }
    if (bms_command_service(BMS_CMD_PROTECTION_SUMMARY) != BMS_SERVICE_PROTECTION) {
        return 2;
    }
    if (bms_command_service(BMS_CMD_IAP_START) != BMS_SERVICE_UNKNOWN) {
        return 3;
    }
    if (bms_service_router_bind(&router, BMS_SERVICE_DEVICE, echo_handler, &device) != BMS_STATUS_OK) {
        return 4;
    }
    if (bms_service_router_bind(&router, BMS_SERVICE_PROTECTION, echo_handler, &protection) != BMS_STATUS_OK) {
        return 5;
    }
    if (bms_service_dispatch(&router, BMS_CMD_DEVICE_INFO, NULL, 0U,
                             response, (uint16_t)sizeof(response), &response_length) != BMS_STATUS_OK) {
        return 6;
    }
    if ((device.calls != 1U) || (device.last_command != BMS_CMD_DEVICE_INFO) || (response_length != 2U)) {
        return 7;
    }
    if (bms_service_dispatch(&router, BMS_CMD_PROTECTION_SUMMARY, NULL, 0U,
                             response, (uint16_t)sizeof(response), &response_length) != BMS_STATUS_OK) {
        return 8;
    }
    if ((protection.calls != 1U) || (protection.last_command != BMS_CMD_PROTECTION_SUMMARY)) {
        return 9;
    }
    if (bms_service_dispatch(&router, BMS_CMD_PARAMETER_READ, NULL, 0U,
                             response, (uint16_t)sizeof(response), &response_length) != BMS_STATUS_UNSUPPORTED) {
        return 10;
    }
    if (response_length != 0U) {
        return 11;
    }
    if (bms_service_dispatch(&router, BMS_CMD_IAP_START, NULL, 0U,
                             response, (uint16_t)sizeof(response), &response_length) != BMS_STATUS_UNSUPPORTED) {
        return 12;
    }
    return 0;
}
