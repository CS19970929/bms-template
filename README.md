# BMS Template

Clean-room reusable BMS platform for recovery IAP + BMS APP + serial/BLE PC tooling.

## Development model

VSCode + CMake + Ninja + GCC are the normal development path. Keil projects are retained only as a convenient STM32 debug view; Keil is not the source of truth for source lists, memory layout or release policy.

Current production-core modules already build on the host:

- CRC32;
- transport-independent protocol frame codec;
- boot image target/CRC/vector validation;
- redundant boot metadata selection;
- boot/recovery policy;
- generic protection timing/hysteresis engine;
- AFE interface + mock implementation.

Run:

```bash
python tools/check.py
```

This performs O0/debug, O2/release, sanitizer tests (non-Windows) and Cppcheck when installed.

## Target model

Targets are composed from MCU + board + AFE + product policy. Initial 64 KiB reference targets are STM32F030C8 and STM32F103C8. Hardware startup/StdPeriph ports and Keil debug projects are intentionally isolated from the reusable core and are added from exact vendor-library baselines, not copied application architecture.

See `docs/ARCHITECTURE.md`, `docs/BOOTLOADER.md`, and `AGENTS.md`.
