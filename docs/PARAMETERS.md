# Parameter system

Parameters are a stable product/domain interface, not raw C variables or protocol register aliases.

## Identity and schema

`schema/parameters.json` owns stable cross-firmware/PC/document parameter identity: ID, symbolic name, type, unit, write-access class and persistent flag. Product-specific default/min/max values and cross-field rules stay in product descriptors rather than the platform ABI catalog.

The schema generator produces C IDs, C# IDs and the generated Markdown identity table. CI rejects stale generated output, Host compiles the generated C header and Windows builds the generated C# definitions.

## Runtime descriptors and transactions

Each runtime descriptor defines ID, I32/U32/BOOL type, default, minimum, maximum, write-access mask and persistent flag. Writes occur through a caller-owned transaction: active values are copied to staged values, individual writes validate ID/type/range/permission, an optional cross-field validator sees the complete staged set, then all active values are replaced atomically. Abort or any failed validation leaves active values unchanged.

## Canonical persistence payload

`bms_parameter_persistence` defines a compiler-independent payload; the runtime `bms_param_value_t[]` array is never written raw.

Format version 1 is little-endian: an 8-byte `BPAR`/version/count header plus 8-byte `(parameter ID, type, reserved, 32-bit value)` entries. Only persistent descriptors are encoded. Unknown IDs are ignored during restore; duplicate IDs, known non-persistent IDs, known type mismatches and out-of-range values reject the payload. Restore operates on staged RAM and only replaces active values after optional cross-field validation succeeds.

Missing IDs retain the caller's existing value; startup code should load current product defaults before applying a stored payload. A new value representation or incompatible payload rule requires a persistence-format version change.

## Typed Parameter Store

`bms_parameter_store` composes the canonical Parameter codec with `bms_nvm_store`. It owns no hardware and accepts the same `bms_nvm_store_io_t` callbacks used by the Host fake Flash and STM32 platform adapters.

Load path:

`current product defaults -> NVM latest valid record -> canonical Parameter decode into staged values -> type/range/cross-field validation -> atomic active replacement`

If no valid NVM record exists, `NOT_FOUND` is distinct from corruption/I/O failure so application startup can deliberately retain defaults and log the condition rather than treating every storage problem as first boot.

Commit path:

`validated active values -> canonical Parameter encode -> NVM two-slot atomic commit -> readback/CRC verify`

The store requires an explicit `payload_limit`. Buffer capacities must meet that limit and the limit must fit the physical NVM slot after the outer 24-byte NVM header. This prevents the generic 1 KiB slot size from forcing an F030 application to reserve two near-1 KiB scratch buffers when a product has a much smaller parameter set. Product configuration will own the chosen static buffer budget.

## Access policy

`User`, `Service`, and `Factory` are write capabilities, not UI labels. Protocol handlers must authenticate/select a caller capability before invoking transaction set. Write access never implies direct Flash access.

## Verification

Host tests cover runtime transaction behavior, canonical persistence round-trip/compatibility/error atomicity and composed Parameter Store first-boot NOT_FOUND, commit/load, sequence advance, non-persistent retention and fallback to the older slot when the newest record is corrupted. O0/O2 equivalence and sanitizers execute production sources.

Status: stable Parameter ABI schema + runtime transaction core + canonical persistence codec + typed two-slot Parameter Store **Implemented**; product descriptor generation/dirty cadence and protocol Parameter handlers **Planned**.
