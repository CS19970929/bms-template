# Flash and RAM layout

Memory layout is generated from target configuration. No Boot/APP/NVM address or size may be copied into production C source, Keil project settings or CI scripts as an independent constant.

## 64 KiB reference policy

Current F030C8/F103C8 reference products use 1 KiB erase pages and:

- Boot: 12 KiB, `0x08000000..0x08002FFF`.
- APP: 48 KiB, `0x08003000..0x0800EFFF`.
- Persistent tail: 4 KiB, `0x0800F000..0x0800FFFF`.
  - Boot metadata A: `0x0800F000..0x0800F3FF`.
  - Boot metadata B: `0x0800F400..0x0800F7FF`.
  - APP NVM A: `0x0800F800..0x0800FBFF`.
  - APP NVM B: `0x0800FC00..0x0800FFFF`.

The APP budget intentionally drops from 50 KiB to 48 KiB so Boot recovery state and APP atomic NVM never share an erase page. This is a safety tradeoff, not unused padding.

Exact addresses are generated per MCU page geometry by `tools/generate_target.py` and emitted into linker scripts, `bms_target_config.h`, `target.cmake` and `target-summary.json`. Product configuration owns only `boot_size` and total `persistent_size`; individual Boot metadata/NVM slots are derived from page geometry.

## Invariants

- All persistent partitions are Flash-page aligned.
- Persistent tail has at least four erase pages.
- Boot metadata A/B and APP NVM A/B are pairwise non-overlapping.
- APP linker region cannot include Boot or persistent pages.
- Generic APP image erase/program accepts only the APP image range.
- APP NVM backend accepts only generated NVM A/B pages; it cannot address Boot, APP image or Boot metadata pages.
- Boot metadata storage cannot address APP NVM pages.
- Image header load/range must fit the generated APP range.
- CI reads generated layout and rejects oversized Boot/APP binaries.

If the persistent tail later grows for an event journal, Boot metadata remain the first two persistent pages and APP NVM remain the final two pages; any middle pages are separately assigned. No two logical stores may share an erase page unless a future storage design explicitly provides atomic ownership arbitration.

## Vector handling

STM32F103C8 (Cortex-M3): APP vector base may be selected through VTOR after Boot validates vectors and deinitializes Boot-owned state.

STM32F030C8 (Cortex-M0): no Cortex-M3-style VTOR assumption is allowed. The generated target reserves SRAM vector space and the platform implementation copies/remaps vectors using the MCU-supported memory-remap mechanism.

## RAM

Generated RAM geometry must account for any reserved vector/remap region before normal APP RAM. Stack initial value and APP linker RAM must stay inside physical SRAM.

## Change rule

Changing MCU Flash/RAM geometry, Boot budget, persistent-tail size or vector reservation requires synchronized changes/evidence in `CONFIGURATION.md`, this document, target-generation tests, firmware budget CI and both Boot/APP link verification.
