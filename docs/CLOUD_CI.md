# Cloud CI and self-hosted verification

GitHub Actions is the primary software admission layer.

## Hosted gates

`quality-gate`: documentation structure/change coupling, deterministic parameter/protection schema-generated-output checks, target configuration generation, source/build policy scan, Host O0 and O2 builds/tests, O0/O2 output equivalence, ASan/UBSan and Cppcheck. Generated C schema headers are also compiled by dedicated Host tests, not merely compared as text. The documentation structure gate now includes the Event Log owner document and couples event-log source changes to it.

`firmware-build`: matrix builds F030C8/F103C8 APP+Boot with arm-none-eabi-gcc O2, validates generated image budgets, emits ELF/MAP/HEX/BIN/target summary and generates/parses Keil Boot/APP project XML from the same source graph.

`pc-build`: Windows .NET 8 restore/Release build with warnings as errors plus protocol smoke. Generated C# parameter and protection ABI definitions are compiled as part of `Bms.Protocol`.

`cloud-verify`: manually aggregates hosted software gates.

## Self-hosted gates

`keil-compat`: manual `[self-hosted, Windows, Keil]` runner with licensed/local UV4/ARMCC. It is deliberately not an automatic fork-PR executor.

HIL: future manual/release-gated runner attached to real BMS hardware/fixture. See `HIL.md`.

## Security boundary

Self-hosted runners connected to licensed tools or hardware must not automatically execute untrusted external PR code. Use controlled branches/manual dispatch and minimum repository permissions.

## Interpretation

Hosted green proves the checked source/configuration compiled and passed defined software tests on that commit. It does not prove real analog scaling, AFE/MOS behavior, EMI/power integrity, brownout interruption, wake behavior or safety certification.
