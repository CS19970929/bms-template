# APP architecture

APP is a cooperative, non-blocking set of services around a hardware-independent domain core.

## Runtime flow

`measurement -> derived values -> protection detection -> protection aggregation -> MOS arbitration -> hardware output`

Side services consume validated state: parameter service, NVM/event log, SOC, state machine, protocol/diagnostics and low-power manager.

## Implemented portable primitives

- `bms_scheduler`: deterministic periodic dispatch primitive; platform owns time source.
- `bms_protection`: threshold/hysteresis/delay primitive; no MOS/AFE access.
- `bms_mos_policy`: arbitrates user/system request, software block masks and hardware/AFE block mask.
- `bms_watchdog_supervisor`: reload is allowed only when all required health bits made progress.
- `bms_state_machine`: deterministic high-level state transition core with protection-resume semantics.
- `bms_parameter`: typed access/range validation and staged transaction/commit core.
- `bms_nvm_record`: versioned CRC-protected redundant-record validation/selection core.
- `bms_soc`: fixed-period integer coulomb integration, rest tracking, bounded OCV correction and display smoothing.
- `bms_afe_t` + `bms_afe_bus_t`: concrete AFE implementations remain adapters.

## Planned composition services

`MeasurementService`, multi-item `ProtectionManager`, parameter-schema binding/persistence service, physical `NvmService`, event log, complete protocol services, SOC OCV/product policy and low-power policy.

## Scheduling rules

No task blocks on peripheral completion without a bounded timeout. Long operations are explicit state machines. ISR code records minimum data/events and exits; protocol parsing, Flash commit and protection policy execute outside ISR context.

## Watchdog health contract

A release APP requires main-loop/scheduler plus critical measurement/AFE, protection/state and communication progress before IWDG reload. New critical services add a health bit and document its deadline; timer ISR never unconditionally reloads IWDG.

## Hardware release boundary

Current target APP remains a platform reference. Hardware release additionally requires board polarity, AFE scaling/configuration, MOS topology, hardware protection thresholds, wake sources and HIL evidence.
