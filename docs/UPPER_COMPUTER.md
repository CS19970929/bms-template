# PC upper computer

The desktop tool is a .NET 8 Windows architecture with one domain client and pluggable transports.

## Layers

- `Bms.Protocol`: frame codec, generated stable IDs and wire payload models.
- `Bms.Transport`: Serial/Windows BLE `ITransport`, later CAN/TCP.
- `Bms.Client`: typed Device/Protection/Parameter/Log APIs.
- future `Bms.Upgrade`: package validation and IAP orchestration.
- UI: dashboard, telemetry, protection/alarm, parameters, logs, upgrade, diagnostics and communication console.

Transport classes do not contain BMS business rules. UI does not parse frames directly.

## Generated ABI metadata

`Bms.Protocol` links generated Command, Parameter, Protection and Event IDs. Each originates from the same schema that produces firmware C IDs and generated Markdown. C# must not maintain a separate command enum or magic-number table.

Product thresholds/actions are descriptor data, not identity constants. Event UI strings may be localized but stable IDs and data semantics come from the Event schema.

## Upgrade flow

`select package -> validate -> request APP enter IAP -> reconnect Boot -> query -> transfer -> verify -> commit -> reboot -> reconnect APP -> confirm health/version`

PC validation is not a trust boundary; Boot validates independently.

## Current status

Implemented: shared frame/client scaffolding, Serial transport, Windows BLE scaffold, generated Command/Parameter/Protection/Event IDs and Windows Release/protocol smoke. Concrete payload codecs/service APIs, UI and end-to-end upgrade orchestration remain planned.
