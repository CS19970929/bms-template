# Protection and MOS policy

Protection is split into detection, aggregation/arbitration and hardware actuation. This separation is mandatory for same-port/split-port products and for coexistence with AFE hardware protection.

## Pipeline

`Measurement -> Protection Detection -> Protection Manager -> MOS Policy -> AFE/Board Output`

Detection and manager code never write FETs. Business/communication code never writes FETs directly.

## Stable protection identity schema

`schema/protections.json` is the platform owner for protection identity and measurement semantics. It defines only stable ID, symbolic name, category, detector kind and measurement semantic. It deliberately does **not** define product thresholds, delays, enable defaults, latch policy, severity level or MOS action; those belong to a Product protection profile because they depend on chemistry, AFE capability and board topology.

`tools/generate_protections.py` deterministically generates firmware C IDs, PC C# IDs and `docs/generated/PROTECTION_TABLE.md`. `tools/check.py` rejects stale generated outputs, Host compiles the generated C header, and Windows `Bms.Protocol` compiles the generated C# IDs. IDs are therefore one ABI across firmware, PC and documentation.

The initial catalog reserves semantic IDs for CELL_OV/CELL_UV, charge/discharge OC, short circuit, charge/discharge OTP/UTP, MOS OTP, pack OV/UV, SOC low and AFE fault. Adding or renumbering an identity is an ABI change and requires synchronized schema, generated outputs, owner documentation and compatibility review.

## Detector primitive

`bms_protection` implements HIGH/LOW threshold detection with independent trip/release thresholds, trip delay, release delay and enable state. Its states are `NORMAL`, `PENDING`, `ACTIVE`, `RECOVERING`. Only ACTIVE/RECOVERING are blocking states; PENDING is filtering time, not a power-path trip.

## Implemented Protection Manager

`bms_protection_manager` composes caller-owned arrays of rules/runtime state without dynamic allocation. Each rule has a stable protection ID, detector configuration, charge block mask, discharge block mask and optional latch behavior.

Manager output is directly compatible with the software block-mask inputs of MOS arbitration: aggregate charge mask, aggregate discharge mask, active count and first active ID. Multiple rules may block either direction or both. Block-mask bits are domain reasons; hardware/AFE faults remain a separate higher-priority `hardware_block_mask` in MOS policy.

Rule validation rejects invalid detector enum/enable/latch values, inverted hysteresis (`HIGH: release > trip`, `LOW: release < trip`) and duplicate protection IDs. Every `step` validates all per-rule detector configurations before advancing any runtime state, so an invalid configuration fails atomically instead of partially progressing earlier protection timers.

## Latching

A latch-enabled rule latches when its detector reaches `ACTIVE`. The block remains even after the measured condition has fully returned to `NORMAL`. Explicit clear is rejected while the detector is still non-NORMAL. Disabling a rule clears its detector and latch; permission to disable safety protections belongs to the parameter/product policy, not this generic manager.

## Product protection profile roadmap

The stable identity catalog is implemented. The next layer is a Product profile keyed by protection ID and Parameter ID that supplies enable/default/min/max, trip/release thresholds, trip/release delay, alarm level, latch/recovery policy, direction-specific block reasons and feature applicability. Product profiles must be validated as a whole before runtime activation, and no profile may invent AFE-supported ranges without authoritative hardware data.

## Hardware/software arbitration

Software protection publishes charge/discharge block reasons. Hardware/AFE faults publish separate higher-priority block reasons. MOS policy combines user/system request and all block masks; lower-priority software requests cannot override a hardware fault.

## Same-port / split-port

Topology differences belong in `MosTopology/PowerPathPolicy`, not scattered preprocessor branches. Domain protection meaning remains identical; only actuation/recovery feasibility changes by topology.

## Reverse-current recovery

The desired policy after charge over-current with confirmed discharge current, or discharge over-current with confirmed charge current, remains a separate parameterized arbitration/recovery policy with current threshold, filtering, delay and retry limits. It must not repeatedly fight an AFE hardware latch or chatter FETs.

## Verification

Host tests cover manager trip-delay aggregation, direction-specific masks, release delay, latch retention, rejected clear while condition is active, explicit clear, duplicate IDs, invalid hysteresis and atomic rejection of an invalid rule set without partial detector-state advancement. Schema tests compile selected stable IDs, the generator stale-check compares all generated files, and Windows CI compiles the C# protection IDs. Each product protection item still requires threshold±1, delay±1 tick, enable/disable, simultaneous protections and charge/discharge arbitration tests. HIL adds real AFE thresholds, MOS polarity/topology and current reversal behavior.

Status: detector + multi-rule manager + MOS block arbitration + stable cross-language protection identity schema **Implemented**; product threshold/action profiles, alarm mapping, reverse-current recovery and real AFE/HIL integration **Planned/Hardware-pending**.
