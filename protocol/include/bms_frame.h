#ifndef BMS_FRAME_H
#define BMS_FRAME_H

#include <stddef.h>
#include <stdint.h>

#define BMS_FRAME_MAGIC 0xB54DU
#define BMS_FRAME_VERSION 1U
#define BMS_FRAME_HEADER_SIZE 10U
#define BMS_FRAME_CRC_SIZE 4U
#define BMS_FRAME_MAX_PAYLOAD 512U

typedef enum {
    BMS_FRAME_OK = 0,
    BMS_FRAME_ERR_ARGUMENT,
    BMS_FRAME_ERR_SHORT,
    BMS_FRAME_ERR_MAGIC,
    BMS_FRAME_ERR_VERSION,
    BMS_FRAME_ERR_LENGTH,
    BMS_FRAME_ERR_CRC
} bms_frame_result_t;

typedef struct {
    uint8_t message_type;
    uint8_t sequence;
    uint16_t command;
    const uint8_t *payload;
    uint16_t payload_length;
} bms_frame_view_t;

bms_frame_result_t bms_frame_encode(uint8_t message_type, uint8_t sequence, uint16_t command,
                                    const uint8_t *payload, uint16_t payload_length,
                                    uint8_t *out, size_t out_capacity, size_t *out_length);
bms_frame_result_t bms_frame_decode(const uint8_t *frame, size_t frame_length, bms_frame_view_t *view);

#endif
