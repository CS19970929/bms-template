#!/usr/bin/env python3
from __future__ import annotations
import argparse, shutil, subprocess, sys
from pathlib import Path
from generate_target import load_target
ROOT=Path(__file__).resolve().parents[1]
def run(cmd:list[str])->None: print("+"," ".join(cmd)); subprocess.run(cmd,cwd=ROOT,check=True)
def require(tool:str)->None:
 if shutil.which(tool) is None: raise SystemExit(f"missing required tool: {tool}")
def target_names()->list[str]: return sorted(path.stem for path in (ROOT/"config/targets").glob("*.json"))
def print_targets()->None:
 for name in target_names():
  cfg=load_target(name); print(f"{name}: mcu={cfg['mcu']['part']} afe={cfg['afe']['name']} board={cfg['board']['name']} product={cfg['product']['name']} cells={cfg['product']['cell_count']} app={cfg['flash']['app_size']}B")
def build_firmware(target:str)->None:
 require("cmake"); require("ninja"); require("arm-none-eabi-gcc"); cfg=load_target(target); bootstrap={"stm32f0":"f0","stm32f1":"f1"}.get(cfg["mcu"]["family"])
 if bootstrap is None: raise SystemExit(f"no vendor bootstrap mapping for MCU family {cfg['mcu']['family']}")
 run([sys.executable,"tools/bootstrap_vendor.py","--family",bootstrap]); build_dir=ROOT/"build/targets"/target
 run(["cmake","--fresh","-S",str(ROOT),"-B",str(build_dir),"-G","Ninja",f"-DCMAKE_TOOLCHAIN_FILE={ROOT/'cmake/toolchains/arm-none-eabi.cmake'}","-DCMAKE_BUILD_TYPE=Release","-DCMAKE_C_FLAGS_RELEASE=-O2 -DNDEBUG","-DBMS_BUILD_HOST_TESTS=OFF",f"-DBMS_TARGET={target}"]); run(["cmake","--build",str(build_dir)]); run([sys.executable,"tools/check_firmware_image.py","--build-dir",str(build_dir),"--target",target])
def build_pc()->None:
 require("dotnet"); run(["dotnet","restore","pc/BmsTool.sln","--nologo"]); run(["dotnet","build","pc/BmsTool.sln","-c","Release","--no-restore","--nologo","-warnaserror"]); run(["dotnet","run","--project","pc/tests/Bms.Protocol.Smoke/Bms.Protocol.Smoke.csproj","-c","Release","--no-build"])
def main()->int:
 p=argparse.ArgumentParser(description="Unified bms-template developer entry point"); sub=p.add_subparsers(dest="command",required=True); sub.add_parser("targets"); sub.add_parser("check"); sub.add_parser("schema")
 build=sub.add_parser("build"); build.add_argument("--target",required=True,choices=target_names()); keil=sub.add_parser("keil"); keil.add_argument("--target",required=True,choices=target_names()); sub.add_parser("pc"); all_cmd=sub.add_parser("all"); all_cmd.add_argument("--target",required=True,choices=target_names()); args=p.parse_args()
 if args.command=="targets": print_targets()
 elif args.command=="check": run([sys.executable,"tools/check.py"])
 elif args.command=="schema":
  for generator in ("commands","parameters","protections","events"): run([sys.executable,f"tools/generate_{generator}.py","--write"])
 elif args.command=="build": build_firmware(args.target)
 elif args.command=="keil": run([sys.executable,"tools/generate_keil.py","--target",args.target])
 elif args.command=="pc": build_pc()
 elif args.command=="all": run([sys.executable,"tools/check.py"]); build_firmware(args.target); build_pc()
 else: p.error("unknown command")
 return 0
if __name__=="__main__": raise SystemExit(main())
