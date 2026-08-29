# PC upper computer

The desktop tool is a .NET 8 Windows architecture with one domain client and pluggable transports.

## Layers

- `Bms.Protocol`: frame codec, generated stable IDs, errors and protocol-version/capability model.
- `Bms.Transport`: `ITransport`; Serial and Windows BLE implementations, later CAN/TCP.
- `Bms.Client`: typed device/protection/parameter/log APIs.
- future `Bms.Upgrade`: package validation and Boot/IAP orchestration.
- UI: dashboard, cells/temps/current/SOC, MOS/state/protection/alarm, parameters/calibration, logs, upgrade, diagnostics and communication console.

Transport classes do not contain BMS business rules. UI does not parse frames directly.

## Generated ABI metadata

`Bms.Protocol` links generated Parameter IDs, Protection IDs and Event IDs. All originate from the same platform schemas that produce firmware C IDs and Markdown tables. C# must not maintain independent magic-number enums.

Product thresholds/delays/actions are not embedded in identity classes; the PC reads device/product descriptors through protocol services. Event UI text may be localized in the PC, but the stable event ID and `data0/data1` semantics come from the Event schema.

## Upgrade flow

`select package -> validate manifest/target/checksums -> request APP enter IAP -> reconnect Boot -> query info -> START/ERASE/WRITE -> VERIFY -> COMMIT -> reboot -> reconnect APP -> confirm version/health`

PC validation improves usability but Boot validates independently.

## Current status

Implemented: shared frame protocol/client scaffolding, Serial transport, Windows BLE transport scaffold, generated Parameter/Protection/Event IDs and Windows Release/protocol-smoke build. Concrete service payload models, UI, firmware package and end-to-end upgrade orchestration remain planned.
