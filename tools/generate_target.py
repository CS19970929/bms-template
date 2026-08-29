#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
TOPOLOGY_IDS = {"common": 0, "separate": 1}
SUPPORTED_VECTOR_MODES = {"sram-remap", "vtor"}


def as_int(value: int | str) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


def load_json(kind: str, name: str) -> dict[str, Any]:
    path = ROOT / "config" / kind / f"{name}.json"
    if not path.is_file():
        raise ValueError(f"missing {kind} profile: {name} ({path})")
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("name") != name:
        raise ValueError(f"{path}: name must be {name!r}")
    return data


def load_target(name: str) -> dict[str, Any]:
    target_path = ROOT / "config" / "targets" / f"{name}.json"
    if not target_path.is_file():
        raise ValueError(f"missing target: {name} ({target_path})")
    target = json.loads(target_path.read_text(encoding="utf-8"))
    required = ("name", "mcu", "afe", "board", "product")
    missing = [key for key in required if key not in target]
    if missing:
        raise ValueError(f"{target_path}: missing keys: {', '.join(missing)}")
    if target["name"] != name:
        raise ValueError(f"{target_path}: name must be {name!r}")

    mcu = load_json("mcu", str(target["mcu"]))
    afe = load_json("afe", str(target["afe"]))
    board = load_json("boards", str(target["board"]))
    product = load_json("products", str(target["product"]))

    validate_profiles(mcu, afe, board, product)

    flash_start = as_int(mcu["flash"]["start"])
    flash_size = as_int(mcu["flash"]["size"])
    page_size = as_int(mcu["flash"]["page_size"])
    boot_size = as_int(product["firmware"]["boot_size"])
    persistent_size = as_int(product["firmware"]["persistent_size"])
    flash_end = flash_start + flash_size
    boot_start = flash_start
    app_start = boot_start + boot_size
    persistent_start = flash_end - persistent_size
    app_size = persistent_start - app_start

    if boot_size <= 0 or persistent_size <= 0 or app_size <= 0:
        raise ValueError("firmware flash regions must all be non-empty")
    if (boot_size % page_size) != 0:
        raise ValueError("product boot_size must align to MCU flash page_size")
    if (persistent_size % page_size) != 0:
        raise ValueError("product persistent_size must align to MCU flash page_size")
    if persistent_size < (4 * page_size):
        raise ValueError("persistent_size must provide at least four MCU flash pages")
    if (app_start % page_size) != 0 or (persistent_start % page_size) != 0:
        raise ValueError("derived APP/persistent regions must be page aligned")

    metadata_a = persistent_start
    metadata_b = persistent_start + page_size
    nvm_a = flash_end - (2 * page_size)
    nvm_b = flash_end - page_size
    if (metadata_b + page_size) > nvm_a:
        raise ValueError("persistent tail cannot separate Boot metadata and APP NVM slots")

    ram_start = as_int(mcu["ram"]["start"])
    ram_size = as_int(mcu["ram"]["size"])
    vector_reserved = as_int(mcu["vector"]["reserved_ram"])
    if vector_reserved < 0 or vector_reserved >= ram_size:
        raise ValueError("invalid MCU vector reserved_ram")

    return {
        "name": name,
        "refs": {
            "mcu": target["mcu"],
            "afe": target["afe"],
            "board": target["board"],
            "product": target["product"],
        },
        "mcu": mcu,
        "afe": afe,
        "board": board,
        "product": product,
        "flash": {
            "start": flash_start,
            "size": flash_size,
            "end": flash_end,
            "page_size": page_size,
            "boot_start": boot_start,
            "boot_size": boot_size,
            "app_start": app_start,
            "app_size": app_size,
            "app_end": persistent_start,
            "persistent_start": persistent_start,
            "persistent_size": persistent_size,
            "metadata_a": metadata_a,
            "metadata_b": metadata_b,
            "metadata_slot_size": page_size,
            "nvm_a": nvm_a,
            "nvm_b": nvm_b,
            "nvm_slot_size": page_size,
        },
        "ram": {
            "start": ram_start,
            "size": ram_size,
            "end": ram_start + ram_size,
            "vector_reserved": vector_reserved,
        },
    }


