# PC upper computer

The desktop tool is a .NET 8 Windows application architecture with one domain client and pluggable transports.

## Layers

- `Bms.Protocol`: frame codec, generated stable IDs, errors and protocol-version/capability model.
- `Bms.Transport`: `ITransport`; Serial and Windows BLE implementations, later CAN/TCP.
- `Bms.Client`/device layer: `BmsClient`, typed device/protection/parameter/log APIs.
- future `Bms.Upgrade`: firmware package validation and Boot/IAP orchestration.
- UI: dashboard, cells/temps/current/SOC, MOS/state/protection/alarm, parameters/calibration, logs, upgrade, diagnostics and communication console.

Transport classes do not contain BMS business rules. UI does not parse frames directly.

## Generated protocol metadata

`Bms.Protocol` links `generated/parameters/ParameterIds.g.cs`, generated from the same `schema/parameters.json` that produces firmware IDs and the Markdown table. C# must not maintain a second parameter-ID enum by hand. Windows cloud build therefore also compiles the generated ABI definitions.

## Upgrade flow

`select package -> validate manifest/target/checksums -> request APP enter IAP -> reconnect Boot -> query info -> START/ERASE/WRITE with progress/retry -> VERIFY -> COMMIT -> reboot -> reconnect APP -> confirm version/health`

The PC validator improves usability but is not a trust boundary; Boot validates independently.

## Diagnostics

The tool should expose raw framed logs, sequence/CRC/timeout/retry counters, connection state, device identity, reset/boot reason, firmware/build/protocol versions and IAP result.

## Current status

Implemented scaffolding: shared protocol/client, Serial transport, Windows BLE transport, generated parameter IDs and cloud .NET Release/protocol smoke build. UI, complete service model, parameter editor, firmware package and upgrade workflow remain planned.
