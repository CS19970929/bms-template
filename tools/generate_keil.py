#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run(cmd: list[str]) -> None:
    print('+', ' '.join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def load_sources(path: Path) -> list[Path]:
    result: list[Path] = []
    seen: set[Path] = set()
    for line in path.read_text(encoding='utf-8').splitlines():
        if not line.strip():
            continue
        p = Path(line.strip())
        if not p.is_absolute():
            p = ROOT / p
        p = p.resolve()
        if p not in seen:
            seen.add(p)
            result.append(p)
    return result


def group_name(path: Path) -> str:
    rel = path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else path.as_posix()
    if '/StdPeriph/' in rel or rel.startswith('vendor/'):
        return 'Vendor StdPeriph'
    if rel.startswith('bootloader/core/'):
        return 'Boot Core'
    if rel.startswith('app/core/'):
        return 'App Core'
    if rel.startswith('common/') or rel.startswith('protocol/'):
        return 'Common'
    if rel.startswith('platform/'):
        return 'Platform'
    return 'Target'


def add_text(parent: ET.Element, tag: str, text: str | int) -> ET.Element:
    node = ET.SubElement(parent, tag)
    node.text = str(text)
    return node


def relative_windows(path: Path, project_dir: Path) -> str:
    return os.path.relpath(path, project_dir).replace('/', '\\')


def make_target(project: ET.Element, target_name: str, optim: int, image: str, sources: list[Path],
                project_dir: Path, cfg: dict, generated: Path) -> None:
    target = ET.SubElement(ET.SubElement(project, 'Targets'), 'Target') if project.find('Targets') is None else ET.SubElement(project.find('Targets'), 'Target')
    add_text(target, 'TargetName', target_name)
    add_text(target, 'ToolsetNumber', '0x4')
    add_text(target, 'ToolsetName', 'ARM-ADS')
    add_text(target, 'pCCUsed', '5060960::V5.06 update 7 (build 960)::.\\ARMCC')
    add_text(target, 'uAC6', 0)
    option = ET.SubElement(target, 'TargetOption')
    common = ET.SubElement(option, 'TargetCommonOption')
    add_text(common, 'Device', 'STM32F030C8')
    add_text(common, 'Vendor', 'STMicroelectronics')
    add_text(common, 'PackID', 'Keil.STM32F0xx_DFP.2.1.0')
    flash = cfg['flash']; ram = cfg['ram']
    flash_start = int(flash['boot_start'], 0) if image == 'boot' else int(flash['app_start'], 0)
    flash_size = int(flash['boot_size']) if image == 'boot' else int(flash['app_size'])
    ram_start = int(ram['start'], 0) + (0 if image == 'boot' else (0xC0 if cfg['mcu_family'] == 'stm32f0' else 0))
    ram_size = int(ram['size']) - (ram_start - int(ram['start'], 0))
    add_text(common, 'Cpu', f'IROM(0x{flash_start:08X},0x{flash_size:X}) IRAM(0x{ram_start:08X},0x{ram_size:X}) CPUTYPE("Cortex-M0") CLOCK(48000000) ELITTLE')
    add_text(common, 'OutputDirectory', f'.\\Objects\\{image}-{optim}\\')
    add_text(common, 'OutputName', f'bms_{image}')
    add_text(common, 'CreateExecutable', 1); add_text(common, 'CreateHexFile', 1); add_text(common, 'DebugInformation', 1)
    add_text(common, 'ListingPath', f'.\\Listings\\{image}-{optim}\\')
    arm = ET.SubElement(option, 'TargetArmAds')
    misc = ET.SubElement(arm, 'ArmAdsMisc'); add_text(misc, 'useUlib', 1); add_text(misc, 'AdsCpuType', '"Cortex-M0"')
    cads = ET.SubElement(arm, 'Cads')
    add_text(cads, 'Optim', optim)
    add_text(cads, 'oTime', 0)
    add_text(cads, 'wLevel', 2)
    add_text(cads, 'uThumb', 1)
    add_text(cads, 'VariousControls', '')
    defines = f'STM32F030,USE_STDPERIPH_DRIVER,{"BMS_BOOT_IMAGE" if image == "boot" else "BMS_APP_IMAGE"}'
    add_text(cads, 'Define', defines)
    includes = [
        ROOT/'common/include', ROOT/'protocol/include', ROOT/'bootloader/core/include', ROOT/'app/core/include',
        ROOT/'drivers/afe/include', ROOT/'platform/stm32f0/include', generated,
        ROOT/'vendor/st/stm32f0_stdperiph_v1.5.0/CMSIS', ROOT/'vendor/st/stm32f0_stdperiph_v1.5.0/StdPeriph/inc'
    ]
    add_text(cads, 'IncludePath', ';'.join(relative_windows(p, project_dir) for p in includes))
    ld = ET.SubElement(arm, 'LDads')
    add_text(ld, 'umfTarg', 1); add_text(ld, 'Ropi', 0); add_text(ld, 'Rwpi', 0)
    groups = ET.SubElement(target, 'Groups')
    by_group: dict[str, list[Path]] = {}
    for src in sources:
        by_group.setdefault(group_name(src), []).append(src)
    startup = ROOT/'platform/stm32f0/startup/startup_stm32f030_armcc.s'
    by_group.setdefault('Startup', []).insert(0, startup)
    for name, items in by_group.items():
        group = ET.SubElement(groups, 'Group'); add_text(group, 'GroupName', name); files = ET.SubElement(group, 'Files')
        for src in items:
            file = ET.SubElement(files, 'File'); add_text(file, 'FileName', src.name)
            add_text(file, 'FileType', 2 if src.suffix.lower() == '.s' else 1)
            add_text(file, 'FilePath', relative_windows(src, project_dir))


def generate(image: str, cfg: dict, build_dir: Path, output_dir: Path) -> Path:
    src_file = build_dir/'generated'/cfg['name']/f'keil_{image}_sources.txt'
    sources = load_sources(src_file)
    root = ET.Element('Project')
    add_text(root, 'SchemaVersion', '2.1')
    add_text(root, 'Header', '### uVision Project, generated from bms-template CMake configuration')
    for label, optim in [('Debug-O0', 1), ('Debug-O2', 3)]:
        make_target(root, label, optim, image, sources, output_dir, cfg, build_dir/'generated'/cfg['name'])
    ET.indent(root, space='  ')
    path = output_dir/f'bms_{image}.uvprojx'
    ET.ElementTree(root).write(path, encoding='utf-8', xml_declaration=True)
    return path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('--target', default='stm32f030c8_mock')
    parser.add_argument('--no-configure', action='store_true')
    parser.add_argument('--out', default=None)
    args = parser.parse_args()
    if args.target != 'stm32f030c8_mock':
        raise SystemExit('Keil generator currently supports the completed F030 port only')
    if not args.no_configure:
        run([sys.executable, 'tools/bootstrap_vendor.py'])
        run(['cmake', '--preset', 'f030', '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
    cfg = json.loads((ROOT/'config/targets'/f'{args.target}.json').read_text(encoding='utf-8'))
    build_dir = ROOT/'build/f030'
    out = Path(args.out) if args.out else ROOT/'ide/keil/generated'/args.target
    out.mkdir(parents=True, exist_ok=True)
    for image in ('boot', 'app'):
        print('generated', generate(image, cfg, build_dir, out))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
