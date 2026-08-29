#include "bms_platform.h"
#include <stdint.h>
int main(void){static const uint8_t banner[]="BMS-APP START\r\n";bms_platform_app_vector_remap();bms_platform_uart_init(115200U);(void)bms_platform_uart_write(banner,sizeof(banner)-1U);bms_platform_watchdog_start();for(;;){bms_platform_watchdog_reload();}}
