# Parameter system

Parameters are typed product configuration data with stable identity, validation and persistence semantics. They are not arbitrary exposed C globals.

## Implemented core

`bms_parameter` provides descriptors with stable ID, type (`I32/U32/BOOL`), default/min/max, write-access mask and persistence flag. Caller-owned active/staged arrays keep the core allocation-free.

Transaction flow is implemented as:

`begin copy -> set(id,type,value,caller access) -> per-field validation -> optional cross-field validation -> atomic RAM commit / abort`

A failed set or cross-field validation does not partially update the active array.

## Access

Write masks distinguish User, Service and Factory capability. A descriptor with no matching caller bit is not writable through the transaction API. Read-only/telemetry identity remains outside write transactions.

## Safety rules

The generic core cannot know protection relationships; cross-field callback is the mandatory extension point for product rules such as release<trip, protection-level ordering or hardware-supported ranges.

## Next step / source of truth

Current descriptors are a production primitive, not yet the final schema. A machine-readable parameter schema will generate firmware IDs/descriptors, PC metadata and protocol tables. Independent C/C# ID lists remain forbidden.

Status: typed descriptor/transaction core and Host validation tests **Implemented**; schema generation, persistence binding and protocol services **Planned**.
