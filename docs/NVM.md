# Non-volatile storage

NVM provides atomic/versioned persistence without exposing raw STM32 Flash semantics to domain code.

## Implemented portable record core

`bms_nvm_record` defines a canonical header: magic, schema version, reserved field, sequence, payload length, payload CRC32 and header CRC32. Header CRC is calculated over explicit little-endian bytes, not compiler struct layout.

Validation rejects wrong magic/schema, oversize/truncated payload, damaged header and damaged payload. `bms_nvm_record_select` chooses the newer valid record from redundant candidates using wrap-aware sequence comparison; one corrupt candidate leaves the other selectable.

## Atomic commit contract

Physical storage is not implemented in the core. Platform/service implementation must use copy-on-write/redundant slots or wear-leveled append and never erase the only known-good committed record before a replacement is written and verified.

## Versioning/wear

Schema version remains independent of firmware version. Runtime/SOC records will declare bounded commit cadence/change threshold; high-rate tasks must not write Flash per sample/integration tick.

## Next verification

Add Host Flash backend with erase/program semantics and injected interruption after every physical write/erase/commit step, then bind to F030/F103 range-checked Flash adapters.

Status: record encode/validate/redundant selection **Implemented**; physical atomic backend/migration/wear policy **Planned**.
