# Cloud build and quality gates

GitHub-hosted runners are the default software admission environment.

## `quality-gate`

Ubuntu runner:

- resolves and validates every target composition;
- scans owned production source for forbidden dynamic allocation and suppression patterns;
- rejects `-Ofast` and `-ffast-math` in build/config scripts;
- builds and tests production core at O0;
- builds and tests the same production core at O2;
- requires byte-identical O0/O2 test output;
- runs ASan + UBSan;
- runs Cppcheck as an error gate.

## `firmware-build`

Ubuntu runner with `arm-none-eabi-gcc`:

- bootstraps the pinned, hash-verified STM32 StdPeriph/CMSIS subset;
- builds APP and IAP/Bootloader at O2 for every supported MCU target in the matrix;
- produces ELF, MAP, HEX and BIN;
- checks image size against the generated target Flash layout;
- uploads firmware and `target-summary.json` as Actions artifacts.

Current reference matrix contains STM32F030C8 and STM32F103C8.

## `pc-build`

Windows runner with .NET 8:

- restores the PC solution;
- builds Release with warnings as errors;
- runs protocol smoke tests.

## `cloud-verify`

Manual one-click aggregate workflow. It calls `quality-gate`, `firmware-build`, and `pc-build` as reusable workflows and fails unless all required software gates pass.

## What remains self-hosted

Public GitHub runners do not replace hardware-dependent validation:

- ARMCC/Keil compatibility build: use a licensed Windows self-hosted runner with Keil installed.
- HIL: use a self-hosted runner physically connected to ST-Link/J-Link, programmable power/load equipment, CAN/UART/BLE interfaces and the BMS board.

A release should eventually require:

```text
Cloud quality
  + GCC O2 target build
  + PC Release build
  + Keil compatibility build
  + HIL evidence
```

Cloud success proves the source/toolchain checks passed; it does not prove analog hardware behavior, AFE protection timing, MOS operation or brownout/power-loss behavior on a real board.
