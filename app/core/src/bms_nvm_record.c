#include "bms_nvm_record.h"
#include "bms_crc32.h"
#include <limits.h>

static void put16(uint8_t *dst, uint16_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
}

static void put32(uint8_t *dst, uint32_t value)
{
    dst[0] = (uint8_t)value;
    dst[1] = (uint8_t)(value >> 8U);
    dst[2] = (uint8_t)(value >> 16U);
    dst[3] = (uint8_t)(value >> 24U);
}

static uint32_t header_crc(const bms_nvm_record_header_t *header)
{
    uint8_t bytes[20];
    put32(&bytes[0], header->magic);
    put16(&bytes[4], header->schema_version);
    put16(&bytes[6], header->reserved);
    put32(&bytes[8], header->sequence);
    put32(&bytes[12], header->payload_length);
    put32(&bytes[16], header->payload_crc32);
    return bms_crc32(bytes, sizeof(bytes));
}

bms_nvm_result_t bms_nvm_record_prepare(bms_nvm_record_header_t *header,
                                        uint16_t schema_version,
                                        uint32_t sequence,
                                        const uint8_t *payload,
                                        size_t payload_length)
{
    if ((header == NULL) || ((payload == NULL) && (payload_length != 0U)) ||
        (payload_length > (size_t)UINT32_MAX)) {
        return BMS_NVM_ERR_ARGUMENT;
    }
    header->magic = BMS_NVM_RECORD_MAGIC;
    header->schema_version = schema_version;
    header->reserved = 0U;
    header->sequence = sequence;
    header->payload_length = (uint32_t)payload_length;
    header->payload_crc32 = bms_crc32(payload, payload_length);
    header->header_crc32 = header_crc(header);
    return BMS_NVM_OK;
}

bms_nvm_result_t bms_nvm_record_validate(const bms_nvm_record_header_t *header,
                                         uint16_t expected_schema_version,
                                         const uint8_t *payload,
                                         size_t available_payload_length,
                                         uint32_t maximum_payload_length)
{
    if ((header == NULL) || ((payload == NULL) && (available_payload_length != 0U))) {
        return BMS_NVM_ERR_ARGUMENT;
    }
    if (header->magic != BMS_NVM_RECORD_MAGIC) {
        return BMS_NVM_ERR_MAGIC;
    }
    if (header->schema_version != expected_schema_version) {
        return BMS_NVM_ERR_SCHEMA;
    }
    if ((header->payload_length > maximum_payload_length) ||
        ((size_t)header->payload_length > available_payload_length)) {
        return BMS_NVM_ERR_LENGTH;
    }
    if (header->header_crc32 != header_crc(header)) {
        return BMS_NVM_ERR_HEADER_CRC;
    }
    if (header->payload_crc32 != bms_crc32(payload, (size_t)header->payload_length)) {
        return BMS_NVM_ERR_PAYLOAD_CRC;
    }
    return BMS_NVM_OK;
}

int bms_nvm_sequence_is_newer(uint32_t lhs, uint32_t rhs)
{
    if (lhs == rhs) {
        return 0;
    }
    return ((uint32_t)(lhs - rhs) < 0x80000000UL) ? 1 : 0;
}

const bms_nvm_record_header_t *bms_nvm_record_select(const bms_nvm_record_header_t *a,
                                                     const uint8_t *payload_a,
                                                     size_t available_a,
                                                     const bms_nvm_record_header_t *b,
                                                     const uint8_t *payload_b,
                                                     size_t available_b,
                                                     uint16_t expected_schema_version,
                                                     uint32_t maximum_payload_length)
{
    const int valid_a = (bms_nvm_record_validate(a, expected_schema_version, payload_a, available_a,
                                                  maximum_payload_length) == BMS_NVM_OK) ? 1 : 0;
    const int valid_b = (bms_nvm_record_validate(b, expected_schema_version, payload_b, available_b,
                                                  maximum_payload_length) == BMS_NVM_OK) ? 1 : 0;
    if ((valid_a == 0) && (valid_b == 0)) {
        return NULL;
    }
    if (valid_a == 0) {
        return b;
    }
    if (valid_b == 0) {
        return a;
    }
    return (bms_nvm_sequence_is_newer(b->sequence, a->sequence) != 0) ? b : a;
}
