# AI Development Contract

AI-generated code is an untrusted contribution until repository quality gates and applicable hardware evidence pass.

## Mandatory reading before production changes

1. `docs/README.md` and `docs/ARCHITECTURE.md`.
2. The owner document(s) for every touched subsystem.
3. `docs/engineering/CODING_STANDARD.md`.
4. `docs/engineering/TESTING.md`.
5. `docs/engineering/AI_DEVELOPMENT.md` and `docs/engineering/DOCUMENTATION_POLICY.md`.
6. Relevant ADRs under `docs/adr/`.

## Mandatory workflow

1. Identify behavior/invariants and risk level before editing.
2. Update the owner document/schema first or in the same atomic change.
3. Keep domain/core code independent of MCU, AFE, transport, UI and IDE.
4. Add/update production-source tests for every behavior change. High-risk changes require boundary/failure tests.
5. Run `python tools/check.py` before declaring software work complete; run target/PC gates when affected.
6. Report checks actually executed and explicitly list Keil/HIL/hardware checks not executed.

`tools/check_docs.py` is a mandatory maintenance fence. Do not bypass or weaken its path-to-document coupling rules merely to make CI pass. If ownership changes, update the documentation policy and gate together with rationale.

## Forbidden shortcuts

- no `malloc/calloc/realloc/free` in firmware;
- no recursion or VLA in firmware;
- no unchecked buffer-to-struct casts;
- no unbounded hardware polling or software delay loops;
- no `-Ofast` or `-ffast-math`;
- no global warning/static-analysis suppression to make CI green;
- no deleting/weakening tests to accept a change;
- no direct writes to Boot Flash from APP APIs;
- no direct AFE/MOS calls from protection/business/protocol logic;
- no protocol/parameter/layout IDs or addresses duplicated across firmware/PC/build layers;
- no fabricated GPIO polarity, AFE addresses/scaling or hardware thresholds when board facts are unknown;
- no safety/release claim based only on a successful build or hosted CI.

Keil is a generated debug/build view, not the source of truth for source lists, memory layout or release compiler settings. CMake/configuration files are authoritative.

## Definition of Done

Report: Changed; Docs/Schema Changed; Tests Added; Tests Run; Static Analysis; O0/O2/Differential; Sanitizers; Firmware Builds; PC Build; Flash/RAM Impact; Keil Status; HIL Status; Known Risks; Not Tested.
