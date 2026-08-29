#ifndef BMS_NVM_RECORD_H
#define BMS_NVM_RECORD_H

#include <stddef.h>
#include <stdint.h>

#define BMS_NVM_RECORD_MAGIC 0x424D534EUL

typedef struct {
    uint32_t magic;
    uint16_t schema_version;
    uint16_t reserved;
    uint32_t sequence;
    uint32_t payload_length;
    uint32_t payload_crc32;
    uint32_t header_crc32;
} bms_nvm_record_header_t;

typedef enum {
    BMS_NVM_OK = 0,
    BMS_NVM_ERR_ARGUMENT = -1,
    BMS_NVM_ERR_MAGIC = -2,
    BMS_NVM_ERR_SCHEMA = -3,
    BMS_NVM_ERR_LENGTH = -4,
    BMS_NVM_ERR_HEADER_CRC = -5,
    BMS_NVM_ERR_PAYLOAD_CRC = -6
} bms_nvm_result_t;

bms_nvm_result_t bms_nvm_record_prepare(bms_nvm_record_header_t *header,
                                        uint16_t schema_version,
                                        uint32_t sequence,
                                        const uint8_t *payload,
                                        size_t payload_length);
bms_nvm_result_t bms_nvm_record_validate(const bms_nvm_record_header_t *header,
                                         uint16_t expected_schema_version,
                                         const uint8_t *payload,
                                         size_t available_payload_length,
                                         uint32_t maximum_payload_length);
int bms_nvm_sequence_is_newer(uint32_t lhs, uint32_t rhs);
const bms_nvm_record_header_t *bms_nvm_record_select(const bms_nvm_record_header_t *a,
                                                     const uint8_t *payload_a,
                                                     size_t available_a,
                                                     const bms_nvm_record_header_t *b,
                                                     const uint8_t *payload_b,
                                                     size_t available_b,
                                                     uint16_t expected_schema_version,
                                                     uint32_t maximum_payload_length);

#endif
