# ADR-0004: Domain code does not directly control hardware

Status: Accepted.

Decision: protection detection publishes facts/block reasons; MOS policy arbitrates; hardware adapters apply outputs. Domain services depend on stable interfaces rather than STM32/AFE/transport implementations.

Rationale: required to test behavior on Host, support multiple AFEs/MCUs, avoid same-port/split-port condition scattering and prevent conflicting software/hardware protection actions.

Consequence: direct AFE FET calls in protection/business/protocol code are architecture violations and must be rejected in review/quality checks.
