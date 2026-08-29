# Keil debug view

Keil is intentionally not the project source of truth. Use VSCode/CMake/GCC for normal development, CI and release builds.

Generate F030 projects from the active CMake source graph:

```bash
python tools/generate_keil.py --target stm32f030c8_mock
```

Outputs:

- `ide/keil/generated/stm32f030c8_mock/bms_boot.uvprojx`
- `ide/keil/generated/stm32f030c8_mock/bms_app.uvprojx`

Each generated project contains `Debug-O0` and `Debug-O2` ARMCC5 targets. The generator reads CMake-produced source manifests, so adding/removing production sources is done in CMake, not by hand in uVision.

The ARMCC startup is maintained separately because GCC attributes/startup syntax is toolchain-specific. It only provides reset/vector mechanics and references the same platform/core C sources. No legacy BMS code or architecture is imported.
