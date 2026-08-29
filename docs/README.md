# Documentation index

This directory is the maintained engineering specification for `bms-template`. Code, configuration, tests and generated artifacts must not become the only description of system behavior.

## System design

- `ARCHITECTURE.md` — repository layers, dependency direction, source-of-truth rules.
- `APP_ARCHITECTURE.md` — APP runtime composition and service boundaries.
- `BOOTLOADER.md` — recovery boot/IAP safety contract and update state machine.
- `FLASH_LAYOUT.md` — memory geometry, ownership and vector/jump rules.
- `PROTOCOL.md` — transport-independent application/IAP framing and compatibility rules.
- `PROTECTION.md` — protection detection, arbitration and MOS ownership.
- `STATE_MACHINE.md` — system states, events and transition ownership.
- `PARAMETERS.md` — parameter identity, validation, permissions and persistence model.
- `NVM.md` — atomic records, schema versioning, CRC and wear policy.
- `SOC.md` — SOC estimation/display contract and calibration boundaries.
- `BLE.md` — BLE transport, fragmentation and legacy-adapter boundary.
- `UPPER_COMPUTER.md` — PC architecture, transports and upgrade flow.
- `CONFIGURATION.md` — MCU/AFE/Board/Product/Target composition.
- `STM32F0_PORT.md`, `STM32F1_PORT.md` — MCU-specific constraints.

## Verification and release

- `CLOUD_CI.md` — hosted CI, self-hosted Keil/HIL boundary and artifacts.
- `HIL.md` — hardware-in-loop topology and mandatory cases.
- `RELEASE.md` — release evidence, manifests and release acceptance.

## Engineering governance

- `engineering/CODING_STANDARD.md` — firmware coding constraints.
- `engineering/TESTING.md` — host/unit/differential/sanitizer/fuzz/HIL test rules.
- `engineering/AI_DEVELOPMENT.md` — AI contribution workflow and Definition of Done.
- `engineering/DOCUMENTATION_POLICY.md` — documentation ownership and change-coupling rules.
- `engineering/TRACEABILITY.md` — requirement/design/code/test/evidence traceability model.
- `adr/` — accepted architecture decisions and their rationale.

## Documentation status vocabulary

Every owner document distinguishes:

- **Implemented** — production code exists and is covered by the stated automated tests.
- **Planned** — architecture is accepted but production implementation is not complete.
- **Hardware-pending** — software contract exists but real board/HIL evidence is missing.

Hosted CI green means software admission passed; it never means hardware validation or functional-safety certification is complete.
