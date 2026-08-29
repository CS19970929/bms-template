#!/usr/bin/env python3
from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

PRODUCTION_ROOTS = (
    "common",
    "protocol",
    "bootloader/core",
    "app/core",
    "drivers/afe",
    "platform/common",
    "platform/stm32f0",
    "platform/stm32f1",
)

SOURCE_SUFFIXES = {".c", ".h", ".s", ".S"}

FORBIDDEN_SOURCE_PATTERNS = {
    "dynamic allocation malloc": re.compile(r"\bmalloc\s*\("),
    "dynamic allocation calloc": re.compile(r"\bcalloc\s*\("),
    "dynamic allocation realloc": re.compile(r"\brealloc\s*\("),
    "dynamic allocation free": re.compile(r"\bfree\s*\("),
    "cppcheck suppression": re.compile(r"cppcheck-suppress"),
    "NOLINT suppression": re.compile(r"\bNOLINT\b"),
    "GCC warning suppression": re.compile(r"#\s*pragma\s+GCC\s+diagnostic\s+ignored"),
    "Clang warning suppression": re.compile(r"#\s*pragma\s+clang\s+diagnostic\s+ignored"),
}

FORBIDDEN_BUILD_FLAGS = ("-Ofast", "-ffast-math")
BUILD_POLICY_SUFFIXES = {".txt", ".cmake", ".json", ".yml", ".yaml", ".py"}


def run(cmd: list[str], env: dict[str, str] | None = None) -> None:
    print("+", " ".join(map(str, cmd)))
    result = subprocess.run(cmd, cwd=ROOT, env=env, check=False)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def run_capture(cmd: list[str]) -> bytes:
    print("+", " ".join(map(str, cmd)))
    result = subprocess.run(
        cmd,
        cwd=ROOT,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if result.stdout:
        print(result.stdout.decode("utf-8", errors="replace"), end="")
    if result.returncode != 0:
        raise SystemExit(result.returncode)
    return result.stdout


def require(name: str) -> None:
    if shutil.which(name) is None:
        raise SystemExit(f"missing required tool: {name}")


def cmake_cycle(preset: str) -> None:
    run(["cmake", "--fresh", "--preset", preset])
    run(["cmake", "--build", "--preset", preset])
    run(["ctest", "--preset", preset])


def host_test_binary(preset: str) -> Path:
    suffix = ".exe" if os.name == "nt" else ""
    return ROOT / "build" / preset / f"bms_host_tests{suffix}"


def validate_targets() -> None:
    targets = sorted((ROOT / "config/targets").glob("*.json"))
    if not targets:
        raise SystemExit("no target configurations found")
    output_root = ROOT / "build/config-check"
    for target in targets:
        run(
            [
                sys.executable,
                "tools/generate_target.py",
                "--target",
                target.stem,
                "--out",
                str(output_root / target.stem),
            ]
        )
    print(f"TARGET_CONFIG_GATE_PASS ({len(targets)} targets)")


def scan_source_policy() -> None:
    failures: list[str] = []
    for root_name in PRODUCTION_ROOTS:
        root = ROOT / root_name
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix not in SOURCE_SUFFIXES:
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for label, pattern in FORBIDDEN_SOURCE_PATTERNS.items():
                if pattern.search(text):
                    failures.append(f"{path.relative_to(ROOT)}: {label}")

    ignored_dirs = {".git", "build", "vendor"}
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix not in BUILD_POLICY_SUFFIXES:
            continue
        rel = path.relative_to(ROOT)
        if any(part in ignored_dirs for part in rel.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for flag in FORBIDDEN_BUILD_FLAGS:
            if flag in text and path.name != "check.py":
                failures.append(f"{rel}: forbidden build flag {flag}")

    if failures:
        print("POLICY_GATE_FAIL")
        for item in failures:
            print(" -", item)
        raise SystemExit(2)
    print("POLICY_GATE_PASS")


def run_cppcheck() -> None:
    if shutil.which("cppcheck") is None:
        print("[WARN] cppcheck not installed; CI will enforce it")
        return
    run(
        [
            "cppcheck",
            "--enable=warning,style,performance,portability",
            "--inconclusive",
            "--std=c11",
            "--error-exitcode=1",
            "--quiet",
            "common",
            "protocol",
            "bootloader/core",
            "app/core",
            "drivers/afe",
        ]
    )


def main() -> int:
    require("cmake")
    require("ninja")

    run([sys.executable, "tools/check_docs.py"])
    validate_targets()
    scan_source_policy()

    cmake_cycle("host-debug")
    debug_output = run_capture([str(host_test_binary("host-debug"))])

    cmake_cycle("host-o2")
    o2_output = run_capture([str(host_test_binary("host-o2"))])

    if debug_output != o2_output:
        print("O0_O2_EQUIVALENCE_FAIL")
        print("Debug/O0 and Release/O2 host-test output differs")
        return 3
    print("O0_O2_EQUIVALENCE_PASS")

    if os.name != "nt":
        cmake_cycle("host-sanitize")
        run_capture([str(host_test_binary("host-sanitize"))])

    run_cppcheck()
    print("QUALITY_GATE_PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
