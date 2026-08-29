# Architecture

`bms-template` is a clean-room reusable BMS product platform, not a board-specific firmware copy.

## Source of truth

1. `config/` owns product composition and memory-policy inputs.
2. `tools/generate_target.py` owns generated target constants/linker layout.
3. CMake owns the production source graph and GCC release build.
4. Keil projects are generated compatibility/debug views; they must not carry independent source lists or memory definitions.
5. Protocol/parameter/alarm/protection identities must have one machine-readable owner before multiple firmware/PC implementations exist.
6. Markdown owner documents describe behavior, invariants, failure semantics and validation evidence; they are required companions to code changes.

## Stable layers

- `bootloader/core`: recovery decisions, image validation, metadata and IAP session; no MCU registers.
- `app/core`: protection, state, parameter, NVM/SOC/domain algorithms; no STM32 or concrete AFE headers.
- `protocol`: transport-independent framing and services.
- `drivers/afe`: `bms_afe_t` interface plus per-AFE adapters.
- `platform`: MCU startup, StdPeriph/CMSIS, flash/UART/watchdog/vector implementation.
- `config`: `Target = MCU + AFE + Board + Product`.
- `pc`: one `BmsClient` over serial/BLE and future transports.
- `tests`: host production-core tests and later HIL orchestration.

Dependency direction is hardware/UI -> interfaces -> domain core. Domain code must not call MCU, transport or AFE implementation APIs directly.

## Ownership boundaries

- Protection **detects** and publishes block reasons; it never toggles FETs.
- MOS policy is the only domain arbiter of charge/discharge permission; hardware adapter applies the decision.
- APP APIs cannot erase/write Boot address ranges.
- Boot validates an APP independently of the PC package validator.
- Communication code cannot expose internal C struct layout as protocol ABI.
- Watchdog reload represents system health, not merely timer interrupt activity.

## Product composition

A target independently selects MCU part, AFE adapter/configuration, board wiring/polarity and product policy. Adding a product on supported families should normally add configuration and the needed adapters, not fork APP/IAP/protocol/SOC/protection code.

## Current implementation status

Implemented: portable Boot core, image validation, redundant metadata, IAP session/service, frame codec, protection primitive, MOS arbitration, scheduler, watchdog supervisor, AFE abstraction/mock, F030/F103 target ports, generated layout, .NET protocol/client serial/BLE transport scaffolding and cloud software gates.

Planned next: system state machine, parameter service/schema, NVM record layer, SOC service, event log, complete application protocol services and first real AFE adapter.

Hardware-pending: real AFE/MOS behavior, analog scaling, brownout/power-interruption validation, real transport timing and HIL.
