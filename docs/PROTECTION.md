# Protection and MOS policy

Protection is split into detection, aggregation/arbitration and hardware actuation. This separation is mandatory for same-port/split-port products and for coexistence with AFE hardware protection.

## Pipeline

`Measurement -> Protection Detection -> Protection State -> Protection Aggregation -> MOS Policy -> AFE/Board Output`

Detection code never writes FETs. Business/communication code never writes FETs directly.

## Protection item model

Each protection item is designed to support: enable, trip threshold, release threshold/hysteresis, trip delay, release delay, severity/level, latch mode, recovery policy, charge/discharge action and alarm identity.

Target set includes OV, UV, charge/discharge over-current, short circuit, charge/discharge over-temperature/under-temperature, MOS temperature, pack-voltage/SOC policy and AFE/hardware faults.

## Hardware/software arbitration

Software protection publishes charge/discharge block reasons. Hardware/AFE faults publish separate higher-priority block reasons. MOS policy combines user/system request and all block masks; a lower-priority software request cannot override a hardware fault.

## Same-port / split-port

Topology differences belong in `MosTopology/PowerPathPolicy`, not scattered preprocessor branches. Domain protection meaning remains identical; only actuation/recovery feasibility changes by topology.

## Reverse-current recovery

The desired policy after charge over-current with confirmed discharge current, or discharge over-current with confirmed charge current, is a parameterized arbitration rule with current threshold, filtering, delay and retry limits. It must not repeatedly fight an AFE hardware latch or chatter FETs.

## Implemented / planned

Implemented: generic threshold/hysteresis/delay primitive and MOS block arbitration.

Planned: multi-item Protection Manager, typed configuration/schema, latching/recovery policies, hardware-fault integration, topology policy and structured alarm/event output.

## Verification

Every item requires boundary tests at threshold±1 unit, delay±1 tick, hysteresis/release, enable/disable, latch/recovery, simultaneous protections and both charge/discharge arbitration. HIL adds real AFE thresholds, MOS polarity/topology and current reversal behavior.
