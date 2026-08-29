# AFE adapters

`bms_afe_t` is the only AFE surface visible to application/domain services. A concrete AFE driver owns its vendor register protocol and consumes an injected `bms_afe_bus_t` instead of calling STM32 I2C/SPI APIs directly.

Target configuration chooses an AFE adapter independently from the MCU family. Expected adapters include `sh367309`, `sh367601`, `bq76940`, `bq76942`, and `bq76952`.

This bootstrap intentionally implements only `mock`: no AFE register map, address, current scale, FET polarity, hardware-protection threshold, or alert behavior is guessed without the exact product hardware/configuration. Adding a real adapter must not change protection/SOC/protocol core APIs.
