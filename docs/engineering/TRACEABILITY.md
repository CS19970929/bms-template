# Engineering traceability

The platform maintains a lightweight trace from requirement/policy to design, implementation, verification and release evidence.

## Trace item

Each safety/behavioral feature eventually receives a stable ID (examples: `BOOT-RECOVERY-001`, `PROT-OV-001`, `NVM-ATOMIC-001`). The trace records:

`requirement -> owner document/ADR -> configuration/schema -> production symbols/files -> host tests -> target/HIL tests -> release evidence`

## Initial trace groups

- Boot recovery/image validation/update interruption: `BOOTLOADER.md`, Boot core, host Boot tests, IAP HIL.
- Flash ownership/layout/vector jump: `FLASH_LAYOUT.md`, target generator/platform, target build/link checks, HIL boot/jump cases.
- Protection/MOS separation: `PROTECTION.md`, protection/MOS core, host boundary tests, real AFE/MOS HIL.
- Watchdog health: `APP_ARCHITECTURE.md`, supervisor/platform IWDG, starvation tests, HIL reset/recovery.
- Protocol parser integrity: `PROTOCOL.md`, frame/service code, fuzz/golden tests, serial/BLE HIL.
- Parameter/NVM atomicity: `PARAMETERS.md`/`NVM.md`, schema/service/storage, corruption/power-loss simulation and HIL.

## Rule

Tests must reference or clearly map to trace IDs once the corresponding feature is release-relevant. Release reports list which required trace groups have hosted-only, self-hosted or hardware evidence.
