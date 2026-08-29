# Parameter system

Parameters are typed product configuration data with stable identity, validation and persistence semantics. They are not arbitrary exposed C globals.

## Stable identity source of truth

`schema/parameters.json` is now the single source for cross-component parameter ABI identity: numeric ID, symbolic name, type, unit, write-access class and persistence attribute. It deliberately does **not** define board/product thresholds or min/default/max values; those remain product configuration/descriptor policy so a hardware-specific value cannot silently become platform ABI.

`tools/generate_parameters.py --write` deterministically generates:

- `generated/parameters/bms_parameter_ids.h` for firmware;
- `generated/parameters/ParameterIds.g.cs` linked into `Bms.Protocol`;
- `docs/generated/PARAMETER_TABLE.md` for human/protocol review.

`tools/check.py` runs the generator in `--check` mode. Missing/stale/manual-edited generated outputs fail CI. `python tools/bms.py schema` is the supported regeneration command.

The initial stable IDs cover SOC current-floor, rest-duration and no-upward-rest-correction identity. Product-specific values are still supplied separately.

## Implemented runtime core

`bms_parameter` provides descriptors with stable ID, type (`I32/U32/BOOL`), default/min/max, write-access mask and persistence flag. Caller-owned active/staged arrays keep the core allocation-free.

Transaction flow:

`begin copy -> set(id,type,value,caller access) -> per-field validation -> optional cross-field validation -> atomic RAM commit / abort`

A failed set or cross-field validation does not partially update the active array.

## Access

Write masks distinguish User, Service and Factory capability. A descriptor with no matching caller bit is not writable through the transaction API. Read-only/telemetry identity remains outside write transactions.

## Safety rules

The generic core cannot know protection relationships; cross-field callback is the mandatory extension point for product rules such as release<trip, protection-level ordering or hardware-supported ranges.

## Evolution rules

Published IDs are append-only. Renaming a symbol does not permit ID reuse. Type/semantic incompatible changes require a new ID and, where persistent layout changes, a new NVM schema/migration. Firmware and PC must consume generated definitions rather than private copies.

Status: stable schema/generator + C/C#/Markdown drift gate + typed descriptor/transaction core + Host tests **Implemented**; product value schema, persistence binding and protocol read/write services **Planned**.
