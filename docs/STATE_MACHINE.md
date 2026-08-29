# System state machine

The system state machine owns high-level operating mode; protection modules own protection facts, and MOS policy owns final power-path permission. State code must not duplicate protection thresholds.

## Target states

`INIT`, `NORMAL`, `CHARGING`, `DISCHARGING`, `IDLE`, `PROTECTED`, `SLEEP`, `DEEP_SLEEP`, `SHUTDOWN`, `UPGRADE_PENDING`, `FAULT`.

Not every product must expose every state externally, but internal transitions and entry/exit actions must remain explicit and testable.

## Event model

Transitions consume events/facts such as initialization complete/failed, measured current direction, protection block changes, charger/load detection, inactivity timeout, wake source, low-SOC/deep-discharge policy, upgrade request and fatal service failure.

## Rules

- State transitions are deterministic for the same current state + event/facts.
- Entry/exit actions are short and non-blocking.
- Protection cannot be cleared merely because the system changes state.
- Communication commands request transitions; they do not directly manipulate hardware.
- Upgrade request reaches `UPGRADE_PENDING`, stores the required boot request atomically, then resets through the platform.
- Sleep/deep-sleep policy must declare wake sources and any measurement/communication loss explicitly.

## Status

Planned. Before implementation, the transition table in this document is authoritative; implementation must add table-driven host tests covering every allowed and rejected transition.
