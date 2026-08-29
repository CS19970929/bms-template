#!/usr/bin/env python3
from __future__ import annotations
import argparse,hashlib,sys,urllib.request
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def git_blob_sha(data:bytes)->str:return hashlib.sha1(f"blob {len(data)}\0".encode('ascii')+data).hexdigest()

def sync(repo:str,commit:str,dest:Path,files:dict[str,tuple[str,str]])->int:
    base=f"https://raw.githubusercontent.com/{repo}/{commit}/"
    for rel,(source,expected) in files.items():
        dst=dest/rel
        if dst.exists() and git_blob_sha(dst.read_bytes())==expected:print(f"[ok] {rel}");continue
        print(f"[get] {source}")
        with urllib.request.urlopen(base+source,timeout=30) as response:data=response.read()
        actual=git_blob_sha(data)
        if actual!=expected:print(f"hash mismatch {source}: {expected} != {actual}",file=sys.stderr);return 2
        dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(data)
    return 0

F0={
"CMSIS/core_cm0.h":("Code/Drivers/core_cm0.h","ab31de0ee87f9cb566cef040364c80c1f09bbf86"),"CMSIS/core_cmFunc.h":("Code/Drivers/core_cmFunc.h","0a18fafc301e003d348edf5cae39481d8e5fe7c3"),"CMSIS/core_cmInstr.h":("Code/Drivers/core_cmInstr.h","d213f0eed7ca9335e883a5b55b6de14ba9507f1e"),"CMSIS/stm32f0xx.h":("Code/Drivers/stm32f0xx.h","3dfe2f51d205bc480b7062323eb35912b19ab5fe"),"CMSIS/system_stm32f0xx.h":("Code/Drivers/system_stm32f0xx.h","12027a8b223973ed8d957e7ee01be2a0a719200f"),
"StdPeriph/inc/stm32f0xx_flash.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_flash.h","236cdde64bf07090b0da4057a8efb8c585ddbd48"),"StdPeriph/inc/stm32f0xx_gpio.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_gpio.h","867e4d8fe5564127b3ea703e0f81d5211d27785f"),"StdPeriph/inc/stm32f0xx_iwdg.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_iwdg.h","1f2eece89e1bed1d7f916a9930444cfff7488d32"),"StdPeriph/inc/stm32f0xx_misc.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_misc.h","3811f0fd4eee7e47d53182f42b052422a04e10c0"),"StdPeriph/inc/stm32f0xx_rcc.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_rcc.h","380f98d1766f24b7389dba6333dc315452385f2a"),"StdPeriph/inc/stm32f0xx_syscfg.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_syscfg.h","8c9641443a2dd5097d25837cb45a37782beb81a2"),"StdPeriph/inc/stm32f0xx_usart.h":("Code/STM32F0xx_StdPeriph_Driver/inc/stm32f0xx_usart.h","2e732524a37aea5120bfc086e136ece776029f78"),
"StdPeriph/src/stm32f0xx_flash.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_flash.c","54a8833822f41c7ecabde557f999f2004635899b"),"StdPeriph/src/stm32f0xx_gpio.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_gpio.c","5b6eb2347dae97dba355a0f46d6ac5ea7eec35da"),"StdPeriph/src/stm32f0xx_iwdg.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_iwdg.c","b864cb5b9fbae4bc8c6b3fd92b04aa79db04a456"),"StdPeriph/src/stm32f0xx_misc.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_misc.c","d44d7fe04f234e2a9e09e82ae35f6a22fcf9fc48"),"StdPeriph/src/stm32f0xx_rcc.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_rcc.c","5b8af2c6049ec649c714e08c896490915fb26bcc"),"StdPeriph/src/stm32f0xx_syscfg.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_syscfg.c","5c7cdd3d319ec0e90cbe1913124819d8b4038260"),"StdPeriph/src/stm32f0xx_usart.c":("Code/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_usart.c","46f7b74ae1747ebf7a990dcfebd5450b6988f6ef")}
F1_BASE="C030v1.0/Project/STM32F10x_StdPeriph_Lib_V3.5.0/"
F1={
"CMSIS/core_cm3.h":(F1_BASE+"drivers/core_cm3.h","7ab7b4b43685d3ab2facb53d326c48eeb2bdfda1"),"CMSIS/stm32f10x.h":(F1_BASE+"drivers/stm32f10x.h","18ea2a7e47f5cb3251cee0b277eadc39ce5bb07e"),"CMSIS/system_stm32f10x.h":(F1_BASE+"drivers/system_stm32f10x.h","54bc1abe722b2a82fe0fd9fb1b7acb937597a2c2"),
"StdPeriph/inc/misc.h":(F1_BASE+"inc/misc.h","9a6bd07c17cbe6a24cd23b31c99554f58bcfa5dc"),"StdPeriph/inc/stm32f10x_flash.h":(F1_BASE+"inc/stm32f10x_flash.h","63720dea3446d5e5ef9b078866c79a5e80d99352"),"StdPeriph/inc/stm32f10x_gpio.h":(F1_BASE+"inc/stm32f10x_gpio.h","dd28da89c0e1e03c843c2dd73a4c5816c280dd72"),"StdPeriph/inc/stm32f10x_iwdg.h":(F1_BASE+"inc/stm32f10x_iwdg.h","25b0bb5206b3ed65327c5df08413a895e620b0ea"),"StdPeriph/inc/stm32f10x_rcc.h":(F1_BASE+"inc/stm32f10x_rcc.h","1149c34777ac24b415bf9cefbf80d11d9f0a7a7a"),"StdPeriph/inc/stm32f10x_usart.h":(F1_BASE+"inc/stm32f10x_usart.h","162fa87ccebd3d9261a5841c982712294c9e4908"),
"StdPeriph/src/misc.c":(F1_BASE+"src/misc.c","c0a5e11333d7043a2cb490975a9301c27ecf524a"),"StdPeriph/src/stm32f10x_flash.c":(F1_BASE+"src/stm32f10x_flash.c","cdff9e9b84614df645f1ba683bd3effea83448da"),"StdPeriph/src/stm32f10x_gpio.c":(F1_BASE+"src/stm32f10x_gpio.c","d392c6434ba23f2c60fb715bb817cc24dbd072df"),"StdPeriph/src/stm32f10x_iwdg.c":(F1_BASE+"src/stm32f10x_iwdg.c","c7cbf7ec5ce4a45558c0f2d87084122b5689e54b"),"StdPeriph/src/stm32f10x_rcc.c":(F1_BASE+"src/stm32f10x_rcc.c","a29034bdf72b6b69e204c2841f9ce8ba760113bd"),"StdPeriph/src/stm32f10x_usart.c":(F1_BASE+"src/stm32f10x_usart.c","8bddd0b7afd88bf00a4156ba9c4fc17cabc41baf")}

def main()->int:
 p=argparse.ArgumentParser();p.add_argument('--family',choices=['f0','f1','all'],default='all');a=p.parse_args()
 if a.family in ('f0','all'):
  rc=sync('CS19970929/030-309-template','3cc2db3f263bb466ed2292dfbc75d4a7f09c223c',ROOT/'vendor/st/stm32f0_stdperiph_v1.5.0',F0)
  if rc:return rc
 if a.family in ('f1','all'):
  rc=sync('CS19970929/103-309-template','51e13fb69216b7ff3f3218539dbb41e644ef9575',ROOT/'vendor/st/stm32f10x_stdperiph_v3.5.0',F1)
  if rc:return rc
 return 0
if __name__=='__main__':raise SystemExit(main())
