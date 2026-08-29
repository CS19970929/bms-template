# Documentation maintenance policy

Documentation is part of the production change, not post-release cleanup.

## Owner-document rule

A behaviorally meaningful production change must update its owner document in the same PR unless the document remains exactly correct and the automated change-coupling rule does not require an update. High-risk paths are intentionally strict.

Path -> required owner documentation:

- `bootloader/**`, Boot-facing `platform/**` -> `BOOTLOADER.md`; layout/vector changes also `FLASH_LAYOUT.md`.
- `protocol/**` -> `PROTOCOL.md`; PC protocol mapping also `UPPER_COMPUTER.md`/`BLE.md` as applicable.
- `app/core/*protection*`, `*mos*`, `schema/protections.json`, protection generator/generated outputs -> `PROTECTION.md`.
- state-machine files -> `STATE_MACHINE.md`.
- parameter runtime files, `schema/parameters.json`, parameter generator/generated outputs -> `PARAMETERS.md`.
- NVM/persistence files -> `NVM.md`; STM32 physical NVM backends also update the family Port document.
- event-log runtime/persistence files -> `EVENT_LOG.md`; persistence-backend changes also update `NVM.md` when storage invariants change.
- SOC/SOH files -> `SOC.md`.
- `config/**`, target generator/linker generation -> `CONFIGURATION.md` and `FLASH_LAYOUT.md` when memory geometry changes.
- BLE transport/legacy adapter -> `BLE.md`.
- PC architecture/upgrade/generated ABI consumption -> `UPPER_COMPUTER.md`.
- workflows/quality/release tooling -> `CLOUD_CI.md` or `RELEASE.md`.
- AI/governance rules -> this document/`AI_DEVELOPMENT.md`/`AGENTS.md`.

## Required content of an owner document

Purpose, boundaries/ownership, stable data/state model, invariants, failure behavior, configuration/schema source, implemented vs planned status, tests/evidence and hardware-pending limitations.

## Machine checks

`tools/check_docs.py` verifies the required document set, repository links and PR change coupling. CI fetches PR history sufficiently to compare against the base SHA. Parameter and protection schema generators have independent deterministic stale-output checks, and dedicated Host tests compile the generated C headers. STM32 NVM backend paths are explicitly coupled to both `NVM.md` and their MCU Port owner document. These checks may not be bypassed by warning suppression or test deletion.

## Generated documentation/code

Edit the owning schema/configuration, then run the supported generator. Do not independently edit generated Markdown/C#/C tables, IDs, linker regions or addresses. CI rejects stale generated outputs and invalid target layouts. A generator/schema/layout change is itself a behavior/ABI change and is coupled to the corresponding owner documents.

## Review rule

If code behavior changes but no documentation/schema/test change is expected, the PR must explain why. The default assumption for high-risk modules is that behavior and evidence documents must change together.
