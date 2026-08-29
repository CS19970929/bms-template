# Recovery Bootloader Contract

The recovery bootloader is the software root of recoverability. APP failure must not remove the ability to reflash APP.

Rules:

- single small immutable bootloader on 64 KiB targets; no dual-IAP complexity;
- APP cannot erase/program boot flash; production uses MCU write protection where available;
- never jump based only on non-blank flash;
- validate manifest target, image bounds, CRC32, initial MSP and Thumb reset handler;
- redundant metadata records use sequence + CRC; newest valid record wins;
- invalid/interrupted image stays in recovery;
- force/recovery pin, explicit upgrade request and repeated boot failure can force recovery;
- watchdog-reset before APP health confirmation returns to recovery policy;
- upgrade transport is independent of UART/BLE/CAN implementation.

STM32F030 Cortex-M0 APP vector relocation must use the STM32F0-supported SRAM/remap mechanism; STM32F103 can use VTOR. Do not share an incorrect jump implementation between families.
