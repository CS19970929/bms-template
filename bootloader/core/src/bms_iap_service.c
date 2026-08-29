#include "bms_iap_service.h"
#include "bms_commands.h"
#include <stddef.h>

static uint16_t get_u16_le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8U));
}

static uint32_t get_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8U) | ((uint32_t)p[2] << 16U) | ((uint32_t)p[3] << 24U);
}

static void put_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8U) & 0xFFU);
}

static void put_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFU);
    p[1] = (uint8_t)((v >> 8U) & 0xFFU);
    p[2] = (uint8_t)((v >> 16U) & 0xFFU);
    p[3] = (uint8_t)((v >> 24U) & 0xFFU);
}

static uint8_t result_to_status(bms_iap_result_t result)
{
    switch (result) {
    case BMS_IAP_OK:
    case BMS_IAP_OK_DUPLICATE:
        return (uint8_t)BMS_STATUS_OK;
    case BMS_IAP_ERR_ARGUMENT:
    case BMS_IAP_ERR_CHUNK:
        return (uint8_t)BMS_STATUS_INVALID_ARGUMENT;
    case BMS_IAP_ERR_STATE:
        return (uint8_t)BMS_STATUS_INVALID_STATE;
    case BMS_IAP_ERR_MANIFEST:
        return (uint8_t)BMS_STATUS_INVALID_IMAGE;
    case BMS_IAP_ERR_RANGE:
        return (uint8_t)BMS_STATUS_RANGE;
    case BMS_IAP_ERR_OFFSET:
        return (uint8_t)BMS_STATUS_SEQUENCE;
    case BMS_IAP_ERR_STORAGE:
    case BMS_IAP_ERR_METADATA:
        return (uint8_t)BMS_STATUS_STORAGE;
    case BMS_IAP_ERR_VERIFY:
        return (uint8_t)BMS_STATUS_VERIFY;
    default:
        return (uint8_t)BMS_STATUS_INTERNAL;
    }
}

int bms_iap_manifest_decode(const uint8_t *payload, size_t length, bms_image_manifest_t *image)
{
    if ((payload == NULL) || (image == NULL) || (length != BMS_IAP_MANIFEST_WIRE_SIZE)) return -1;
    image->magic = get_u32_le(&payload[0]);
    image->manifest_version = get_u16_le(&payload[4]);
    image->mcu_id = get_u16_le(&payload[6]);
    image->product_id = get_u32_le(&payload[8]);
    image->image_size = get_u32_le(&payload[12]);
    image->image_crc32 = get_u32_le(&payload[16]);
    image->firmware_version = get_u32_le(&payload[20]);
    return 0;
}

int bms_iap_manifest_encode(const bms_image_manifest_t *image, uint8_t *payload, size_t capacity)
{
    if ((image == NULL) || (payload == NULL) || (capacity < BMS_IAP_MANIFEST_WIRE_SIZE)) return -1;
    put_u32_le(&payload[0], image->magic);
    put_u16_le(&payload[4], image->manifest_version);
    put_u16_le(&payload[6], image->mcu_id);
    put_u32_le(&payload[8], image->product_id);
    put_u32_le(&payload[12], image->image_size);
    put_u32_le(&payload[16], image->image_crc32);
    put_u32_le(&payload[20], image->firmware_version);
    return 0;
}

int bms_iap_service_handle(bms_iap_service_t *service, uint16_t command,
                           const uint8_t *payload, uint16_t payload_length,
                           uint8_t *response, size_t response_capacity, uint16_t *response_length)
{
    bms_iap_result_t result = BMS_IAP_ERR_ARGUMENT;
    if ((service == NULL) || (service->session == NULL) || (service->storage == NULL) ||
        (response == NULL) || (response_length == NULL) || (response_capacity < 1U)) return -1;
    *response_length = 0U;

    switch (command) {
    case BMS_CMD_IAP_INFO:
    case BMS_CMD_IAP_PROGRESS:
        if (response_capacity < 10U) return -1;
        response[0] = (uint8_t)BMS_STATUS_OK;
        response[1] = (uint8_t)service->session->state;
        put_u32_le(&response[2], service->session->next_offset);
        put_u32_le(&response[6], service->session->image.image_size);
        *response_length = 10U;
        return 0;
    case BMS_CMD_IAP_START:
    {
        bms_image_manifest_t image;
        if (bms_iap_manifest_decode(payload, payload_length, &image) != 0) result = BMS_IAP_ERR_MANIFEST;
        else result = bms_iap_start(service->session, service->storage, &image);
        break;
    }
    case BMS_CMD_IAP_WRITE:
        if ((payload == NULL) || (payload_length < BMS_IAP_WRITE_PREFIX_SIZE)) {
            result = BMS_IAP_ERR_CHUNK;
        } else {
            const uint32_t offset = get_u32_le(&payload[0]);
            const uint16_t declared = get_u16_le(&payload[4]);
            const uint16_t actual = (uint16_t)(payload_length - BMS_IAP_WRITE_PREFIX_SIZE);
            if ((declared != actual) || (declared == 0U)) result = BMS_IAP_ERR_CHUNK;
            else result = bms_iap_write_chunk(service->session, service->storage, offset, &payload[BMS_IAP_WRITE_PREFIX_SIZE], declared);
        }
        break;
    case BMS_CMD_IAP_VERIFY:
        if (payload_length != 0U) result = BMS_IAP_ERR_ARGUMENT;
        else result = bms_iap_verify(service->session, service->storage);
        break;
    case BMS_CMD_IAP_COMMIT:
        if (payload_length != 0U) result = BMS_IAP_ERR_ARGUMENT;
        else result = bms_iap_commit(service->session, service->storage);
        break;
    case BMS_CMD_IAP_ABORT:
        if (payload_length != 0U) result = BMS_IAP_ERR_ARGUMENT;
        else result = bms_iap_abort(service->session, service->storage);
        break;
    default:
        response[0] = (uint8_t)BMS_STATUS_INVALID_ARGUMENT;
        *response_length = 1U;
        return 0;
    }

    response[0] = result_to_status(result);
    if (response_capacity >= 2U) {
        response[1] = (uint8_t)service->session->state;
        *response_length = 2U;
    } else {
        *response_length = 1U;
    }
    return 0;
}
