# Flash and RAM layout

Memory layout is generated from target configuration. No Boot/APP address or size may be copied into production C source, Keil project settings or CI scripts as an independent constant.

## 64 KiB reference policy

Current reference products use:

- Boot: 12 KiB from MCU Flash base.
- APP: 50 KiB following Boot.
- Metadata: 2 KiB at Flash tail.

Exact addresses are generated per MCU page geometry by `tools/generate_target.py` and emitted into linker scripts, `bms_target_config.h`, `target.cmake` and `target-summary.json`.

## Invariants

- All persistent partitions are Flash-page aligned.
- Boot, APP and metadata ranges never overlap.
- APP linker region cannot include Boot or metadata.
- Platform Flash APIs validate every erase/program span against the caller-owned range.
- Image header `load_address`/size must fit the generated APP range.
- CI reads generated layout and rejects oversized Boot/APP binaries.

## Vector handling

STM32F103C8 (Cortex-M3): APP vector base may be selected through VTOR after Boot validates vectors and deinitializes Boot-owned state.

STM32F030C8 (Cortex-M0): no Cortex-M3-style VTOR assumption is allowed. The generated target reserves SRAM vector space and the platform implementation copies/remaps vectors using the MCU-supported memory-remap mechanism.

## RAM

Generated RAM geometry must account for any reserved vector/remap region before normal APP RAM. Stack initial value and APP linker RAM must stay inside physical SRAM.

## Change rule

Changing MCU Flash/RAM geometry, Boot budget, metadata size or vector reservation requires synchronized changes/evidence in `CONFIGURATION.md`, this document, target-generation tests, firmware budget CI and both Boot/APP link verification.
