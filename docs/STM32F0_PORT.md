# STM32F030C8 reference port

The reference F0 port uses STM32F0xx StdPeriph Library V1.5.0 vendor files and a GNU startup/linker implementation owned by this repository.

## Development

```bash
python tools/bootstrap_vendor.py --family f0
cmake --preset f030
cmake --build --preset f030
```

This builds both `bms_bootloader` and `bms_app` with `arm-none-eabi-gcc -O2` without Keil.

## Cortex-M0 APP vector handling

F030 has no Cortex-M3-style VTOR solution. The APP linker reserves `0x20000000..0x200000BF` for 48 vectors. APP reset copies its vector table from `0x08003000` into SRAM and uses SYSCFG SRAM memory remap. Application `.data/.bss` therefore begin at `0x200000C0`.

Boot jumps with interrupts disabled after validating MSP/reset handler. APP startup owns vector relocation before application code enables interrupts.

## Flash safety

The 64 KiB reference layout is Boot 12 KiB + APP 48 KiB + persistent tail 4 KiB. Generic APP image erase/write accepts only `0x08003000..0x0800EFFF`; it cannot touch the Boot or persistent tail.

Boot metadata owns `0x0800F000` and `0x0800F400`. APP NVM owns separate pages `0x0800F800` and `0x0800FC00`. `bms_nvm_stm32f0.c` exposes only `(slot, offset, length)` within these two generated NVM pages and uses STM32F0 StdPeriph V1.5.0 `FLASH_ErasePage`/`FLASH_ProgramHalfWord` with readback verification. No APP NVM operation can erase Boot metadata through this API.

Hardware brownout/power-cut behavior remains HIL evidence, not a hosted-CI claim.
