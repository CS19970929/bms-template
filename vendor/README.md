# Vendor dependencies

STM32 vendor code is isolated from BMS production-core code.

For the STM32F030 reference target:

```bash
python tools/bootstrap_vendor.py
```

The bootstrap downloads only whitelisted CMSIS device headers and STM32F0xx StdPeriph V1.5.0 driver files from a pinned mature repository and verifies Git blob hashes. It deliberately rejects/replaces project-modified infrastructure: the old repository's `system_stm32f0xx.c` is not imported because CI proved it depends on legacy `main.h`. This template owns a clean platform `SystemInit` instead.

No legacy BMS application, IAP, protocol or board source is imported.
