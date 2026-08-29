# Application and IAP protocol

Protocol is an application ABI independent of UART, BLE, RS485, CAN or future network transports.

## Principles

- Never expose internal C struct/register layout as wire ABI.
- Stable command/parameter/protection/event identities have one schema owner.
- Every frame is length-bounded and integrity-checked before dispatch.
- Unknown command/service IDs return an explicit error and do not alter state.
- Parser input is untrusted; truncated/oversized/malformed frames are required tests.

## Command ID single source

`schema/commands.json` is now the authoritative Command ID catalog. It assigns each command to a service range and the generator rejects IDs outside that range or duplicate IDs/names. `tools/generate_commands.py` emits firmware `bms_command_ids.h`, .NET `CommandIds.g.cs` and `docs/generated/COMMAND_TABLE.md`.

`bms_commands.h` retains only message-type and status-code enums and includes the generated command constants. No firmware or PC component should independently reproduce command numbers. Command renumbering is an ABI break requiring compatibility review.

## Frame contract

The application frame is versioned and contains magic/version, message type, sequence, command ID, payload length, payload and CRC32 in explicit little-endian representation. Serial may add COBS+delimiter; BLE adds only fragmentation/reassembly. CAN/4G adapters do not redefine domain commands.

## Service namespace

- `0x0000..0x00FF` Device/Protocol
- `0x0100..0x01FF` Telemetry
- `0x0200..0x02FF` Protection
- `0x0300..0x03FF` Parameter
- `0x0400..0x04FF` Control
- `0x0500..0x05FF` Log
- `0x0600..0x06FF` Diagnostic
- `0x1000..0x10FF` Recovery Boot/IAP, outside the APP service router

`bms_service` is an allocation-free APP router. It validates pointer/length pairs, classifies a command before handler execution and returns `UNSUPPORTED` for unknown/unbound services. Concrete handlers call domain APIs; the router contains no protection/NVM logic.

Reserved command IDs now cover Device/Protocol/Capabilities, telemetry, Protection summary/descriptors/latch clear, Parameter descriptors/read/write/commit/abort, Control, Log, Diagnostic and IAP session operations. Reservation does not mean a payload ABI is frozen; each concrete payload must be field-specified and cross-language golden-vector tested before release.

## Compatibility

The new protocol is authoritative. Legacy BLE compatibility, when implemented, remains a separate adapter: `legacy frame -> legacy parser -> mapping -> new domain API`. Legacy quirks never leak into the new wire/domain model.

## Verification

Quality gates stale-check Command/Parameter/Protection/Event generated outputs. Host compiles generated C Command IDs; Windows compiles generated C# IDs. Router tests cover service classification/binding/rejection. Frame round-trip/CRC exists. Concrete payload golden vectors, fuzzing, fragmentation and capability negotiation remain required.

Status: frame codec + command single-source schema + command namespace + allocation-free service router **Implemented**; frozen Device/Telemetry/Protection/Parameter/Log/Diagnostic payloads and legacy adapter **Planned**.
