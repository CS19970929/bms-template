# Architecture

`bms-template` is a clean-room BMS product platform, not a board-specific firmware copy.

## Stable layers

- `bootloader/core`: recovery decisions, image validation, metadata/state; no MCU registers.
- `app/core`: protection/state/domain algorithms; no MCU or AFE-specific calls.
- `protocol`: transport-independent application framing and services.
- `drivers/afe`: `bms_afe_t` interface plus per-AFE adapters.
- `platform`: MCU/CMSIS/StdPeriph/startup/flash/UART/watchdog/vector relocation.
- `config`: target = MCU + board + AFE + product policy.
- `pc`: serial/BLE transports consume the same application protocol.

Dependency direction is always hardware/UI -> interfaces -> domain core. Domain core must never include STM32 or vendor AFE headers.

## Configuration model

A product target selects four axes independently:

1. MCU family/part;
2. board wiring and polarity;
3. AFE adapter/configuration;
4. product policy (cell count, topology, protection defaults, features).

Adding another STM32/AFE should not fork protection, boot image validation, protocol, parameter schema, or PC client code.

## 64 KiB reference memory map

- 0x08000000..0x08002FFF: 12 KiB immutable recovery bootloader.
- 0x08003000..0x0800F7FF: 50 KiB APP.
- 0x0800F800..0x0800FFFF: 2 KiB redundant boot metadata / reserved NVM.

The initial recovery bootloader is not field self-upgradable. APP must never be able to erase/program outside its allowed application/NVM ranges.
