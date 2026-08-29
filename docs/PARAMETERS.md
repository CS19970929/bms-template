# Parameter system

Parameters are product configuration data with stable identity, validation and persistence semantics. They are not arbitrary exposed C globals.

## Parameter descriptor

Each parameter has: stable ID, symbolic name, type, unit, default, minimum, maximum, access permission, persistence class, optional product/feature applicability and documentation text.

The long-term source of truth is a machine-readable parameter schema that generates firmware identifiers/descriptors, PC metadata and human-readable protocol tables. IDs must never be independently duplicated in C and C#.

## Update flow

`protocol/UI request -> decode -> permission check -> type/range validation -> cross-field validation -> RAM shadow -> apply/commit policy -> atomic NVM commit -> result/event`

Invalid input never partially updates active configuration. Multi-field changes that have cross-field constraints use a transaction/shadow object.

## Permissions

At minimum distinguish read-only telemetry/identity, normal user configuration, service/calibration and factory-only values. Permission policy belongs to the service layer; raw NVM access is not a protocol feature.

## Safety rules

Protection threshold updates must validate relationships such as release vs trip threshold, level ordering, delay bounds and hardware capability. A value accepted by syntax/type alone is not necessarily safe to apply.

## Status

Planned. The first implementation will introduce the descriptor/schema core, validation APIs, transaction semantics and host tests before protocol write commands are enabled.
