# STM32F103C8 reference port

The second reference MCU proves the domain/IAP core is not tied to F030.

- MCU: STM32F103C8T6, explicitly treated as 64 KiB Flash.
- Vendor baseline: STM32F10x StdPeriph Library V3.5.0 subset, pinned and blob-verified from a mature repository; legacy application source is not imported.
- Clock: internal HSI/2 * 16 PLL = 64 MHz, so the template does not assume a board HSE crystal.
- APP vector relocation: Cortex-M3 `SCB->VTOR = BMS_TARGET_APP_START`; no F030 SRAM remap.
- Flash page: 1 KiB for this medium-density device.
- Same generated target memory map, recovery IAP core, frame protocol and APP core sources as F030.

## Flash/NVM safety

The 64 KiB reference layout is Boot 12 KiB + APP 48 KiB + persistent tail 4 KiB. Generic APP image writes stop before `0x0800F000`. Boot metadata owns the first two tail pages; APP NVM owns `0x0800F800` and `0x0800FC00`.

`bms_nvm_stm32f1.c` range-checks `(slot, offset, length)` against only those generated NVM pages, then uses the F10x StdPeriph `FLASH_ErasePage`/`FLASH_ProgramHalfWord` path with halfword readback verification. The physical callback compiles in GCC target CI, while actual power interruption during Flash mutation remains HIL-only evidence.

Build:

```bash
python tools/bootstrap_vendor.py --family f1
cmake --preset f103
cmake --build --preset f103
```
