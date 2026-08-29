# Firmware Coding Standard

Baseline: C11 subset suitable for freestanding Cortex-M. MISRA concepts are used as engineering guidance; passing this repository does not claim certified MISRA compliance.

- use `<stdint.h>` fixed-width integers for protocol, storage, hardware and persisted values;
- encode physical units in names (`cell_mv`, `current_ma`, `temperature_decic`, `timeout_ms`);
- explicit endian helpers for serialized data; never cast byte buffers to structs;
- validate array indices, payload lengths, flash address + length and shift counts before use;
- avoid reliance on signed overflow or implementation-defined conversions;
- `volatile` is for observable access, not atomicity or synchronization;
- ISR/main shared state needs an explicit critical-section/event design;
- all hardware waits require a timeout and an error path;
- no heap allocation, recursion, VLAs or busy-loop delays in firmware;
- production optimization target is `-O2`; `-Ofast`/fast-math are forbidden;
- project-owned warnings are errors. Vendor code is isolated and does not weaken project warnings.
