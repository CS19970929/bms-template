# Application and IAP protocol

Protocol is an application ABI independent of UART, BLE, RS485, CAN or future network transports.

## Principles

- Never expose internal C struct/register layout as wire ABI.
- Stable IDs belong to a single schema/source before multiple implementations are added.
- Every frame is length-bounded and integrity-checked before command dispatch.
- Unknown command/service IDs return an explicit error and do not alter state.
- Parser input is untrusted; truncated, oversized and malformed frames are normal test cases.

## Frame contract

The target application frame is versioned and contains at minimum magic/version, message type, sequence, command/service ID, payload length, payload and CRC32. Integer byte order is explicitly little-endian. `read_leXX`/`write_leXX` helpers are preferred over packed-struct casts.

Serial may use COBS plus a `0x00` delimiter. BLE carries the same application frame and adds only transport fragmentation/reassembly. CAN or 4G adapters must not redefine domain commands.

## Implemented service namespace

Application command space is partitioned into stable ranges:

- `0x0000..0x00FF` Device/Protocol
- `0x0100..0x01FF` Telemetry
- `0x0200..0x02FF` Protection
- `0x0300..0x03FF` Parameter
- `0x0400..0x04FF` Control
- `0x0500..0x05FF` Log
- `0x0600..0x06FF` Diagnostic
- `0x1000..` Recovery Boot/IAP, intentionally outside the APP service router

`bms_service` implements an allocation-free router with one explicitly bound handler per APP service. Dispatch validates request/response pointer-length combinations, classifies the command before handler execution, returns `BMS_STATUS_UNSUPPORTED` for unknown/unbound services and resets response length before any dispatch attempt. The router contains no domain logic; service handlers call the APP/domain APIs.

Current command identities reserve Device Info/Protocol Info/Capabilities, telemetry snapshot/cells/temperatures, protection summary/descriptors/clear-latch, parameter descriptors/read/write/commit/abort, power-path/enter-IAP control, log info/read/clear and diagnostic snapshot/reset-cause/counters.

Payload layouts are not yet frozen for all commands. A command ID being reserved does not mean its service handler or payload ABI is implemented. Each payload is specified and golden-vector tested before it becomes release ABI.

`GET_PROTOCOL_INFO` will expose protocol major/minor, firmware/product/hardware identity and feature bits so PC behavior is capability-driven rather than firmware-string hacks.

## IAP commands

Current portable Boot service provides update-session semantics equivalent to START, ERASE, WRITE, VERIFY, COMMIT, ABORT and REBOOT. Final target protocol additionally exposes Boot/device info, progress and detailed error reporting.

Repeated WRITE of an already accepted identical chunk is idempotent. Out-of-order, out-of-range, conflicting repeat or invalid-state commands fail without corrupting session state.

## Compatibility

The new protocol is authoritative. Legacy BLE/app compatibility, when implemented, lives in a separate `legacy_v1` parser/mapping adapter: `legacy frame -> legacy parser -> mapping -> new domain API`. Legacy field quirks must never leak into new domain data structures.

## Test requirements

Host tests now cover service-range classification, handler binding, successful Device/Protection dispatch, unbound service rejection, IAP exclusion from the APP router and response-length reset on rejected dispatch. Frame round-trip/bad-CRC tests remain in the core suite. Random/truncated/oversized input, sequence handling, fragmentation boundaries, maximum payload and cross-language firmware/.NET golden vectors remain required before protocol release.

Status: frame codec + command namespace + allocation-free APP service router **Implemented**; concrete APP service payloads/handlers, capability negotiation, legacy adapter and cross-language golden vectors **Planned**.
