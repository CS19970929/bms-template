# Configuration model

A build target is composition, not a copied project:

```text
Target = MCU + AFE + Board + Product
```

## Layers

- `config/mcu/`: MCU identity, Cortex core, compiler define, clock, Flash/RAM geometry, vector relocation model, Keil device metadata and pinned vendor-library key.
- `config/afe/`: AFE capabilities and driver/bus selection. No MCU pin or product protection thresholds belong here.
- `config/boards/`: PCB wiring/topology and physical interfaces. Same-port/split-port and board features belong here.
- `config/products/`: product identity, cell count, firmware budgets/persistent-tail budget and product features.
- `config/targets/`: only references the four profiles above.

Example target:

```json
{
  "name": "stm32f030c8_mock",
  "mcu": "stm32f030c8",
  "afe": "mock",
  "board": "reference_common",
  "product": "reference_16s"
}
```

Product firmware policy supplies `boot_size` and `persistent_size`; it does not repeat derived addresses. `tools/generate_target.py` resolves MCU page geometry and creates one authoritative set of build inputs:

- `bms_target_config.h`
- `target.cmake`
- `boot.ld`
- `app.ld`
- `target-summary.json`

Boot starts at MCU Flash origin. The persistent tail is placed at Flash end. APP occupies the region between them. The current persistent contract requires at least four pages: two Boot metadata slots plus two APP NVM slots. Boot metadata are derived at the beginning of the tail and NVM slots at the end, leaving room for separately owned middle pages if a larger future persistent tail is configured.

## Adding a new product on an existing MCU family

1. Add or reuse an MCU profile.
2. Add the AFE profile/adapter if it does not exist.
3. Add the Board profile for PCB wiring.
4. Add the Product profile, including page-aligned Boot/persistent budgets.
5. Add a Target that references the four profiles.
6. Add the target to CI coverage before treating it as a supported release target.

For an already supported MCU family, CMake must not require a target-name branch. If adding a target requires editing core/protection/protocol code, the abstraction boundary is probably wrong.

## Validation

`python tools/check.py` resolves every target in `config/targets/`. Validation rejects missing references, unsupported topology/vector models, AFE cell-count incompatibility, invalid RAM reservation, Flash overlap, non-page-aligned Boot/persistent regions and a persistent tail smaller than four erase pages.

Firmware CI reads `target-summary.json` to enforce generated image budgets; workflows do not contain product-specific 12K/48K/4K constants.
