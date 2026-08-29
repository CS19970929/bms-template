# Event log

Event logging is a domain service for diagnostic/lifecycle evidence and is independent of UART, BLE, UI or a specific Flash peripheral.

## Runtime core

`bms_event_log` is an allocation-free caller-owned ring. Append is O(1); on full capacity the oldest record is overwritten. Logical read order remains oldest-to-newest across wrap-around.

Each record contains sequence, timestamp_ms, event_id, severity, source and signed `data0/data1`. Invalid event ID zero or invalid enum inputs are rejected before mutation. Clear removes retained RAM records but does not reset the next sequence.

## Stable Event ABI schema

`schema/events.json` now owns event ID plus default source/severity and the semantic names of `data0/data1`. It deliberately does not encode UI strings or transport layout. `tools/generate_events.py` emits firmware C IDs, .NET C# IDs and `docs/generated/EVENT_TABLE.md`; stale output is a quality-gate failure. Host compiles the generated C header and Windows `Bms.Protocol` compiles the generated C# constants.

The initial catalog covers Boot start, state change, watchdog-health failure, protection trip/release/latch clear, parameter commit/restore failure, NVM error, IAP result, AFE fault and communication error. IDs are release ABI: renumbering requires explicit compatibility review rather than editing an enum in one component.

The schema's `data0/data1` names define interpretation, not numeric sub-enums. When a field such as `boot_reason`, `operation_code` or `channel_id` receives a stable numeric catalog, that catalog must also gain a single source of truth before the payload is released.

## Persistence boundary

Persistent event history will use an explicit encoded journal, not the in-memory struct. Parameter/SOC atomic records and append-oriented event history are different write workloads and do not share the current two-slot format.

## Protocol/PC roadmap

The Log service will encode records field-by-field in little-endian order and .NET will decode the same golden vectors. The PC viewer will resolve generated Event IDs plus generated Protection/Parameter IDs rather than hard-coded magic numbers.

## Verification

Host tests cover ring behavior and generated Event IDs. Generator stale checks, O0/O2 equivalence, sanitizer and Windows C# compilation are admission gates. Persistent power-loss/wear and Log service golden vectors remain future evidence.

Status: allocation-free RAM event ring + stable cross-language Event ID schema **Implemented**; persistent journal, concrete Log protocol payload and PC viewer **Planned**.
