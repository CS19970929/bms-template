# Non-volatile storage

NVM provides atomic, versioned persistence for parameters, SOC/runtime checkpoints, event logs and Boot requests without exposing raw Flash semantics to domain code.

## Record format

A persistent record contains at minimum magic, record/schema version, sequence number, payload length, payload CRC and record/header integrity fields. Records are selected only after structural/range/CRC validation.

## Atomic commit

Use copy-on-write/redundant slots or a wear-leveled append strategy. Never erase the only known-good committed value before a replacement is fully written and verified. Power loss at every erase/program/commit boundary must leave a deterministic result: previous valid record, new valid record, or explicit default/recovery state.

## Versioning and migration

Schema version is independent of firmware version. A migration is explicit, one-way, host-tested and never guesses unknown future layouts. Unsupported/corrupt records fall back according to the owning domain policy and emit diagnostics/event evidence.

## Wear policy

High-rate runtime values must not be committed on every sampling/integration tick. Each record class declares minimum commit interval/change threshold and expected lifetime. Event log uses bounded/wear-aware retention.

## Interfaces

Domain services request typed record load/store; platform owns physical Flash page erase/program/read. Domain modules must not include STM32 Flash headers.

## Status

Planned. Host Flash simulation with injected interruption is mandatory before target Flash integration.
