# STM32F030C8 reference port

The reference F0 port uses STM32F0xx StdPeriph Library V1.5.0 vendor files and a GNU startup/linker implementation owned by this repository.

## Development

```bash
python tools/bootstrap_vendor.py
cmake --preset f030
cmake --build --preset f030
```

This builds both `bms_bootloader` and `bms_app` with `arm-none-eabi-gcc -O2` without Keil.

## Cortex-M0 APP vector handling

F030 has no Cortex-M3-style VTOR solution. The APP linker reserves 0x20000000..0x200000BF for 48 vectors. APP reset copies its vector table from `0x08003000` into SRAM and uses SYSCFG SRAM memory remap. Application `.data/.bss` therefore begin at `0x200000C0`.

Boot jumps with interrupts disabled after validating MSP/reset handler. APP startup owns vector relocation before application code enables interrupts.

## Flash safety

Platform erase/write functions only accept the APP region `0x08003000..0x0800F7FF`; the bootloader and metadata pages are not writable through the generic APP image writer. Metadata uses a separate future storage backend so write authority stays explicit.
