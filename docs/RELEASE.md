# Release process

A release is a reproducible set of binaries, configuration identity and verification evidence, not only a Git tag.

## Required software gates

Host O0/O2 tests and equivalence, ASan/UBSan, Cppcheck/policy/documentation gates, target GCC O2 builds, generated Flash/RAM budget checks, PC Release build/protocol smoke and generated-file/schema consistency.

Keil/ARMCC compatibility is an additional self-hosted gate for products that still require Keil delivery/debug. Product release additionally requires the applicable HIL set.

## Artifacts

- Boot: ELF/MAP/HEX/BIN.
- APP: ELF/MAP/HEX/BIN.
- target/config summary and hashes.
- firmware package (`.bmsfw`) once package tooling is implemented.
- PC application/package.
- release manifest.
- static-analysis, unit/integration/fuzz and HIL reports/evidence.

## Release manifest

Include release version, Git SHA, dirty-state prohibition, toolchain versions, MCU/board/AFE/product target, Boot/APP/protocol/parameter schema versions, image sizes/checksums, generated layout, CI run IDs/results, HIL evidence ID and known limitations.

## Version policy

Protocol/schema compatibility versions are independent of marketing/firmware version. Build metadata contains Git commit/build identity. A release must be reproducible from the tagged commit and pinned/recorded toolchain inputs.

## Acceptance boundary

No release claim may state hardware validation or functional-safety compliance solely from hosted CI. Missing Keil/HIL/hardware evidence is explicitly marked Not Tested/Hardware-pending.
