# Non-volatile storage

NVM provides atomic/versioned persistence without exposing raw STM32 Flash semantics to domain code.

## Portable record format

`bms_nvm_record` defines a canonical 24-byte little-endian outer header: magic, schema version, reserved field, sequence, payload length, payload CRC32 and header CRC32. Header encoding/decoding is explicit; persistent format never depends on compiler struct packing/alignment. Header CRC covers the first 20 canonical bytes.

Validation rejects wrong magic/schema, oversize/truncated payload, damaged header and damaged payload. Sequence comparison is wrap-aware.

## Redundant two-slot store

`bms_nvm_store` is a hardware-independent two-slot transactional store over `read/erase/program` callbacks. Caller supplies all payload/scratch buffers; there is no dynamic allocation.

Commit order is deliberately:

`scan latest valid -> choose inactive slot -> erase inactive -> program payload -> program CRC-protected header LAST -> read back and verify`

The header is the commit marker. The previously valid slot is never erased during replacement. A partially erased/programmed inactive slot is ignored on the next boot because header/payload validation fails.

## Typed payload boundary

The NVM outer record does not serialize domain structs. Parameter persistence now uses `bms_parameter_persistence`: an ID/type/value little-endian payload with its own format version. This lets NVM retain atomicity/CRC/sequence responsibilities while the Parameter owner controls schema evolution and default/range/cross-field validation. SOC and future stores will define their own canonical payloads rather than casting RAM structs to Flash bytes.

## STM32 physical backend

F030/F103 provide `bms_platform_nvm_read/erase/program` callbacks over two generated, page-isolated NVM slots. The portable store remains unaware of STM32 registers or StdPeriph.

For the 64 KiB reference layout the NVM slots are the final two 1 KiB pages: `0x0800F800` and `0x0800FC00`. Addressing is `(slot, offset, length)` and is range-checked against one generated slot only. Invalid slot/range cannot reach Boot, APP image or Boot metadata pages.

Both ports use family StdPeriph Flash and halfword programming. Program start must be halfword aligned; odd payload length is completed with erased `0xFF` in the unused high byte. Each halfword is read back before success is returned. Erase/program service the watchdog during Flash mutation.

## Power-loss evidence

Host `bms_nvm_power_loss_test` uses Flash-like 1->0 programming and injects failure after erase inactive slot, payload program and final header program. Starting from a known-good record, reboot/load always resolves to the previous valid payload or the fully committed new payload.

Target GCC CI compiles the actual F030/F103 physical callbacks and generated addresses. This proves source/layout integration but not brownout electrical behavior. True power-cut HIL must still cut supply during erase, payload programming and header commit and verify recovery after reboot.

## Versioning/wear

Outer NVM schema version and inner domain payload format version are explicit and independent of firmware version. Runtime/SOC records will declare bounded commit cadence/change threshold; high-rate tasks must not write Flash per sample/integration tick. Migrations remain explicit and host-tested.

The current two 1 KiB slots are suitable for low-rate parameter/SOC state, not high-rate event history. Event Log uses a separate RAM model and will receive an append/wear-oriented persistent journal only after Flash budget and product logging requirements are measured.

Status: canonical outer record + CRC + transactional two-slot store + Host mutation-fault injection + F030/F103 physical callbacks + canonical Parameter payload **Implemented**; composed typed store service, SOC payload/migration policy and hardware power-cut HIL **Planned/Hardware-pending**.
