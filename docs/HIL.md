# Hardware-in-loop verification

HIL is the evidence layer between software CI and product release. Hosted CI cannot validate analog/AFe/MOS/power behavior.

## Runner topology

A Windows/Linux self-hosted GitHub runner may control: debugger/programmer, USB serial/RS485/CAN adapters, programmable supply/charger, electronic load, relays/fault-injection fixture and optional measurement equipment. HIL workflows are never run on untrusted fork PR code against lab hardware.

## IAP mandatory cases

Normal/repeated update, wrong MCU/product/version policy, malformed header, bad CRC, truncated/oversize image, packet loss/repeat/reorder, disconnect/reconnect and power interruption during erase/write/verify/metadata commit. Invalid APP, HardFault/watchdog/repeated boot failures must end in a reflashing-capable Recovery Boot.

## APP mandatory cases

OV/UV, charge/discharge OC, short circuit, temperature protections, threshold/delay boundaries, release/latch behavior, same-port/split-port policy, reverse-current recovery, hardware AFE protection interaction, invalid parameter transactions, NVM corruption/interruption, watchdog starvation, malformed communication and low-power wake cases.

## Evidence

Each run records firmware/boot Git SHA, target config hash, hardware revision/fixture revision, tool versions, test-case IDs, measurements, pass/fail and raw logs. Release manifest links the accepted HIL run.

## Status

Planned. Hosted software gates are already present; real lab runner/fixture and hardware-specific cases remain hardware-pending.
