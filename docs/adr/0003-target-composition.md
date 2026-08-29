# ADR-0003: Target is MCU + AFE + Board + Product

Status: Accepted.

Decision: product variants are composed from independent MCU, AFE, board and product profiles. CMake must branch on supported platform capability/family, not product target names.

Rationale: avoids copied BMS projects and lets new boards/AFEs reuse Boot, protection, SOC, protocol, NVM and PC code.

Consequence: target generation validates cross-profile constraints and emits the only build/layout constants consumed by firmware/CI/Keil generation.
