# Cloud CI and self-hosted verification

GitHub Actions is the primary software admission layer.

## Hosted gates

`quality-gate`: documentation coupling; deterministic Command/Parameter/Protection/Event schema stale checks; target/layout generation; source policy; Host O0/O2 tests/equivalence; ASan/UBSan; Cppcheck. Dedicated Host tests compile generated C identity headers.

`firmware-build`: F030C8/F103C8 arm-none-eabi-gcc O2 APP+Boot, image budgets, artifacts and Keil-project generation. Actual STM32 NVM adapters and generated Command IDs are in the production source graph.

`pc-build`: Windows .NET 8 Release warnings-as-errors plus protocol smoke. `Bms.Protocol` compiles generated Command/Parameter/Protection/Event C# definitions.

Hosted green is software evidence only; hardware behavior and functional-safety claims require controlled Keil/HIL evidence.
