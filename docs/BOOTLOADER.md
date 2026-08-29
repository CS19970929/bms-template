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

## IAP session invariants

The transport-independent production IAP core implements `IDLE -> ERASING -> RECEIVING -> VERIFYING -> VERIFIED -> READY` with an explicit `INVALID` path.

- START stores `RECEIVING` metadata before erasing APP. A power cut from that point cannot leave metadata that authorizes boot.
- WRITE is offset-based and bounds checked against both declared image size and APP flash range.
- only the next sequential offset is accepted; an exact retransmission of the immediately previous chunk is accepted idempotently only after flash readback matches;
- VERIFY requires every declared byte to have been received and then re-runs complete image CRC/vector/target validation from flash;
- COMMIT is the only operation that writes `READY` metadata;
- ABORT/failed validation records `INVALID`.

This means an interrupted single-slot update intentionally sacrifices the old APP contents but never sacrifices the immutable recovery path.

STM32F030 Cortex-M0 APP vector relocation must use the STM32F0-supported SRAM/remap mechanism; STM32F103 can use VTOR. Do not share an incorrect jump implementation between families.
