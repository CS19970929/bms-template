# SOC service

SOC separates physical estimate from user-visible display behavior.

## State

- `soc_est`: estimator state driven primarily by coulomb integration and bounded calibration.
- `soc_display`: user-visible state that converges smoothly toward the estimate and endpoint anchors.

## Reference timing/policy

- Current sampling reference cadence: 250 ms.
- Coulomb integration cadence: fixed 200 ms.
- Currents below the configured measurement floor (reference 200 mA) may be excluded from integration and corrected over long rest periods.
- Rest/OCV calibration becomes eligible after at least 10 minutes of qualified rest.
- During rest calibration, SOC must not jump upward under the reference policy; long-term deviation converges slowly to the accepted OCV error-band boundary.

## Endpoint behavior

Confirmed full-charge protection/condition may anchor 100%. Discharge endpoint policy must approach/anchor 0% without a large display jump. Display rate limiting/smoothing is independent from estimator correction.

## Capacity/SOH

Capacity learning is a separate state machine and is currently disabled in the initial platform. No unvalidated learned capacity may silently replace configured nominal capacity. SOH must have its own validity indication.

## Persistence

SOC/runtime checkpoints use the NVM service with bounded commit rate. A reboot must distinguish valid recent checkpoint from corrupt/stale data.

## Status

Planned in this repository. Implementation must be host-testable with synthetic current/voltage/time traces before AFE coupling.
