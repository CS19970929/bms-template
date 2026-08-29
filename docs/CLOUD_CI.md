# Cloud CI and self-hosted verification

GitHub Actions is the primary software admission layer.

## Hosted gates

`quality-gate`: documentation structure/change coupling; deterministic Parameter/Protection/Event schema generated-output checks; target generation/layout validation; source/build policy; Host O0/O2 tests and equivalence; ASan/UBSan; Cppcheck. Dedicated Host tests compile all generated C identity headers.

`firmware-build`: F030C8/F103C8 arm-none-eabi-gcc O2 APP+Boot matrix, generated image budgets, ELF/MAP/HEX/BIN/summary artifacts and generated Keil project XML. Actual family NVM Flash adapters are part of this source graph.

`pc-build`: Windows .NET 8 restore/Release warnings-as-errors plus protocol smoke. `Bms.Protocol` compiles generated Parameter, Protection and Event IDs on every PR.

`cloud-verify`: manually aggregates hosted software gates.

## Self-hosted gates

`keil-compat`: manual `[self-hosted, Windows, Keil]` runner with licensed/local UV4/ARMCC. HIL is a future controlled runner attached to real BMS hardware/fixture.

## Interpretation

Hosted green proves only the defined software evidence for that commit. It does not prove real analog scaling, AFE/MOS behavior, Flash brownout guarantees, EMI/power integrity, wake behavior or functional-safety certification.
