# Non-volatile storage

NVM provides atomic/versioned persistence without exposing raw STM32 Flash semantics to domain code.

## Portable record format

`bms_nvm_record` defines a canonical 24-byte little-endian header: magic, schema version, reserved field, sequence, payload length, payload CRC32 and header CRC32. Header encoding/decoding is explicit; persistent format never depends on compiler struct packing/alignment. Header CRC covers the first 20 canonical bytes.

Validation rejects wrong magic/schema, oversize/truncated payload, damaged header and damaged payload. Sequence comparison is wrap-aware.

## Implemented redundant two-slot store

`bms_nvm_store` is a hardware-independent two-slot transactional store over `read/erase/program` callbacks. Caller supplies all payload/scratch buffers; there is no dynamic allocation.

Commit order is deliberately:

`scan latest valid -> choose inactive slot -> erase inactive -> program payload -> program CRC-protected header LAST -> read back and verify`

The header is the commit marker. The previously valid slot is never erased during replacement. A partially erased/programmed inactive slot is ignored on the next boot because header/payload validation fails. If the final header has become valid before an I/O failure is reported, reboot may select either the previous or the fully valid newer record; it must never select arbitrary partial data.

Load reads both slot headers, validates candidate payloads and selects the newest valid sequence. Transient physical read failure is surfaced as I/O error rather than silently treated as corruption/defaults.

## Power-loss simulation evidence

Host `bms_nvm_power_loss_test` uses Flash-like 1->0 programming and injects failure after each mutating phase: erase inactive slot, payload program and final header program. Starting from a known-good record, reboot/load always resolves to the previous valid payload or the fully committed new payload. The test also validates a normal second commit increments the sequence and becomes selected.

This proves the portable transaction ordering, not STM32 brownout electrical behavior. F030/F103 Flash drivers and true power-cut HIL are still required.

## Versioning/wear

Schema version remains independent of firmware version. Runtime/SOC records will declare bounded commit cadence/change threshold; high-rate tasks must not write Flash per sample/integration tick. Migrations remain explicit and host-tested.

## Next integration

Bind the store callbacks to range-checked F030/F103 Flash adapters using generated data partitions, then build typed parameter/SOC record services and power-cut HIL.

Status: canonical record codec + CRC validation + two-slot transactional store + Host mutation-fault injection **Implemented**; STM32 physical backend, schema migration, wear policy and hardware power-cut HIL **Planned/Hardware-pending**.
