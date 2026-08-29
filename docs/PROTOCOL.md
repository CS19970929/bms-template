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

## Service model

Planned application services: Device, Telemetry, Protection, Parameter, Control, Log, Firmware and Diagnostic. `GET_PROTOCOL_INFO` exposes protocol major/minor, firmware/product/hardware identity and feature bits so PC behavior is capability-driven rather than firmware-string hacks.

## IAP commands

Current portable Boot service provides update-session semantics equivalent to START, ERASE, WRITE, VERIFY, COMMIT, ABORT and REBOOT. Final target protocol additionally exposes Boot/device info, progress and detailed error reporting.

Repeated WRITE of an already accepted identical chunk is idempotent. Out-of-order, out-of-range, conflicting repeat or invalid-state commands fail without corrupting session state.

## Compatibility

The new protocol is authoritative. Legacy BLE/app compatibility, when implemented, lives in a separate `legacy_v1` parser/mapping adapter: `legacy frame -> legacy parser -> mapping -> new domain API`. Legacy field quirks must never leak into new domain data structures.

## Test requirements

Round-trip encode/decode, random/truncated/oversized/bad-CRC input, unknown IDs, sequence handling, fragmentation boundaries, maximum payload and cross-language firmware/.NET golden vectors are required before protocol release.
