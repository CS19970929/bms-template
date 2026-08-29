#ifndef BMS_NVM_STORE_H
#define BMS_NVM_STORE_H

#include <stddef.h>
#include <stdint.h>

#define BMS_NVM_STORE_SLOT_COUNT 2U

typedef int (*bms_nvm_store_read_fn)(void *context, uint8_t slot, uint32_t offset,
                                     uint8_t *data, size_t length);
typedef int (*bms_nvm_store_erase_fn)(void *context, uint8_t slot);
typedef int (*bms_nvm_store_program_fn)(void *context, uint8_t slot, uint32_t offset,
                                        const uint8_t *data, size_t length);

typedef struct {
    void *context;
    uint32_t slot_size;
    bms_nvm_store_read_fn read;
    bms_nvm_store_erase_fn erase;
    bms_nvm_store_program_fn program;
} bms_nvm_store_io_t;

typedef struct {
    uint32_t sequence;
    uint32_t payload_length;
    uint8_t slot;
    uint8_t valid;
} bms_nvm_store_info_t;

typedef enum {
    BMS_NVM_STORE_OK = 0,
    BMS_NVM_STORE_ERR_ARGUMENT = -1,
    BMS_NVM_STORE_ERR_LAYOUT = -2,
    BMS_NVM_STORE_ERR_IO = -3,
    BMS_NVM_STORE_ERR_NOT_FOUND = -4,
    BMS_NVM_STORE_ERR_VERIFY = -5
} bms_nvm_store_result_t;

bms_nvm_store_result_t bms_nvm_store_load(const bms_nvm_store_io_t *io,
                                          uint16_t schema_version,
                                          uint32_t maximum_payload_length,
                                          uint8_t *payload_out,
                                          size_t payload_capacity,
                                          uint8_t *scratch,
                                          size_t scratch_capacity,
                                          bms_nvm_store_info_t *info_out);
bms_nvm_store_result_t bms_nvm_store_commit(const bms_nvm_store_io_t *io,
                                            uint16_t schema_version,
                                            const uint8_t *payload,
                                            size_t payload_length,
                                            uint8_t *scratch,
                                            size_t scratch_capacity,
                                            bms_nvm_store_info_t *info_out);

#endif
