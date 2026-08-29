# AI development workflow

AI-generated changes are untrusted contributions until repository gates and required hardware evidence pass.

## Before editing

Read `docs/README.md`, `ARCHITECTURE.md`, the owner document(s) for the touched subsystem, `CODING_STANDARD.md`, `TESTING.md` and `DOCUMENTATION_POLICY.md`. Inspect existing production tests and configuration/schema owners before inventing interfaces.

## Change sequence

1. State behavior/invariants and identify risk level.
2. Update owner documentation/schema first or in the same atomic change.
3. Implement the smallest architecture-consistent change.
4. Add boundary/failure tests using production source, not copied test implementations.
5. Run the relevant host/target/PC gates.
6. Report only checks actually executed; mark Keil/HIL/hardware gaps explicitly.

## Risk levels

High risk: Boot/IAP/Flash/vector/watchdog/ISR, protection/MOS, AFE hardware control/scaling, NVM/parameters, protocol parser/write commands, low-power/wake and release tooling. High-risk changes require explicit failure/boundary tests and HIL applicability assessment.

## Forbidden AI shortcuts

No test deletion/weakening, broad warning/static-analysis suppression, hardcoded duplicate protocol/parameter/layout definitions, fabricated hardware constants, compatibility hacks inside new domain core, or declaration of safety based only on a successful build.

## Definition of Done

Every AI completion reports: Changed, Docs/Schema Changed, Tests Added, Tests Run, Static Analysis, O0/O2/Differential result, Sanitizers, Firmware builds, PC build if affected, Flash/RAM impact, Keil status, HIL status, Known Risks and Not Tested.
