# Documentation maintenance policy

Documentation is part of the production change, not post-release cleanup.

## Owner-document rule

Behaviorally meaningful changes update their owner document in the same PR. High-risk paths are strict; cosmetic/include-only fixes do not need artificial document churn when the owner spec remains exact.

Key path ownership:

- Boot and Boot-facing platform -> `BOOTLOADER.md`; layout/vector changes also `FLASH_LAYOUT.md`.
- `protocol/**` -> `PROTOCOL.md`; PC/BLE mapping docs as applicable.
- protection/MOS and Protection schema/generated outputs -> `PROTECTION.md`.
- state machine -> `STATE_MACHINE.md`.
- parameter runtime/schema/store -> `PARAMETERS.md`; NVM composition also `NVM.md` when storage invariants change.
- NVM physical backends -> `NVM.md` + MCU Port document.
- Event Log runtime/schema/generated outputs -> `EVENT_LOG.md`.
- SOC/SOH -> `SOC.md`.
- config/target/linker generation -> `CONFIGURATION.md` and `FLASH_LAYOUT.md` for memory changes.
- PC -> `UPPER_COMPUTER.md`; workflows/quality/release -> `CLOUD_CI.md`/`RELEASE.md`.
- AI/governance -> this document, `AI_DEVELOPMENT.md`, `AGENTS.md`.

## Machine checks

`tools/check_docs.py` enforces required owner docs and PR base-to-HEAD change coupling. Parameter, Protection and Event generators have deterministic stale-output checks; dedicated Host tests compile generated C headers while Windows compiles generated C# constants. Generator/schema changes are coupled to their owner docs and this policy.

## Generated outputs

Edit the owning schema/configuration, then run the supported generator (`python tools/bms.py schema` for identity schemas). Never independently edit generated C/C#/Markdown IDs, linker regions or addresses. CI rejects stale or invalid generated outputs.

## Review rule

For high-risk changes the default evidence is code + owner documentation + boundary tests. If behavior changes but a related artifact does not need to change, the PR should make that reason explicit rather than weakening the gate.
