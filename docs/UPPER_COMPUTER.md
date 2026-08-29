# PC Tool

The desktop tool is one product, not separate serial and BLE applications.

- `Bms.Protocol`: exact application frame codec shared conceptually with firmware.
- `Bms.Transport`: `IBmsTransport`, serial implementation and Windows BLE GATT implementation.
- `Bms.Client`: stream reassembly + common command client.
- `Bms.Tool`: .NET 8 WPF UI shell.

Serial and BLE therefore feed the same `BmsClient`. Future CAN/TCP transports must implement `IBmsTransport` rather than fork command/UI logic.

BLE UUIDs are intentionally runtime/config values. A later `legacy_v1` adapter will map existing BLE application frames to the new service API without contaminating domain internals.
