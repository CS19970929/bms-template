# BMS Template

Clean-room reusable BMS platform for recovery IAP + BMS APP + serial/BLE PC tooling.

## Development model

VSCode + CMake + Ninja + GCC are the normal development path. Keil is a generated debug/compatibility view, not the project source of truth.

```bash
python tools/check.py

python tools/bootstrap_vendor.py --family f0
cmake --preset f030
cmake --build --preset f030

python tools/bootstrap_vendor.py --family f1
cmake --preset f103
cmake --build --preset f103
```

Generate Keil ARMCC5 debug projects only when needed:

```bash
python tools/generate_keil.py --target stm32f030c8_mock
python tools/generate_keil.py --target stm32f103c8_mock
```

## Configuration

A target is a composition rather than a copied project:

```text
Target = MCU + AFE + Board + Product
```

Profiles live under `config/mcu`, `config/afe`, `config/boards`, `config/products`, and `config/targets`. `tools/generate_target.py` resolves a target and generates the C header, CMake metadata, GCC linker scripts and machine-readable target summary used by both build and CI.

Adding a new product on an already supported MCU family should normally require configuration and the relevant Board/AFE adapter, not a new copy of APP/IAP or a new CMake branch.

## Portable production core

The repository currently includes:

- CRC32 and transport-independent frame codec;
- boot image MCU/product/size/CRC/vector validation;
- redundant boot metadata and recovery policy;
- sequential IAP session/service with duplicate-chunk handling and final whole-image verification;
- generic protection threshold/hysteresis/delay engine;
- MOS arbitration separate from protection detection;
- cooperative scheduler and watchdog health supervisor;
- MCU-neutral AFE and AFE-bus interfaces;
- .NET 8 PC client with shared protocol over serial and Windows BLE transports.

Real AFE adapters are intentionally not copied from legacy BMS business projects. `mock` is the only AFE adapter until each real device is implemented against its authoritative datasheet/reference material.

## Cloud verification

GitHub Actions provides software admission gates:

- `quality-gate`: every target config, policy scan, O0/O2 host tests, O0/O2 equivalence, ASan/UBSan and Cppcheck;
- `firmware-build`: STM32F030C8 + STM32F103C8 GCC O2 APP/IAP builds, generated Flash-budget checks and ELF/MAP/HEX/BIN artifacts;
- `pc-build`: Windows .NET 8 Release build with warnings as errors and protocol smoke test;
- `cloud-verify`: one-click aggregate of all hosted software gates.

Keil/ARMCC compatibility and real-board HIL remain self-hosted gates because they require licensed/local tooling and physical hardware.

See `docs/ARCHITECTURE.md`, `docs/CONFIGURATION.md`, `docs/CLOUD_CI.md`, `docs/BOOTLOADER.md`, `docs/STM32F0_PORT.md`, `docs/STM32F1_PORT.md`, `docs/UPPER_COMPUTER.md` and `AGENTS.md`.
