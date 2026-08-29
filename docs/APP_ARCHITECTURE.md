# APP architecture

APP is a cooperative, non-blocking set of services around a hardware-independent domain core.

## Runtime flow

`measurement -> derived values -> protection detection -> protection aggregation -> MOS arbitration -> hardware output`

Side services consume the same validated state: parameter service, NVM/event log, SOC, protocol/diagnostics and low-power manager.

## Implemented primitives

- `bms_scheduler`: deterministic periodic dispatch primitive; platform owns the time source.
- `bms_protection`: threshold/hysteresis/delay primitive; no MOS/AFE access.
- `bms_mos_policy`: arbitrates requested state, software block masks and hardware/AFE block mask.
- `bms_watchdog_supervisor`: reload is allowed only when all required health bits made progress in the supervision window.
- `bms_afe_t` + `bms_afe_bus_t`: concrete AFE implementations are adapters behind stable interfaces.

## Planned service boundaries

- `StateService`: owns system state and event-driven transitions.
- `ParameterService`: validates typed parameters, permissions and atomic commit requests.
- `NvmService`: owns redundant/versioned persistent records and migration.
- `SocService`: owns estimated/display SOC; never writes raw measurement ownership.
- `EventLog`: structured boot/reset/protection/IAP/NVM/parameter/communication events.
- `ProtocolService`: maps stable service/command IDs to domain APIs.

## Scheduling rules

No task may block on peripheral completion without a bounded timeout. Long operations must be explicit state machines. ISR code records minimum data/events and exits; protocol parsing, Flash commit and protection policy execute outside ISR context.

## Watchdog health contract

A release APP must require at least scheduler/main-loop progress plus critical measurement/AFE, protection and communication/state progress before IWDG reload. A timer ISR must never unconditionally reload the watchdog. New critical services must add a health bit and document its deadline.

## Hardware release boundary

The current target APP is a platform reference. A product is not hardware-releasable until board polarity, AFE scaling/configuration, MOS topology, hardware protection thresholds, wake sources and HIL evidence are complete.
