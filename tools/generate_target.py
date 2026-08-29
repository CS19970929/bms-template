#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MCU_IDS = {"STM32F030C8T6": 1, "STM32F103C8T6": 2}
TOPOLOGY_IDS = {"common": 0, "separate": 1}
VECTOR_RESERVED = {"stm32f0": 0xC0, "stm32f1": 0}


def as_int(value: int | str) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


def validate(cfg: dict) -> None:
    required = ["name", "mcu", "mcu_family", "afe", "product_id", "cell_count", "port_topology", "flash", "ram"]
    missing = [key for key in required if key not in cfg]
    if missing:
        raise ValueError(f"missing target keys: {', '.join(missing)}")
    if cfg["mcu"] not in MCU_IDS:
        raise ValueError(f"unsupported MCU id mapping: {cfg['mcu']}")
    if cfg["port_topology"] not in TOPOLOGY_IDS:
        raise ValueError(f"unsupported port_topology: {cfg['port_topology']}")
    if cfg["mcu_family"] not in VECTOR_RESERVED:
        raise ValueError(f"unsupported MCU family: {cfg['mcu_family']}")
    if not (1 <= int(cfg["cell_count"]) <= 32):
        raise ValueError("cell_count must be 1..32")

    f = cfg["flash"]
    boot_start, boot_size = as_int(f["boot_start"]), as_int(f["boot_size"])
    app_start, app_size = as_int(f["app_start"]), as_int(f["app_size"])
    meta_start, meta_size = as_int(f["metadata_start"]), as_int(f["metadata_size"])
    if boot_start + boot_size != app_start:
        raise ValueError("boot region must end exactly at app_start")
    if app_start + app_size != meta_start:
        raise ValueError("app region must end exactly at metadata_start")
    if meta_size < 2048:
        raise ValueError("metadata region must provide at least two 1 KiB records/pages")
    if (boot_start | app_start | meta_start) & 1:
        raise ValueError("flash regions must be at least halfword aligned")

    r = cfg["ram"]
    ram_size = as_int(r["size"])
    reserved = VECTOR_RESERVED[cfg["mcu_family"]]
    if ram_size <= reserved:
        raise ValueError("RAM is too small for vector reservation")


def write_header(cfg: dict, out: Path) -> None:
    f, r = cfg["flash"], cfg["ram"]
    boot_start, boot_size = as_int(f["boot_start"]), as_int(f["boot_size"])
    app_start, app_size = as_int(f["app_start"]), as_int(f["app_size"])
    meta_start, meta_size = as_int(f["metadata_start"]), as_int(f["metadata_size"])
    ram_start, ram_size = as_int(r["start"]), as_int(r["size"])
    text = f"""#ifndef BMS_TARGET_CONFIG_H
#define BMS_TARGET_CONFIG_H

#define BMS_TARGET_NAME \"{cfg['name']}\"
#define BMS_TARGET_MCU_NAME \"{cfg['mcu']}\"
#define BMS_TARGET_AFE_NAME \"{cfg['afe']}\"
#define BMS_TARGET_MCU_ID {MCU_IDS[cfg['mcu']]}U
#define BMS_TARGET_PRODUCT_ID {int(cfg['product_id'])}UL
#define BMS_TARGET_CELL_COUNT {int(cfg['cell_count'])}U
#define BMS_TARGET_PORT_TOPOLOGY {TOPOLOGY_IDS[cfg['port_topology']]}U
#define BMS_TARGET_BOOT_START 0x{boot_start:08X}UL
#define BMS_TARGET_BOOT_SIZE {boot_size}UL
#define BMS_TARGET_APP_START 0x{app_start:08X}UL
#define BMS_TARGET_APP_SIZE {app_size}UL
#define BMS_TARGET_APP_END 0x{app_start + app_size:08X}UL
#define BMS_TARGET_METADATA_START 0x{meta_start:08X}UL
#define BMS_TARGET_METADATA_SIZE {meta_size}UL
#define BMS_TARGET_METADATA_A 0x{meta_start:08X}UL
#define BMS_TARGET_METADATA_B 0x{meta_start + 1024:08X}UL
#define BMS_TARGET_FLASH_END 0x{meta_start + meta_size:08X}UL
#define BMS_TARGET_RAM_START 0x{ram_start:08X}UL
#define BMS_TARGET_RAM_SIZE {ram_size}UL
#define BMS_TARGET_RAM_END 0x{ram_start + ram_size:08X}UL
#define BMS_TARGET_VECTOR_RESERVED {VECTOR_RESERVED[cfg['mcu_family']]}UL

#endif
"""
    out.write_text(text, encoding="utf-8")


def linker_text(cfg: dict, app: bool) -> str:
    f, r = cfg["flash"], cfg["ram"]
    flash_start = as_int(f["app_start"] if app else f["boot_start"])
    flash_size = as_int(f["app_size"] if app else f["boot_size"])
    ram_start, ram_size = as_int(r["start"]), as_int(r["size"])
    reserved = VECTOR_RESERVED[cfg["mcu_family"]] if app else 0
    usable_ram_start = ram_start + reserved
    usable_ram_size = ram_size - reserved
    return f"""/* GENERATED from config/targets/{cfg['name']}.json - do not hand edit. */
MEMORY
{{
  FLASH (rx) : ORIGIN = 0x{flash_start:08X}, LENGTH = {flash_size}
  RAM (xrw)  : ORIGIN = 0x{usable_ram_start:08X}, LENGTH = {usable_ram_size}
}}
_estack = 0x{ram_start + ram_size:08X};
SECTIONS
{{
  .isr_vector : {{ KEEP(*(.isr_vector)) }} > FLASH
  .text : {{ *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini)) }} > FLASH
  .ARM.exidx : {{ *(.ARM.exidx*) }} > FLASH
  _sidata = LOADADDR(.data);
  .data : {{ _sdata = .; *(.data*) _edata = .; }} > RAM AT> FLASH
  .bss : {{ _sbss = .; *(.bss*) *(COMMON) _ebss = .; }} > RAM
  . = ALIGN(8); _end = .;
}}
"""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    path = ROOT / "config" / "targets" / f"{args.target}.json"
    cfg = json.loads(path.read_text(encoding="utf-8"))
    validate(cfg)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    write_header(cfg, out / "bms_target_config.h")
    (out / "boot.ld").write_text(linker_text(cfg, False), encoding="utf-8")
    (out / "app.ld").write_text(linker_text(cfg, True), encoding="utf-8")
    summary = {
        "name": cfg["name"], "mcu": cfg["mcu"], "mcu_family": cfg["mcu_family"], "afe": cfg["afe"],
        "product_id": cfg["product_id"], "cell_count": cfg["cell_count"], "port_topology": cfg["port_topology"],
        "vector_reserved": VECTOR_RESERVED[cfg["mcu_family"]]
    }
    (out / "target-summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(f"generated target {cfg['name']} -> {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
