#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any

from generate_target import load_target

ROOT = Path(__file__).resolve().parents[1]


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def load_sources(path: Path) -> list[Path]:
    result: list[Path] = []
    seen: set[Path] = set()
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        source = Path(line.strip())
        if not source.is_absolute():
            source = ROOT / source
        source = source.resolve()
        if source not in seen:
            seen.add(source)
            result.append(source)
    return result


def group_name(path: Path) -> str:
    rel = path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else path.as_posix()
    if "/StdPeriph/" in rel or rel.startswith("vendor/"):
        return "Vendor StdPeriph"
    if rel.startswith("bootloader/core/"):
        return "Boot Core"
    if rel.startswith("app/core/"):
        return "App Core"
    if rel.startswith("common/") or rel.startswith("protocol/"):
        return "Common"
    if rel.startswith("platform/"):
        return "Platform"
    if rel.startswith("drivers/"):
        return "Drivers"
    return "Target"


def add_text(parent: ET.Element, tag: str, text: str | int) -> ET.Element:
    node = ET.SubElement(parent, tag)
    node.text = str(text)
    return node


def relative_windows(path: Path, project_dir: Path) -> str:
    return os.path.relpath(path, project_dir).replace("/", "\\")


def family_paths(cfg: dict[str, Any], generated: Path) -> tuple[list[Path], Path]:
    family = cfg["mcu"]["family"]
    vendor = cfg["mcu"]["vendor"]
    if family == "stm32f0":
        vendor_root = ROOT / "vendor/st" / vendor
        includes = [
            ROOT / "platform/stm32f0/include",
            generated,
            vendor_root / "CMSIS",
            vendor_root / "StdPeriph/inc",
        ]
        startup = ROOT / "platform/stm32f0/startup/startup_stm32f030_armcc.s"
        return includes, startup
    if family == "stm32f1":
        vendor_root = ROOT / "vendor/st" / vendor
        includes = [
            ROOT / "platform/stm32f1/include",
            generated,
            vendor_root / "CMSIS",
            vendor_root / "StdPeriph/inc",
        ]
        startup = ROOT / "platform/stm32f1/startup/startup_stm32f103_armcc.s"
        return includes, startup
    raise ValueError(f"Keil generator does not support MCU family {family}")


