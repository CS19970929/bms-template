# Parameter system

Parameters are a stable product/domain interface, not raw C variables or protocol register aliases.

## Identity and schema

`schema/parameters.json` owns stable cross-firmware/PC/document parameter identity: ID, symbolic name, type, unit, write-access class and persistent flag. Product-specific default/min/max values and cross-field rules stay in product descriptors rather than the platform ABI catalog.

The schema generator produces C IDs, C# IDs and the generated Markdown identity table. CI rejects stale generated output, Host compiles the generated C header and Windows builds the generated C# definitions.

## Runtime descriptors and transactions

Each runtime descriptor defines ID, I32/U32/BOOL type, default, minimum, maximum, write-access mask and persistent flag. Writes occur through a caller-owned transaction: active values are copied to staged values, individual writes validate ID/type/range/permission, an optional cross-field validator sees the complete staged set, then all active values are replaced atomically. Abort or any failed validation leaves active values unchanged.

## Canonical persistence payload

`bms_parameter_persistence` now defines a compiler-independent payload that can be stored inside `bms_nvm_store`; the runtime `bms_param_value_t[]` array is never written raw.

Format version 1 is little-endian:

- 8-byte header: magic `BPAR`, format version `u16`, entry count `u16`.
- each 8-byte entry: parameter ID `u16`, type `u8`, reserved `u8=0`, canonical 32-bit value.
- only descriptors marked persistent are encoded.

Encoding validates the descriptor table, duplicate/non-zero IDs, type/default validity and every persisted active value before producing bytes. Restore copies active values to caller-owned staged RAM, validates the complete payload, rejects duplicate entries, known non-persistent IDs, known type mismatches and out-of-range values, optionally runs the same cross-field validator, then replaces active values only after all checks succeed.

Unknown IDs are ignored so adding a new persistent parameter does not make an older descriptor set unreadable. Missing IDs retain the caller's existing value; startup code should therefore load current product defaults before applying a stored payload. A new value representation or incompatible payload rule requires a persistence-format version change.

## Access policy

`User`, `Service`, and `Factory` are write capabilities, not UI labels. Protocol handlers must authenticate/select a caller capability before invoking transaction set. Read visibility can be added separately; write access never implies direct Flash access.

## Integration with NVM

The parameter codec is independent of Flash and sequence/CRC. `bms_nvm_store` supplies atomic two-slot commit, sequence and outer CRC; STM32 platform callbacks supply physical erase/program. A product parameter service will compose these layers and define commit cadence, dirty tracking and startup fallback policy.

## Verification

Host tests cover runtime type/range/permission/transaction behavior plus persistence round-trip, signed I32 encoding, persistent-only output, unknown-ID compatibility, duplicate-entry rejection, invalid-value atomic rejection and encode-time value validation. O0/O2 equivalence and sanitizers execute the production codec.

Status: stable Parameter ABI schema + runtime transaction core + canonical ID-based persistence codec **Implemented**; product descriptor generation, typed NVM service/dirty policy and protocol Parameter handlers **Planned**.
