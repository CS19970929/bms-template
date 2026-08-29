# Documentation maintenance policy

Documentation is part of the production change, not post-release cleanup.

Behaviorally meaningful changes update their owner document in the same PR. High-risk paths are strict; cosmetic/include-only fixes do not need artificial document churn when the owner spec remains exact.

Key ownership includes Boot->`BOOTLOADER.md`, protocol/Command schema->`PROTOCOL.md`, protection/MOS->`PROTECTION.md`, state->`STATE_MACHINE.md`, Parameter schema/store->`PARAMETERS.md`, NVM/physical backend->`NVM.md` plus MCU Port, Event runtime/schema->`EVENT_LOG.md`, SOC->`SOC.md`, config/layout->`CONFIGURATION.md`/`FLASH_LAYOUT.md`, PC->`UPPER_COMPUTER.md`, CI/release->`CLOUD_CI.md`/`RELEASE.md`.

`tools/check_docs.py` enforces required documents and PR base-to-HEAD path coupling. Command, Parameter, Protection and Event generators have deterministic stale-output checks. Host compiles generated C identities and Windows compiles generated C# identities.

Edit the owning schema/configuration, then regenerate. `python tools/bms.py schema` regenerates all stable identity catalogs. Do not hand-edit generated C/C#/Markdown IDs or generated linker addresses.

For high-risk changes the default evidence is code + owner doc + boundary tests. If behavior changes without a related artifact change, the PR should explain why rather than weakening the gate.
