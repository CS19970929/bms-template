# ADR-0001: CMake/GCC configuration is the build source of truth

Status: Accepted.

Decision: VSCode + CMake + Ninja + arm-none-eabi-gcc is the normal reproducible firmware path. CMake/configuration owns source lists and generated layout. Keil projects are generated debug/compatibility views and may not become an independent source graph.

Rationale: one graph prevents AI/manual drift between GCC, CI and Keil projects while retaining Keil debugging/ARMCC compatibility where needed.

Consequence: Keil-specific changes must flow through generator/profile inputs; a hand-edited `.uvprojx` is disposable.