def validate_profiles(
    mcu: dict[str, Any],
    afe: dict[str, Any],
    board: dict[str, Any],
    product: dict[str, Any],
) -> None:
    for key in (
        "id",
        "family",
        "core",
        "device_define",
        "keil_device",
        "keil_pack",
        "clock_hz",
        "flash",
        "ram",
        "vector",
        "vendor",
    ):
        if key not in mcu:
            raise ValueError(f"MCU {mcu['name']}: missing {key}")

    if int(mcu["id"]) <= 0:
        raise ValueError("MCU id must be positive")
    if as_int(mcu["clock_hz"]) <= 0:
        raise ValueError("MCU clock_hz must be positive")
    if as_int(mcu["flash"]["size"]) <= 0 or as_int(mcu["flash"]["page_size"]) <= 0:
        raise ValueError("MCU flash size/page_size must be positive")
    if as_int(mcu["ram"]["size"]) <= 0:
        raise ValueError("MCU RAM size must be positive")
    if mcu["vector"].get("mode") not in SUPPORTED_VECTOR_MODES:
        raise ValueError(f"unsupported vector mode: {mcu['vector'].get('mode')}")

    cell_count = int(product.get("cell_count", 0))
    if not 1 <= cell_count <= 32:
        raise ValueError("product cell_count must be 1..32")
    if int(product.get("product_id", 0)) <= 0:
        raise ValueError("product product_id must be positive")
    if cell_count > int(afe.get("max_cells", 0)):
        raise ValueError(
            f"product requires {cell_count} cells but AFE {afe['name']} supports only {afe.get('max_cells')}"
        )
    if board.get("port_topology") not in TOPOLOGY_IDS:
        raise ValueError(f"unsupported board port_topology: {board.get('port_topology')}")
    firmware = product.get("firmware")
    if not isinstance(firmware, dict):
        raise ValueError("product firmware layout policy is missing")
    if "boot_size" not in firmware or "persistent_size" not in firmware:
        raise ValueError("product firmware policy requires boot_size and persistent_size")


def write_header(cfg: dict[str, Any], out: Path) -> None:
    mcu = cfg["mcu"]
    afe = cfg["afe"]
    board = cfg["board"]
    product = cfg["product"]
    flash = cfg["flash"]
    ram = cfg["ram"]

    text = f"""#ifndef BMS_TARGET_CONFIG_H
#define BMS_TARGET_CONFIG_H

#define BMS_TARGET_NAME \"{cfg['name']}\"
#define BMS_TARGET_MCU_NAME \"{mcu['part']}\"
#define BMS_TARGET_MCU_ID {int(mcu['id'])}U
#define BMS_TARGET_MCU_FAMILY \"{mcu['family']}\"
#define BMS_TARGET_AFE_NAME \"{afe['name']}\"
#define BMS_TARGET_BOARD_NAME \"{board['name']}\"
#define BMS_TARGET_PRODUCT_NAME \"{product['name']}\"
#define BMS_TARGET_PRODUCT_ID {int(product['product_id'])}UL
#define BMS_TARGET_CELL_COUNT {int(product['cell_count'])}U
#define BMS_TARGET_PORT_TOPOLOGY {TOPOLOGY_IDS[board['port_topology']]}U
#define BMS_TARGET_FLASH_PAGE_SIZE {flash['page_size']}UL
#define BMS_TARGET_BOOT_START 0x{flash['boot_start']:08X}UL
#define BMS_TARGET_BOOT_SIZE {flash['boot_size']}UL
#define BMS_TARGET_APP_START 0x{flash['app_start']:08X}UL
#define BMS_TARGET_APP_SIZE {flash['app_size']}UL
#define BMS_TARGET_APP_END 0x{flash['app_end']:08X}UL
#define BMS_TARGET_PERSISTENT_START 0x{flash['persistent_start']:08X}UL
#define BMS_TARGET_PERSISTENT_SIZE {flash['persistent_size']}UL
#define BMS_TARGET_METADATA_A 0x{flash['metadata_a']:08X}UL
#define BMS_TARGET_METADATA_B 0x{flash['metadata_b']:08X}UL
#define BMS_TARGET_METADATA_SLOT_SIZE {flash['metadata_slot_size']}UL
#define BMS_TARGET_NVM_A 0x{flash['nvm_a']:08X}UL
#define BMS_TARGET_NVM_B 0x{flash['nvm_b']:08X}UL
#define BMS_TARGET_NVM_SLOT_SIZE {flash['nvm_slot_size']}UL
#define BMS_TARGET_FLASH_END 0x{flash['end']:08X}UL
#define BMS_TARGET_RAM_START 0x{ram['start']:08X}UL
#define BMS_TARGET_RAM_SIZE {ram['size']}UL
#define BMS_TARGET_RAM_END 0x{ram['end']:08X}UL
#define BMS_TARGET_VECTOR_RESERVED {ram['vector_reserved']}UL
#define BMS_TARGET_CLOCK_HZ {as_int(mcu['clock_hz'])}UL

#endif
"""
    out.write_text(text, encoding="utf-8")


