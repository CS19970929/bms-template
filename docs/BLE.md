# BLE transport

BLE is a transport for the same application protocol used by serial; it is not a second BMS protocol/domain model.

## New transport

The PC BLE transport performs scan/filter/connect/service discovery, notification subscription, MTU-aware fragmentation/reassembly, timeout/reconnect and diagnostics. Reassembled bytes are passed to the same `BmsClient`/protocol parser as serial.

Application sequence, length and CRC checks remain active over BLE; BLE link integrity is not treated as replacement for application framing.

## Device-side boundary

A BLE MCU/module or integrated BLE stack exposes a byte/frame transport adapter. It may cache connection/MTU state but must not own protection/parameter semantics.

## Legacy compatibility

Legacy mobile applications are supported only through an isolated compatibility adapter where practical. Exact old frame fields must be obtained from authoritative old code/protocol documentation and classified as Supported, Mapped, Deprecated or Unsupported. Missing legacy behavior is never guessed.

`legacy frame -> legacy parser -> mapping -> new domain service`

New domain/protocol IDs and semantics must not be distorted to preserve undocumented legacy quirks.

## Verification

Golden frames shared with .NET, fragmentation at every MTU boundary, combined/split notifications, reconnect mid-frame, duplicate notification, malformed length/CRC and capability negotiation are required.
