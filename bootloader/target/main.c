#include "bms_boot_image.h"
#include "bms_boot_metadata.h"
#include "bms_boot_policy.h"
#include "bms_commands.h"
#include "bms_frame.h"
#include "bms_iap_service.h"
#include "bms_platform_stm32f0.h"
#include "bms_target_config.h"
#include <stddef.h>
#include <stdint.h>

#define RX_BUFFER_SIZE (BMS_FRAME_HEADER_SIZE + BMS_FRAME_MAX_PAYLOAD + BMS_FRAME_CRC_SIZE)

static bms_image_constraints_t constraints(void)
{
    bms_image_constraints_t c;
    c.app_start = BMS_TARGET_APP_START; c.app_end_exclusive = BMS_TARGET_APP_END;
    c.ram_start = BMS_TARGET_RAM_START; c.ram_end_exclusive = BMS_TARGET_RAM_END;
    c.mcu_id = (uint16_t)BMS_TARGET_MCU_ID; c.product_id = BMS_TARGET_PRODUCT_ID;
    return c;
}

static void send_response(const bms_frame_view_t *request, const uint8_t *payload, uint16_t payload_length)
{
    uint8_t frame[64];
    size_t frame_length = 0U;
    if ((request == NULL) || (payload_length > 48U)) return;
    if (bms_frame_encode((uint8_t)BMS_MSG_RESPONSE, request->sequence, request->command,
                         payload, payload_length, frame, sizeof(frame), &frame_length) == BMS_FRAME_OK) {
        (void)bms_platform_uart_write(frame, frame_length);
    }
}

static void process_frame(bms_iap_service_t *service, const uint8_t *frame, size_t frame_length)
{
    bms_frame_view_t view;
    uint8_t response[16];
    uint16_t response_length = 0U;
    if (bms_frame_decode(frame, frame_length, &view) != BMS_FRAME_OK) return;
    if (view.message_type != (uint8_t)BMS_MSG_REQUEST) return;
    if (view.command == (uint16_t)BMS_CMD_IAP_REBOOT) {
        response[0] = (uint8_t)BMS_STATUS_OK;
        send_response(&view, response, 1U);
        bms_platform_system_reset();
    }
    if (bms_iap_service_handle(service, view.command, view.payload, view.payload_length,
                               response, sizeof(response), &response_length) == 0) send_response(&view, response, response_length);
}

int main(void)
{
    const bms_boot_meta_record_t *a = (const bms_boot_meta_record_t *)(uintptr_t)BMS_TARGET_METADATA_A;
    const bms_boot_meta_record_t *b = (const bms_boot_meta_record_t *)(uintptr_t)BMS_TARGET_METADATA_B;
    const bms_boot_meta_record_t *meta;
    bms_boot_policy_input_t policy = {0};
    bms_image_constraints_t c = constraints();
    bms_iap_session_t session;
    bms_iap_storage_t storage = {bms_platform_flash_erase_app, bms_platform_flash_write, bms_platform_flash_read, bms_platform_metadata_store, NULL};
    bms_iap_service_t service = {&session, &storage};
    uint8_t rx[RX_BUFFER_SIZE];
    size_t rx_length = 0U;
    size_t expected_length = 0U;
    static const uint8_t recovery_msg[] = "BMS-BOOT RECOVERY\r\n";

    bms_platform_uart_init(115200U);
    bms_platform_watchdog_start();
    bms_iap_session_init(&session, &c);
    meta = bms_boot_metadata_select(a, b);
    if ((meta != NULL) && ((meta->state == (uint32_t)BMS_BOOT_META_READY) || (meta->state == (uint32_t)BMS_BOOT_META_CONFIRMED))) {
        policy.app_valid = (bms_boot_image_validate(&meta->image, &c, bms_platform_flash_read, NULL) == BMS_IMAGE_OK) ? 1U : 0U;
    }
    policy.app_confirmed_healthy = (meta != NULL && meta->state == (uint32_t)BMS_BOOT_META_CONFIRMED) ? 1U : 0U;
    policy.boot_failure_limit = 3U;
    if (bms_boot_policy_decide(&policy) == BMS_BOOT_START_APP) bms_platform_jump_to_app(BMS_TARGET_APP_START);

    (void)bms_platform_uart_write(recovery_msg, sizeof(recovery_msg) - 1U);
    for (;;) {
        uint8_t byte;
        bms_platform_watchdog_reload();
        if (bms_platform_uart_try_read(&byte) == 0) continue;
        if (rx_length == 0U) {
            if (byte != (uint8_t)(BMS_FRAME_MAGIC & 0xFFU)) continue;
            rx[rx_length++] = byte;
            continue;
        }
        if (rx_length == 1U && byte != (uint8_t)((BMS_FRAME_MAGIC >> 8U) & 0xFFU)) {
            rx_length = (byte == (uint8_t)(BMS_FRAME_MAGIC & 0xFFU)) ? 1U : 0U;
            if (rx_length == 1U) rx[0] = byte;
            continue;
        }
        if (rx_length >= sizeof(rx)) { rx_length = 0U; expected_length = 0U; continue; }
        rx[rx_length++] = byte;
        if (rx_length == BMS_FRAME_HEADER_SIZE) {
            const uint16_t payload_length = (uint16_t)((uint16_t)rx[8] | ((uint16_t)rx[9] << 8U));
            if (payload_length > BMS_FRAME_MAX_PAYLOAD) { rx_length = 0U; expected_length = 0U; continue; }
            expected_length = BMS_FRAME_HEADER_SIZE + (size_t)payload_length + BMS_FRAME_CRC_SIZE;
        }
        if ((expected_length != 0U) && (rx_length == expected_length)) {
            process_frame(&service, rx, rx_length);
            rx_length = 0U; expected_length = 0U;
        }
    }
}
