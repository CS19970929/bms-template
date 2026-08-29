# APP core architecture

APP uses cooperative non-blocking services rather than a monolithic main loop.

- `bms_scheduler`: deterministic periodic dispatch primitive; target timing source is platform-owned.
- `bms_protection`: detects one protection condition with threshold/hysteresis/delay; it never touches MOS/AFE.
- protection aggregation produces charge/discharge block masks.
- `bms_mos_policy`: arbitrates requested state, software block masks and AFE/hardware block mask; only the output/application adapter may call AFE FET operations.
- `bms_watchdog_supervisor`: a watchdog window can be fed only if every configured health bit reported progress during that window. Timer ISR must never unconditionally feed IWDG.
- `bms_afe_t`: AFE-neutral measurement/control interface; concrete AFE register access consumes `bms_afe_bus_t`.

The current hardware target APP is intentionally minimal until a board/AFE target is selected. Real target release must require AFE, protection, scheduler, communication and state-machine health bits in the supervisor before IWDG reload is allowed.