def write_cmake(cfg: dict[str, Any], out: Path) -> None:
    mcu = cfg["mcu"]
    lines = [
        "# GENERATED - do not hand edit.",
        f'set(BMS_GENERATED_TARGET "{cfg["name"]}")',
        f'set(BMS_GENERATED_MCU_FAMILY "{mcu["family"]}")',
        f'set(BMS_GENERATED_MCU_PART "{mcu["part"]}")',
        f'set(BMS_GENERATED_MCU_ID "{int(mcu["id"])}")',
        f'set(BMS_GENERATED_MCU_CORE "{mcu["core"]}")',
        f'set(BMS_GENERATED_MCU_DEFINE "{mcu["device_define"]}")',
        f'set(BMS_GENERATED_MCU_VENDOR "{mcu["vendor"]}")',
        f'set(BMS_GENERATED_FLASH_PAGE_SIZE "{cfg["flash"]["page_size"]}")',
        f'set(BMS_GENERATED_VECTOR_MODE "{mcu["vector"]["mode"]}")',
        f'set(BMS_GENERATED_CLOCK_HZ "{as_int(mcu["clock_hz"])}")',
        f'set(BMS_GENERATED_KEIL_DEVICE "{mcu["keil_device"]}")',
        f'set(BMS_GENERATED_KEIL_PACK "{mcu["keil_pack"]}")',
        f'set(BMS_GENERATED_AFE "{cfg["afe"]["name"]}")',
        f'set(BMS_GENERATED_BOARD "{cfg["board"]["name"]}")',
        f'set(BMS_GENERATED_PRODUCT "{cfg["product"]["name"]}")',
        "",
    ]
    out.write_text("\n".join(lines), encoding="utf-8")


def linker_text(cfg: dict[str, Any], app: bool) -> str:
    flash = cfg["flash"]
    ram = cfg["ram"]
    flash_start = flash["app_start"] if app else flash["boot_start"]
    flash_size = flash["app_size"] if app else flash["boot_size"]
    reserved = ram["vector_reserved"] if app else 0
    usable_ram_start = ram["start"] + reserved
    usable_ram_size = ram["size"] - reserved
    return f"""/* GENERATED from layered target config {cfg['name']} - do not hand edit. */
MEMORY
{{
  FLASH (rx) : ORIGIN = 0x{flash_start:08X}, LENGTH = {flash_size}
  RAM (xrw)  : ORIGIN = 0x{usable_ram_start:08X}, LENGTH = {usable_ram_size}
}}
_estack = 0x{ram['end']:08X};
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


def write_summary(cfg: dict[str, Any], out: Path) -> None:
    summary = {
        "name": cfg["name"],
        "refs": cfg["refs"],
        "resolved": {
            "mcu": cfg["mcu"]["part"],
            "mcu_id": cfg["mcu"]["id"],
            "mcu_family": cfg["mcu"]["family"],
            "afe": cfg["afe"]["name"],
            "board": cfg["board"]["name"],
            "product": cfg["product"]["name"],
            "product_id": cfg["product"]["product_id"],
            "cell_count": cfg["product"]["cell_count"],
            "port_topology": cfg["board"]["port_topology"],
            "flash": cfg["flash"],
            "ram": cfg["ram"],
        },
    }
    out.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")


def generate(name: str, out: Path) -> dict[str, Any]:
    cfg = load_target(name)
    out.mkdir(parents=True, exist_ok=True)
    write_header(cfg, out / "bms_target_config.h")
    write_cmake(cfg, out / "target.cmake")
    (out / "boot.ld").write_text(linker_text(cfg, False), encoding="utf-8")
    (out / "app.ld").write_text(linker_text(cfg, True), encoding="utf-8")
    write_summary(cfg, out / "target-summary.json")
    return cfg


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    out = Path(args.out)
    cfg = generate(args.target, out)
    print(
        f"generated target {cfg['name']} -> {out} "
        f"({cfg['mcu']['part']} + {cfg['afe']['name']} + {cfg['board']['name']} + {cfg['product']['name']})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
