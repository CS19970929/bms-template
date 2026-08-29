# Vendor dependencies

STM32 vendor code is deliberately isolated from BMS production-core code.

For the STM32F030 reference target, run:

```bash
python tools/bootstrap_vendor.py
```

The script downloads only a whitelist of ST CMSIS / STM32F0xx StdPeriph V1.5.0 files from a pinned commit of an existing mature repository and verifies each Git blob hash before accepting it. No BMS application source is imported.

The downloaded directory is ignored by Git so the authoritative dependency recipe is reviewable and deterministic. For an offline manufacturing environment, mirror the exact verified vendor directory internally.
