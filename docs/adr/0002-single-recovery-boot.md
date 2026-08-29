# ADR-0002: One protected Recovery Boot on 64 KiB targets

Status: Accepted.

Decision: F030C8/F103C8 64 KiB products use one minimal protected Recovery Boot plus a single APP slot and redundant metadata. Boot V1 is not remotely self-updated.

Rationale: the primary requirement is permanent APP recovery under software failures. A dual-IAP/self-update design consumes scarce Flash and adds a second failure/coordination state machine without improving the primary recovery guarantee enough to justify the complexity.

Consequence: single-slot products guarantee recovery reflashing, not rollback to the previous APP. Boot updates use SWD/factory/service until a separately justified mechanism is designed.
