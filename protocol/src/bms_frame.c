#include "bms_frame.h"
#include "bms_crc32.h"

static void put_u16_le(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static void put_u32_le(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
    dst[2] = (uint8_t)((value >> 16U) & 0xFFU);
    dst[3] = (uint8_t)((value >> 24U) & 0xFFU);
}

static uint16_t get_u16_le(const uint8_t *src)
{
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8U));
}

static uint32_t get_u32_le(const uint8_t *src)
{
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8U) | ((uint32_t)src[2] << 16U) | ((uint32_t)src[3] << 24U);
}

bms_frame_result_t bms_frame_encode(uint8_t message_type, uint8_t sequence, uint16_t command,
                                    const uint8_t *payload, uint16_t payload_length,
                                    uint8_t *out, size_t out_capacity, size_t *out_length)
{
    size_t total;
    size_t i;
    uint32_t crc;

    if ((out == NULL) || (out_length == NULL) || ((payload == NULL) && (payload_length != 0U))) {
        return BMS_FRAME_ERR_ARGUMENT;
    }
    if (payload_length > BMS_FRAME_MAX_PAYLOAD) {
        return BMS_FRAME_ERR_LENGTH;
    }
    total = BMS_FRAME_HEADER_SIZE + (size_t)payload_length + BMS_FRAME_CRC_SIZE;
    if (out_capacity < total) {
        return BMS_FRAME_ERR_LENGTH;
    }

    put_u16_le(&out[0], BMS_FRAME_MAGIC);
    out[2] = BMS_FRAME_VERSION;
    out[3] = message_type;
    out[4] = sequence;
    out[5] = 0U;
    put_u16_le(&out[6], command);
    put_u16_le(&out[8], payload_length);
    for (i = 0U; i < (size_t)payload_length; ++i) {
        out[BMS_FRAME_HEADER_SIZE + i] = payload[i];
    }
    crc = bms_crc32(out, BMS_FRAME_HEADER_SIZE + (size_t)payload_length);
    put_u32_le(&out[BMS_FRAME_HEADER_SIZE + (size_t)payload_length], crc);
    *out_length = total;
    return BMS_FRAME_OK;
}

bms_frame_result_t bms_frame_decode(const uint8_t *frame, size_t frame_length, bms_frame_view_t *view)
{
    uint16_t payload_length;
    size_t expected;
    uint32_t expected_crc;
    uint32_t actual_crc;

    if ((frame == NULL) || (view == NULL)) {
        return BMS_FRAME_ERR_ARGUMENT;
    }
    if (frame_length < (BMS_FRAME_HEADER_SIZE + BMS_FRAME_CRC_SIZE)) {
        return BMS_FRAME_ERR_SHORT;
    }
    if (get_u16_le(&frame[0]) != BMS_FRAME_MAGIC) {
        return BMS_FRAME_ERR_MAGIC;
    }
    if (frame[2] != BMS_FRAME_VERSION) {
        return BMS_FRAME_ERR_VERSION;
    }
    payload_length = get_u16_le(&frame[8]);
    if (payload_length > BMS_FRAME_MAX_PAYLOAD) {
        return BMS_FRAME_ERR_LENGTH;
    }
    expected = BMS_FRAME_HEADER_SIZE + (size_t)payload_length + BMS_FRAME_CRC_SIZE;
    if (frame_length != expected) {
        return BMS_FRAME_ERR_LENGTH;
    }
    expected_crc = get_u32_le(&frame[BMS_FRAME_HEADER_SIZE + (size_t)payload_length]);
    actual_crc = bms_crc32(frame, BMS_FRAME_HEADER_SIZE + (size_t)payload_length);
    if (expected_crc != actual_crc) {
        return BMS_FRAME_ERR_CRC;
    }

    view->message_type = frame[3];
    view->sequence = frame[4];
    view->command = get_u16_le(&frame[6]);
    view->payload = &frame[BMS_FRAME_HEADER_SIZE];
    view->payload_length = payload_length;
    return BMS_FRAME_OK;
}
