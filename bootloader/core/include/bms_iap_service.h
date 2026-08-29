#ifndef BMS_IAP_SERVICE_H
#define BMS_IAP_SERVICE_H

#include "bms_iap_session.h"
#include <stddef.h>
#include <stdint.h>

#define BMS_IAP_MANIFEST_WIRE_SIZE 24U
#define BMS_IAP_WRITE_PREFIX_SIZE 6U

typedef struct {
    bms_iap_session_t *session;
    const bms_iap_storage_t *storage;
} bms_iap_service_t;

int bms_iap_manifest_decode(const uint8_t *payload, size_t length, bms_image_manifest_t *image);
int bms_iap_manifest_encode(const bms_image_manifest_t *image, uint8_t *payload, size_t capacity);
int bms_iap_service_handle(bms_iap_service_t *service, uint16_t command,
                           const uint8_t *payload, uint16_t payload_length,
                           uint8_t *response, size_t response_capacity, uint16_t *response_length);

#endif
