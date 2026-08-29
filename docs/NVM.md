# Non-volatile storage

NVM provides atomic/versioned persistence without exposing raw STM32 Flash semantics to domain code.

## Portable record and transaction

`bms_nvm_record` defines a canonical 24-byte little-endian outer header: magic, schema version, sequence, payload length, payload CRC32 and header CRC32. `bms_nvm_store` provides a hardware-independent two-slot transaction over `read/erase/program` callbacks.

Commit order is:

`scan latest valid -> choose inactive -> erase inactive -> program payload -> program CRC-protected header LAST -> read back and verify`

The previous valid slot is never erased during replacement. The final header is the commit marker.

## Typed payload boundary

Outer NVM never serializes domain structs. Parameter persistence uses an inner canonical `BPAR` ID/type/value format. `bms_parameter_store` now composes these layers and preserves a critical distinction: no-record/first-boot is `NOT_FOUND`, while I/O/CRC/codec failures are errors. Startup policy can therefore retain defaults only under an explicit policy instead of silently converting corruption into factory defaults.

Parameter Store has an explicit `payload_limit` independent of the physical 1 KiB slot. The caller supplies static payload/scratch buffers meeting that limit; this bounds SRAM consumption on small MCUs while outer NVM still validates the chosen limit against `slot_size - 24`.

## STM32 physical backend

F030/F103 provide `bms_platform_nvm_read/erase/program` over generated, page-isolated NVM slots. The 64 KiB reference layout reserves final pages `0x0800F800` and `0x0800FC00`. Invalid slot/range cannot reach Boot, APP image or Boot metadata pages.

Both ports use family StdPeriph Flash halfword programming with readback verification and watchdog service. F030 uses STM32F0 StdPeriph V1.5.0; F103 uses the pinned F10x StdPeriph baseline.

## Evidence

Host mutation-fault tests verify the two-slot commit ordering under failures after erase, payload program and final header program. Parameter Store integration tests verify normal commit/load and selection of the older valid slot when the newest record is corrupted. Target GCC CI cross-compiles the actual F030/F103 physical callbacks and generated addresses.

This is not yet electrical brownout evidence. HIL must cut real supply during erase/payload/header phases and verify reboot recovery.

## Versioning/wear

Outer NVM schema version and inner domain payload format version are explicit and independent of firmware version. High-rate tasks must not write Flash per sample/integration tick. Product Parameter/SOC services will define dirty thresholds and minimum commit intervals.

The current two 1 KiB slots are intended for low-rate parameter/SOC state, not high-rate event history. Event Log persistence will use an append/wear-oriented journal after Flash budget and logging requirements are measured.

Status: outer record + transactional store + Host power-loss model + F030/F103 physical callbacks + canonical Parameter payload + typed Parameter Store **Implemented**; dirty/wear policy, SOC typed payload/migration and hardware power-cut HIL **Planned/Hardware-pending**.
