# BMS Template

Clean-room reusable BMS platform for recovery IAP + BMS APP + serial/BLE PC tooling.

## Daily development

VSCode + CMake + Ninja + GCC are the normal development path. Keil is a generated debug view only.

```bash
python tools/check.py
python tools/bootstrap_vendor.py
cmake --preset f030
cmake --build --preset f030
```

Generate Keil debug projects only when needed:

```bash
python tools/generate_keil.py --target stm32f030c8_mock
```

## Current architecture

Portable production core already includes CRC32, transport-independent frame codec, boot image target/CRC/vector validation, redundant metadata, recovery policy, IAP session/service, generic protection timing/hysteresis and AFE abstraction. Host tests run the same C sources at Debug/O0, Release/O2 and ASan/UBSan.

The F030 reference target uses a pinned/verified STM32F0xx StdPeriph V1.5.0 subset, a clean platform startup/clock/flash/UART/watchdog port and a generated 64 KiB memory layout. Product target JSON generates firmware constants and linker scripts.

The .NET 8 PC tool uses a common `BmsClient` over `IBmsTransport`; serial and Windows BLE therefore share one protocol/client implementation.

See `docs/ARCHITECTURE.md`, `docs/BOOTLOADER.md`, `docs/STM32F0_PORT.md`, `docs/UPPER_COMPUTER.md` and `AGENTS.md`.
