#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run(cmd, env=None):
    print('+', ' '.join(map(str, cmd)))
    result = subprocess.run(cmd, cwd=ROOT, env=env, check=False)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def require(name):
    if shutil.which(name) is None:
        raise SystemExit(f'missing required tool: {name}')


def cmake_cycle(preset):
    run(['cmake', '--preset', preset])
    run(['cmake', '--build', '--preset', preset])
    run(['ctest', '--preset', preset])


def main():
    require('cmake'); require('ninja')
    cmake_cycle('host-debug')
    cmake_cycle('host-o2')
    if os.name != 'nt':
        cmake_cycle('host-sanitize')
    if shutil.which('cppcheck'):
        run(['cppcheck','--enable=warning,style,performance,portability','--std=c11','--error-exitcode=1','--quiet',
             'common','protocol','bootloader/core','app/core','drivers/afe'])
    else:
        print('[WARN] cppcheck not installed; CI will enforce it')
    print('QUALITY_GATE_PASS')

if __name__ == '__main__':
    main()
