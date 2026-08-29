# Event log

Event logging is a domain service for diagnostic and lifecycle evidence. It must not directly depend on UART, BLE, UI or a specific Flash peripheral.

## Implemented RAM core

`bms_event_log` is an allocation-free caller-owned fixed-capacity ring buffer. Append is O(1). When full, the oldest record is overwritten and logical reads remain oldest-to-newest across wrap-around.

Each runtime record contains:

- monotonic `sequence` assigned by the log;
- `timestamp_ms` supplied by the system time owner;
- stable/non-zero `event_id` supplied by the producer;
- severity: INFO/WARNING/PROTECTION/FAULT;
- source: BOOT/SYSTEM/PROTECTION/PARAMETER/NVM/COMM/AFE/IAP;
- two signed 32-bit event-specific data fields.

The core validates enum ranges and rejects event ID zero before modifying the ring. `clear` removes retained RAM records but deliberately does not reset `next_sequence`, so clearing history cannot make new records reuse the immediately preceding sequence space.

## Ownership boundaries

Protection, Boot/IAP, parameter commit, NVM, AFE and communication modules publish events; they do not own event storage policy. Protocol Log service reads the domain log through a future adapter and must not expose the in-memory C struct as wire ABI.

## Persistence roadmap

Persistent logging will use an explicit canonical encoded record format and a Flash backend separate from the RAM struct. It will reuse the same fail-safe principles already proven by the NVM transaction tests: CRC/versioned records, bounded writes, recovery after interrupted program/erase and wear-aware page rotation. Parameter/SOC atomic configuration records and append-oriented event history are different workloads and will not be forced into the same on-flash layout.

## Stable event identities

The current core accepts a non-zero `event_id`; the cross-language event identity catalog is still planned. Event IDs for boot/reset/watchdog, protection trip/release/latch, parameter commit, NVM error, IAP result, AFE fault and communication fault will be assigned from one schema before the Log payload ABI is frozen.

## Verification

Host tests cover empty read, sequence assignment, append, full-ring overwrite, oldest-to-newest reads across wrap-around, invalid event rejection without sequence advancement and clear-with-sequence-retention. Persistent power-loss/wear tests are required when the Flash backend is added.

Status: allocation-free RAM event ring **Implemented**; stable event-ID schema, persistent Flash journal, protocol Log handler and PC viewer **Planned**.
