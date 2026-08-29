# Configuration model

A build target is composition, not a copied project:

```text
Target = MCU + AFE + Board + Product
```

## Layers

- `config/mcu/`: MCU identity, Cortex core, compiler define, clock, Flash/RAM geometry, vector relocation model, Keil device metadata and pinned vendor-library key.
- `config/afe/`: AFE capabilities and driver/bus selection. No MCU pin or product protection thresholds belong here.
- `config/boards/`: PCB wiring/topology and physical interfaces. Same-port/split-port and board features belong here.
- `config/products/`: product identity, cell count, firmware region budgets and product features.
- `config/targets/`: only references the four profiles above.

Example:

```json
{
  "name": "stm32f030c8_mock",
  "mcu": "stm32f030c8",
  "afe": "mock",
  "board": "reference_common",
  "product": "reference_16s"
}
```

`tools/generate_target.py` resolves the references and generates one authoritative set of build inputs:

- `bms_target_config.h`
- `target.cmake`
- `boot.ld`
- `app.ld`
- `target-summary.json`

Flash regions are derived rather than repeated in multiple files. Boot starts at MCU Flash origin; metadata is placed at the end of MCU Flash; APP occupies the region between them. Boot and metadata sizes come from the Product profile and must align to the MCU erase-page size.

## Adding a new product on an existing MCU family

1. Add or reuse an MCU profile.
2. Add the AFE profile/adapter if it does not exist.
3. Add the Board profile for PCB wiring.
4. Add the Product profile.
5. Add a Target that references the four profiles.
6. Add the target to CI coverage before treating it as a supported release target.

For an already supported MCU family, CMake must not require a target-name branch. If adding a target requires editing core/protection/protocol code, the abstraction boundary is probably wrong.

## Validation

`python tools/check.py` resolves every target in `config/targets/`. Validation rejects missing references, unsupported topology/vector models, AFE cell-count incompatibility, invalid RAM reservation, Flash overlap, and non-page-aligned Boot/metadata regions.

Firmware CI reads `target-summary.json` to enforce the generated image budgets; the workflow does not contain product-specific 12K/50K constants.
