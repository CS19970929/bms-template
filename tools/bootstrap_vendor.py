#!/usr/bin/env python3
"""Fetch only whitelisted ST CMSIS/StdPeriph files from pinned reference commits.

Reference repositories are content sources only. Files known to contain project-specific
modifications (for example the old system_stm32f0xx.c) are deliberately not imported.
"""
from __future__ import annotations

import hashlib
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REF_REPO = "CS19970929/030-309-template"
REF_COMMIT = "3cc2db3f263bb466ed2292dfbc75d4a7f09c223c"
BASE = f"https://raw.githubusercontent.com/{REF_REPO}/{REF_COMMIT}/"
DEST = ROOT / "vendor" / "st" / "stm32f0_stdperiph_v1.5.0"

FILES = {
    "CMSIS/core_cm0.h": ("Code/Drivers/core_cm0.h", "ab31de0ee87f9cb566cef040364c80c1f09bbf86"),
    "CMSIS/core_cmFunc.h": ("Code/Drivers/core_cmFunc.h", "0a18fafc301e003d348edf5cae39481d8e5fe7c3"),
    "CMSIS/core_cmInstr.h": ("Code/Drivers/core_cmInstr.h", "d213f0eed7ca9335e883a5b55b6de14ba9507f1e"),
    "CMSIS/stm32f0xx.h": ("Code/Drivers/stm32f0xx.h", "3dfe2f51d205bc480b7062323eb35912b19ab5fe"),
    "CMSIS/system_stm32f0xx.h": ("Code/Drivers/system_stm32f0xx.h", "12027a8b223973ed8d957e7ee01be2a0a719200f"),
    "StdPeriph/inc/stm32f0xx_flash.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_flash.h", "236cdde64bf07090b0da4057a8efb8c585ddbd48"),
    "StdPeriph/inc/stm32f0xx_gpio.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_gpio.h", "867e4d8fe5564127b3ea703e0f81d5211d27785f"),
    "StdPeriph/inc/stm32f0xx_iwdg.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_iwdg.h", "1f2eece89e1bed1d7f916a9930444cfff7488d32"),
    "StdPeriph/inc/stm32f0xx_misc.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_misc.h", "3811f0fd4eee7e47d53182f42b052422a04e10c0"),
    "StdPeriph/inc/stm32f0xx_rcc.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_rcc.h", "380f98d1766f24b7389dba6333dc315452385f2a"),
    "StdPeriph/inc/stm32f0xx_syscfg.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_syscfg.h", "8c9641443a2dd5097d25837cb45a37782beb81a2"),
    "StdPeriph/inc/stm32f0xx_usart.h": ("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_usart.h", "2e732524a37aea5120bfc086e136ece776029f78"),
    "StdPeriph/src/stm32f0xx_flash.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_flash.c", "54a8833822f41c7ecabde557f999f2004635899b"),
    "StdPeriph/src/stm32f0xx_gpio.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_gpio.c", "5b6eb2347dae97dba355a0f46d6ac5ea7eec35da"),
    "StdPeriph/src/stm32f0xx_iwdg.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_iwdg.c", "b864cb5b9fbae4bc8c6b3fd92b04aa79db04a456"),
    "StdPeriph/src/stm32f0xx_misc.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_misc.c", "d44d7fe04f234e2a9e09e82ae35f6a22fcf9fc48"),
    "StdPeriph/src/stm32f0xx_rcc.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_rcc.c", "5b8af2c6049ec649c714e08c896490915fb26bcc"),
    "StdPeriph/src/stm32f0xx_syscfg.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_syscfg.c", "5c7cdd3d319ec0e90cbe1913124819d8b4038260"),
    "StdPeriph/src/stm32f0xx_usart.c": ("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_usart.c", "46f7b74ae1747ebf7a990dcfebd5450b6988f6ef"),
}


def git_blob_sha(data: bytes) -> str:
    return hashlib.sha1(f"blob {len(data)}\0".encode("ascii") + data).hexdigest()


def main() -> int:
    for rel, (source, expected) in FILES.items():
        dst = DEST / rel
        if dst.exists() and git_blob_sha(dst.read_bytes()) == expected:
            print(f"[ok] {rel}")
            continue
        print(f"[get] {source}")
        with urllib.request.urlopen(BASE + source, timeout=30) as response:
            data = response.read()
        actual = git_blob_sha(data)
        if actual != expected:
            print(f"hash mismatch for {source}: expected {expected}, got {actual}", file=sys.stderr)
            return 2
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(data)
    print(f"vendor ready: {DEST}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
