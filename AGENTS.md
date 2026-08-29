# AI Development Contract

AI-generated code is an untrusted contribution until the repository quality gates pass.

## Mandatory workflow

1. Read `docs/ARCHITECTURE.md`, `docs/engineering/CODING_STANDARD.md`, and `docs/engineering/TESTING.md` before modifying production code.
2. Keep domain/core code independent of MCU, AFE, transport, UI, and IDE.
3. Add or update tests for every behavior change. High-risk changes (boot/IAP, flash, watchdog, protection, MOS, parser, NVM) require boundary/failure tests.
4. Run `python tools/check.py` before declaring work complete.
5. Report checks actually executed and any target/HIL checks not executed.

## Forbidden shortcuts

- no `malloc/calloc/realloc/free` in firmware;
- no recursion or VLA in firmware;
- no unchecked buffer-to-struct casts;
- no unbounded hardware polling;
- no software delay loops;
- no `-Ofast` or `-ffast-math`;
- no global warning suppression to make CI green;
- no deleting/weakening tests to accept a change;
- no direct writes to bootloader flash from APP APIs;
- no direct AFE/MOS calls from protection detection logic;
- no protocol IDs or parameter IDs duplicated across firmware and PC layers.

Keil is a supported debug/build view, not the source of truth for source lists, memory layout, or release compiler settings. CMake/configuration files are authoritative.
