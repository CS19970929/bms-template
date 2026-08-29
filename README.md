# BMS Template

Clean-room reusable BMS platform for recovery IAP + BMS APP + serial/BLE PC tooling.

## Development model

VSCode + CMake + Ninja + GCC are the normal development path. Keil is a generated debug/compatibility view, not the project source of truth.

Use the unified entry point:

```bash
python tools/bms.py targets
python tools/bms.py check
python tools/bms.py build --target stm32f030c8_mock
python tools/bms.py build --target stm32f103c8_mock
python tools/bms.py keil --target stm32f030c8_mock
python tools/bms.py pc
python tools/bms.py all --target stm32f030c8_mock
```

## Configuration

A target is composition rather than a copied project:

```text
Target = MCU + AFE + Board + Product
```

Profiles live under `config/mcu`, `config/afe`, `config/boards`, `config/products`, and `config/targets`. `tools/generate_target.py` resolves a target and generates the C header, CMake metadata, GCC linker scripts and machine-readable target summary used by build and CI.

## Portable production core

Current implementation includes CRC32/frame codec, Boot image validation, redundant metadata, IAP session/service, protection primitive, MOS arbitration, scheduler, watchdog health supervisor, MCU-neutral AFE interfaces/mock, F030/F103 StdPeriph ports and a .NET 8 shared PC protocol/client with Serial/Windows BLE transport scaffolding.

Real AFE adapters are intentionally not copied from legacy BMS business projects. Hardware constants are introduced only from authoritative board/datasheet information.

## Documentation is part of the build contract

Start at `docs/README.md`. Architecture, Boot, Flash layout, protocol, protection, state machine, parameter/NVM, SOC, BLE/PC, HIL/release, AI development and ADR documents have explicit ownership. `tools/check_docs.py` enforces the required document set and PR path-to-owner-document coupling. Code must not advance while its owner specification stays stale.

## Cloud verification

GitHub Actions software admission gates include target/config generation, source/document policy, Host O0/O2 and equivalence, ASan/UBSan, Cppcheck, F030/F103 GCC O2 APP/IAP builds with generated size budgets, generated Keil project checks and Windows .NET Release/protocol smoke.

Keil/ARMCC compatibility and real-board HIL remain self-hosted gates because they require licensed/local tooling and physical hardware. Hosted green is not hardware validation.
