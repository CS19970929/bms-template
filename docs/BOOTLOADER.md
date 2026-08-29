# Recovery Bootloader / IAP

The Bootloader is the highest-safety software component. Its primary contract is: **if Boot code/Flash remains physically intact and the MCU can execute, an invalid, crashed or partially updated APP must not remove the ability to reflash APP.**

## Architecture decision

For 64 KiB F030C8/F103C8 targets use one small protected Recovery Boot, not dual IAP. Boot V1 is not remotely self-updated. Boot replacement is a factory/service operation through SWD or a separately designed future mechanism.

## Flash ownership

- Boot owns its immutable code range and boot metadata backend.
- APP owns only APP/data ranges exposed by platform APIs.
- Every erase/program API must range-check against generated layout.
- APP has no API capable of modifying Boot.

See `FLASH_LAYOUT.md`.

## APP validity before jump

Boot must reject APP unless all required checks pass: target/product identity, header/version, declared image length and address range, payload CRC32, vector location, initial MSP in valid SRAM, reset handler in APP Flash and Thumb state. A failed check enters Recovery; Boot never jumps optimistically.

F030 Cortex-M0 and F103 Cortex-M3 use different vector relocation mechanisms; the platform layer owns that distinction.

## Update state machine

`IDLE -> ENTER_UPDATE -> ERASING -> RECEIVING -> VERIFYING -> READY/INVALID -> COMMIT/RECOVERY`

Current portable IAP core implements START/ERASE/WRITE/VERIFY/COMMIT/ABORT/REBOOT semantics with ordered range checking, duplicate-chunk idempotency, Flash readback and whole-image verification.

## Metadata

Boot metadata is redundant and sequence-numbered. A record is accepted only if its own integrity check is valid. Power loss during a metadata update must leave either the previous record or the new record selectable; an ambiguous/invalid update falls back to Recovery rather than APP jump.

## Reset/watchdog policy

Planned target policy records boot attempts and APP healthy confirmation. Repeated watchdog/startup failures may force Recovery. Single-slot 64 KiB products do not promise old-image rollback; they promise recovery reflashing.

## Transport independence

IAP session/service owns update semantics and must not know UART/BLE/CAN. Transport adapters provide framed bytes and reconnect behavior. Host tooling may validate packages, but Boot performs its own independent validation.

## Required verification

Host: image boundary tests, malformed headers, CRC/vector failures, duplicate/reordered/out-of-range chunks, metadata corruption and simulated power interruption at every erase/write/commit boundary.

HIL: valid/invalid APP boot, interrupted erase/write/verify/commit, repeated reset/watchdog, forced-recovery entry, reconnect/update and proof that Boot remains reflashing-capable.