def make_target(
    project: ET.Element,
    target_name: str,
    optim: int,
    image: str,
    sources: list[Path],
    project_dir: Path,
    cfg: dict[str, Any],
    generated: Path,
) -> None:
    targets = project.find("Targets")
    if targets is None:
        targets = ET.SubElement(project, "Targets")
    target = ET.SubElement(targets, "Target")
    add_text(target, "TargetName", target_name)
    add_text(target, "ToolsetNumber", "0x4")
    add_text(target, "ToolsetName", "ARM-ADS")
    add_text(target, "pCCUsed", "5060960::V5.06 update 7 (build 960)::.\\ARMCC")
    add_text(target, "uAC6", 0)

    option = ET.SubElement(target, "TargetOption")
    common = ET.SubElement(option, "TargetCommonOption")
    mcu = cfg["mcu"]
    flash = cfg["flash"]
    ram = cfg["ram"]
    add_text(common, "Device", mcu["keil_device"])
    add_text(common, "Vendor", "STMicroelectronics")
    add_text(common, "PackID", mcu["keil_pack"])
    add_text(common, "PackURL", "http://www.keil.com/pack/")

    flash_start = flash["boot_start"] if image == "boot" else flash["app_start"]
    flash_size = flash["boot_size"] if image == "boot" else flash["app_size"]
    ram_start = ram["start"] + (0 if image == "boot" else ram["vector_reserved"])
    ram_size = ram["size"] - (ram_start - ram["start"])
    cpu_type = "Cortex-M0" if mcu["core"] == "cortex-m0" else "Cortex-M3"
    add_text(
        common,
        "Cpu",
        f'IROM(0x{flash_start:08X},0x{flash_size:X}) '
        f'IRAM(0x{ram_start:08X},0x{ram_size:X}) '
        f'CPUTYPE("{cpu_type}") CLOCK({int(mcu["clock_hz"])}) ELITTLE',
    )
    add_text(common, "OutputDirectory", f".\\Objects\\{image}-{optim}\\")
    add_text(common, "OutputName", f"bms_{image}")
    add_text(common, "CreateExecutable", 1)
    add_text(common, "CreateHexFile", 1)
    add_text(common, "DebugInformation", 1)
    add_text(common, "ListingPath", f".\\Listings\\{image}-{optim}\\")

    arm = ET.SubElement(option, "TargetArmAds")
    misc = ET.SubElement(arm, "ArmAdsMisc")
    add_text(misc, "useUlib", 1)
    add_text(misc, "AdsCpuType", f'"{cpu_type}"')

    cads = ET.SubElement(arm, "Cads")
    add_text(cads, "Optim", optim)
    add_text(cads, "oTime", 0)
    add_text(cads, "wLevel", 2)
    add_text(cads, "uThumb", 1)
    add_text(cads, "VariousControls", "")
    image_define = "BMS_BOOT_IMAGE" if image == "boot" else "BMS_APP_IMAGE"
    defines = f'{mcu["device_define"]},USE_STDPERIPH_DRIVER,{image_define}'
    add_text(cads, "Define", defines)

    family_includes, startup = family_paths(cfg, generated)
    includes = [
        ROOT / "common/include",
        ROOT / "protocol/include",
        ROOT / "bootloader/core/include",
        ROOT / "app/core/include",
        ROOT / "drivers/afe/include",
        *family_includes,
    ]
    add_text(cads, "IncludePath", ";".join(relative_windows(p, project_dir) for p in includes))

    ld = ET.SubElement(arm, "LDads")
    add_text(ld, "umfTarg", 1)
    add_text(ld, "Ropi", 0)
    add_text(ld, "Rwpi", 0)

    groups = ET.SubElement(target, "Groups")
    by_group: dict[str, list[Path]] = {}
    for source in sources:
        by_group.setdefault(group_name(source), []).append(source)
    by_group.setdefault("Startup", []).insert(0, startup)

    for name, items in by_group.items():
        group = ET.SubElement(groups, "Group")
        add_text(group, "GroupName", name)
        files = ET.SubElement(group, "Files")
        for source in items:
            file_node = ET.SubElement(files, "File")
            add_text(file_node, "FileName", source.name)
            add_text(file_node, "FileType", 2 if source.suffix.lower() == ".s" else 1)
            add_text(file_node, "FilePath", relative_windows(source, project_dir))


def generate(
    image: str,
    cfg: dict[str, Any],
    build_dir: Path,
    output_dir: Path,
) -> Path:
    generated = build_dir / "generated" / cfg["name"]
    source_file = generated / f"keil_{image}_sources.txt"
    sources = load_sources(source_file)

    root = ET.Element("Project")
    add_text(root, "SchemaVersion", "2.1")
    add_text(root, "Header", "### uVision Project, generated from bms-template configuration")
    for label, optim in (("Debug-O0", 1), ("Debug-O2", 3)):
        make_target(root, label, optim, image, sources, output_dir, cfg, generated)

    ET.indent(root, space="  ")
    path = output_dir / f"bms_{image}.uvprojx"
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)
    return path


def configure_for_keil(target_name: str, cfg: dict[str, Any], build_dir: Path) -> None:
    family = cfg["mcu"]["family"]
    bootstrap_family = {"stm32f0": "f0", "stm32f1": "f1"}.get(family)
    if bootstrap_family is None:
        raise ValueError(f"unsupported MCU family for vendor bootstrap: {family}")
    run([sys.executable, "tools/bootstrap_vendor.py", "--family", bootstrap_family])
    run(
        [
            "cmake",
            "--fresh",
            "-S",
            str(ROOT),
            "-B",
            str(build_dir),
            "-G",
            "Ninja",
            f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/toolchains/arm-none-eabi.cmake'}",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_C_FLAGS_RELEASE=-O2 -DNDEBUG",
            "-DBMS_BUILD_HOST_TESTS=OFF",
            f"-DBMS_TARGET={target_name}",
        ]
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", default="stm32f030c8_mock")
    parser.add_argument("--no-configure", action="store_true")
    parser.add_argument("--out", default=None)
    args = parser.parse_args()

    cfg = load_target(args.target)
    build_dir = ROOT / "build/keil" / args.target
    if not args.no_configure:
        configure_for_keil(args.target, cfg, build_dir)

    out = Path(args.out) if args.out else ROOT / "ide/keil/generated" / args.target
    out.mkdir(parents=True, exist_ok=True)
    for image in ("boot", "app"):
        print("generated", generate(image, cfg, build_dir, out))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
