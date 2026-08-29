#include "bms_afe_mock.h"
#include "bms_boot_image.h"
#include "bms_boot_metadata.h"
#include "bms_boot_policy.h"
#include "bms_crc32.h"
#include "bms_frame.h"
#include "bms_protection.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { printf("FAIL:%s:%d:%s\n", __FILE__, __LINE__, #x); return 1; } } while (0)

typedef struct { uint32_t base; uint8_t bytes[512]; } flash_model_t;

static int flash_read(void *ctx, uint32_t address, uint8_t *dst, size_t length)
{
    flash_model_t *flash = (flash_model_t *)ctx;
    size_t offset;
    if ((flash == NULL) || (dst == NULL) || (address < flash->base)) return -1;
    offset = (size_t)(address - flash->base);
    if ((offset > sizeof(flash->bytes)) || (length > (sizeof(flash->bytes) - offset))) return -1;
    (void)memcpy(dst, &flash->bytes[offset], length);
    return 0;
}

static void put32(uint8_t *p, uint32_t v)
{
    p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8U); p[2]=(uint8_t)(v>>16U); p[3]=(uint8_t)(v>>24U);
}

static int test_crc(void)
{
    static const uint8_t text[] = "123456789";
    CHECK(bms_crc32(text, 9U) == 0xCBF43926UL);
    return 0;
}

static int test_frame(void)
{
    uint8_t frame[64]; size_t n = 0U; bms_frame_view_t view; const uint8_t payload[] = {1U,2U,3U};
    CHECK(bms_frame_encode(1U, 7U, 0x1234U, payload, 3U, frame, sizeof(frame), &n) == BMS_FRAME_OK);
    CHECK(bms_frame_decode(frame, n, &view) == BMS_FRAME_OK);
    CHECK(view.sequence == 7U); CHECK(view.command == 0x1234U); CHECK(view.payload_length == 3U);
    frame[10] ^= 1U; CHECK(bms_frame_decode(frame, n, &view) == BMS_FRAME_ERR_CRC);
    return 0;
}

static int test_image(void)
{
    flash_model_t flash = {0}; bms_image_manifest_t m = {0}; bms_image_constraints_t c = {0};
    flash.base=0x08003000UL; put32(&flash.bytes[0],0x20002000UL); put32(&flash.bytes[4],0x08003101UL);
    flash.bytes[8]=0xAAU; flash.bytes[9]=0x55U;
    m.magic=BMS_IMAGE_MAGIC; m.manifest_version=BMS_IMAGE_MANIFEST_VERSION; m.mcu_id=BMS_MCU_STM32F030C8;
    m.product_id=42U; m.image_size=10U; m.image_crc32=bms_crc32(flash.bytes,10U);
    c.app_start=flash.base; c.app_end_exclusive=0x0800F800UL; c.ram_start=0x20000000UL; c.ram_end_exclusive=0x20002000UL;
    c.mcu_id=BMS_MCU_STM32F030C8; c.product_id=42U;
    CHECK(bms_boot_image_validate(&m,&c,flash_read,&flash)==BMS_IMAGE_OK);
    m.image_crc32 ^= 1U; CHECK(bms_boot_image_validate(&m,&c,flash_read,&flash)==BMS_IMAGE_ERR_CRC);
    return 0;
}

static int test_metadata(void)
{
    bms_boot_meta_record_t a,b; bms_image_manifest_t image = {0}; const bms_boot_meta_record_t *selected;
    image.magic=BMS_IMAGE_MAGIC; image.manifest_version=BMS_IMAGE_MANIFEST_VERSION;
    bms_boot_metadata_prepare_next(&a,NULL,BMS_BOOT_META_RECEIVING,&image);
    bms_boot_metadata_prepare_next(&b,&a,BMS_BOOT_META_READY,&image);
    selected=bms_boot_metadata_select(&a,&b); CHECK(selected==&b); CHECK(selected->sequence==2U);
    b.record_crc32 ^= 1U; CHECK(bms_boot_metadata_select(&a,&b)==&a);
    return 0;
}

static int test_policy(void)
{
    bms_boot_policy_input_t in = {0}; in.app_valid=1U; in.boot_failure_limit=3U; in.app_confirmed_healthy=1U;
    CHECK(bms_boot_policy_decide(&in)==BMS_BOOT_START_APP);
    in.upgrade_requested=1U; CHECK(bms_boot_policy_decide(&in)==BMS_BOOT_ENTER_RECOVERY);
    in.upgrade_requested=0U; in.consecutive_boot_failures=3U; CHECK(bms_boot_policy_decide(&in)==BMS_BOOT_ENTER_RECOVERY);
    return 0;
}

static int test_protection(void)
{
    bms_protection_t p; bms_protection_config_t c={3650,3600,1000U,500U,BMS_PROTECT_MODE_HIGH,1U};
    bms_protection_init(&p); CHECK(bms_protection_step(&p,&c,3649,500U)==BMS_PROTECT_NORMAL);
    CHECK(bms_protection_step(&p,&c,3650,500U)==BMS_PROTECT_PENDING);
    CHECK(bms_protection_step(&p,&c,3651,499U)==BMS_PROTECT_PENDING);
    CHECK(bms_protection_step(&p,&c,3651,1U)==BMS_PROTECT_ACTIVE);
    CHECK(bms_protection_step(&p,&c,3600,499U)==BMS_PROTECT_RECOVERING);
    CHECK(bms_protection_step(&p,&c,3600,1U)==BMS_PROTECT_NORMAL);
    return 0;
}

static int test_afe_interface(void)
{
    bms_afe_t afe; bms_afe_mock_context_t ctx = {0}; bms_afe_sample_t out;
    ctx.sample.current_ma=1234; bms_afe_mock_bind(&afe,&ctx); CHECK(afe.ops->init(&afe)==0);
    CHECK(afe.ops->sample(&afe,&out)==0); CHECK(out.current_ma==1234); CHECK(afe.ops->set_charge_fet(&afe,1U)==0); CHECK(ctx.charge_fet==1U);
    return 0;
}

int main(void)
{
    CHECK(test_crc()==0); CHECK(test_frame()==0); CHECK(test_image()==0); CHECK(test_metadata()==0);
    CHECK(test_policy()==0); CHECK(test_protection()==0); CHECK(test_afe_interface()==0);
    printf("BMS_HOST_TESTS_PASS\n");
    return 0;
}
