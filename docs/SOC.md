# SOC service

SOC separates physical estimate from user-visible display behavior.

## Implemented portable core

`bms_soc` stores coulomb state in integer milliamp-seconds with a milliamp-millisecond remainder, avoiding floating point and preserving sub-second integration. Current sign convention in the core is positive=charging, negative=discharging.

Configuration includes nominal capacity, fixed integration period, current measurement floor, rest-current threshold/rest duration, display slew step, OCV correction step and the reference rule that forbids upward rest correction.

Reference product configuration remains: current sampling about 250 ms, SOC integration fixed at 200 ms, measurement floor about 200 mA, qualified rest at least 10 minutes. Those product values are not hardcoded into the generic core.

## Behavior

- `soc_est_permille` is derived from bounded coulomb state.
- `soc_display_permille` approaches estimate by a configurable maximum step.
- Current below the configured integration floor is not coulomb-integrated.
- Rest qualification accumulates only while current magnitude is within the rest threshold.
- OCV target is rejected before qualified rest; when upward rest correction is forbidden, an OCV target above estimate produces no increase.
- Accepted OCV correction is rate-limited.
- Explicit full/empty anchors set estimate and display to 1000/0 permille.

## Still outside this core

OCV table lookup/error band, temperature/chemistry selection, endpoint qualification from protection/state facts, persistent checkpoint validity, SOH/capacity learning and user-facing percent conversion remain separate services/policies.

Capacity learning remains disabled in the initial platform.

Status: integer integration/rest/downward OCV correction/display smoothing/endpoint anchors with Host tests **Implemented**; OCV table, persistence and product endpoint integration **Planned**.
