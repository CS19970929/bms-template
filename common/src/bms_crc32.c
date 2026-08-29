#include "bms_crc32.h"

uint32_t bms_crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    size_t i;
    unsigned bit;

    if ((data == NULL) && (length != 0U)) {
        return 0U;
    }

    for (i = 0U; i < length; ++i) {
        crc ^= (uint32_t)data[i];
        for (bit = 0U; bit < 8U; ++bit) {
            const uint32_t mask = (uint32_t)(0U - (crc & 1U));
            crc = (crc >> 1U) ^ (0xEDB88320UL & mask);
        }
    }
    return crc;
}

uint32_t bms_crc32(const uint8_t *data, size_t length)
{
    return bms_crc32_update(0xFFFFFFFFUL, data, length) ^ 0xFFFFFFFFUL;
}
