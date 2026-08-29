# PC upper computer

The desktop tool is a .NET 8 Windows application architecture with one domain client and pluggable transports.

## Layers

- `Bms.Protocol`: frame codec, IDs, errors and protocol-version/capability model.
- `Bms.Transport`: `ITransport`; Serial and Windows BLE implementations, later CAN/TCP.
- `Bms.Device`: `BmsClient`, typed device/protection/parameter/log APIs.
- `Bms.Upgrade`: firmware package validation and Boot/IAP orchestration.
- UI: dashboard, cells/temps/current/SOC, MOS/state/protection/alarm, parameters/calibration, logs, upgrade, diagnostics and communication console.

Transport classes do not contain BMS business rules. UI does not parse frames directly.

## Upgrade flow

`select package -> validate manifest/target/checksums -> request APP enter IAP -> reconnect Boot -> query info -> START/ERASE/WRITE with progress/retry -> VERIFY -> COMMIT -> reboot -> reconnect APP -> confirm version/health`

The PC validator improves usability but is not a trust boundary; Boot validates independently.

## Diagnostics

The tool should expose raw framed logs, sequence/CRC/timeout/retry counters, connection state, device identity, reset/boot reason, firmware/build/protocol versions and IAP result.

## Current status

Implemented scaffolding: shared protocol/client, Serial transport, Windows BLE transport and cloud .NET Release/protocol smoke build. UI, full service model, firmware package and upgrade workflow are planned.
