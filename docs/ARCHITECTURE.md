# Architecture

`bms-template` is a clean-room BMS product platform, not a board-specific firmware copy.

## Stable layers

- `bootloader/core`: recovery decisions, image validation, IAP session/metadata; no MCU registers.
- `app/core`: protection/state/domain algorithms; no MCU or AFE-specific calls.
- `protocol`: transport-independent application framing and services.
- `drivers/afe`: `bms_afe_t` interface plus per-AFE adapters.
- `platform`: MCU/CMSIS/StdPeriph/startup/flash/UART/watchdog/vector relocation.
- `config`: target = MCU + board + AFE + product policy.
- `pc`: serial/BLE transports consume the same application protocol.

Dependency direction is hardware/UI -> interfaces -> domain core. Domain core must never include STM32 or vendor AFE headers.

## Configuration model

A product target selects four axes independently: MCU family/part, board wiring/polarity, AFE adapter/configuration, and product policy. `config/targets/*.json` is machine readable. `tools/generate_target.py` validates a target and generates the C target header and GNU linker scripts; address constants must not be copied manually into application/boot source.

Adding another STM32/AFE should not fork protection, boot image validation, protocol, parameter schema, or PC client code.

## 64 KiB reference memory map

The initial reference target config defines 12 KiB immutable recovery bootloader, 50 KiB APP, and 2 KiB redundant boot metadata. F030 APP generation reserves the first 0xC0 bytes of SRAM for Cortex-M0 vector remap automatically.
