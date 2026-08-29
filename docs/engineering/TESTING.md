# Testing Strategy

Pure production code is built on the host and on MCU targets from the same source files.

Required host gates:

- Debug/O0 build and tests;
- Release/O2 build and tests;
- ASan + UBSan build and tests;
- Cppcheck on project-owned firmware code;
- deterministic O0/O2 behavior for the same test vectors.

High-risk modules require boundary and fault tests. Examples: protection threshold/delay ±1, malformed protocol lengths/CRC, image target/size/CRC/vector validation, redundant metadata corruption, simulated power loss between erase/write/metadata steps.

Cloud CI cannot validate interrupt timing, flash electrical behavior, watchdog recovery, AFE/MOS wiring or brownout. Those remain HIL release gates.
