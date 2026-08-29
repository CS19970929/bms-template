## Change summary

Describe behavior, not only files changed.

## Risk

- [ ] LOW — tooling/docs/non-runtime behavior
- [ ] MEDIUM — normal runtime behavior with bounded impact
- [ ] HIGH — Boot/IAP/Flash/vector/watchdog/ISR, protection/MOS, AFE, NVM/parameters, parser/write protocol, low-power/wake or release path

## Synchronized maintenance

- [ ] Owner documentation updated for every behaviorally changed subsystem
- [ ] Schema/config/generated-source owner updated where applicable
- [ ] Tests added/updated for the changed behavior
- [ ] `tools/check_docs.py` mapping remains correct
- [ ] No duplicate protocol/parameter/layout definition introduced

## Verification actually executed

- [ ] Host O0
- [ ] Host O2
- [ ] O0/O2 equivalence/differential
- [ ] ASan/UBSan
- [ ] Cppcheck/policy/docs gate
- [ ] F030 GCC O2 if affected
- [ ] F103 GCC O2 if affected
- [ ] PC Release/protocol smoke if affected
- [ ] Keil/ARMCC if required
- [ ] HIL if required

## Evidence / limitations

List Flash/RAM impact, CI/HIL links, known risks and anything Not Tested. Do not describe hosted CI as hardware validation.
