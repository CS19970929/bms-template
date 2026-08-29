# System state machine

The system state machine owns high-level operating mode; protection modules own protection facts and MOS policy owns final power-path permission. State code does not duplicate protection thresholds.

## Implemented states

`INIT`, `NORMAL`, `CHARGING`, `DISCHARGING`, `IDLE`, `PROTECTED`, `SLEEP`, `DEEP_SLEEP`, `SHUTDOWN`, `UPGRADE_PENDING`, `FAULT`.

The portable `bms_state_machine` owns only deterministic transition state. Target entry/exit actions remain outside the pure core so they can be separately scheduled and hardware-tested.

## Implemented events

Initialization success/failure, normal/charge/discharge/idle activity, protection active/cleared, sleep/deep-sleep request, wake, shutdown request, upgrade request and fatal fault.

## Invariants

- Invalid transitions return an error and leave state unchanged.
- Entering `PROTECTED` from an operational state records the prior operational mode and protection-clear resumes that mode.
- Protection cannot be cleared by ordinary activity events.
- Sleep/deep-sleep entry is restricted to explicit idle/normal paths; wake returns to `IDLE`.
- `UPGRADE_PENDING` is a state request only; atomic Boot request persistence/reset is owned by the application/platform service.
- Fatal fault always drives `FAULT`.

## Verification

Host tests cover initialization, activity, protection/resume, rejected activity during protection and fatal-fault transition. Next expansion adds a complete transition-table test and product-specific state-policy adapter for charger/load detection, inactivity and low-power wake sources.

Status: core state engine **Implemented**; board/product transition policy and entry/exit hardware actions **Planned/Hardware-pending**.
