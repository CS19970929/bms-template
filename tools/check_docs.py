#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

REQUIRED_DOCS = (
    "docs/README.md",
    "docs/ARCHITECTURE.md",
    "docs/APP_ARCHITECTURE.md",
    "docs/BOOTLOADER.md",
    "docs/FLASH_LAYOUT.md",
    "docs/PROTOCOL.md",
    "docs/PROTECTION.md",
    "docs/STATE_MACHINE.md",
    "docs/PARAMETERS.md",
    "docs/NVM.md",
    "docs/EVENT_LOG.md",
    "docs/SOC.md",
    "docs/BLE.md",
    "docs/UPPER_COMPUTER.md",
    "docs/CONFIGURATION.md",
    "docs/CLOUD_CI.md",
    "docs/HIL.md",
    "docs/RELEASE.md",
    "docs/engineering/CODING_STANDARD.md",
    "docs/engineering/TESTING.md",
    "docs/engineering/AI_DEVELOPMENT.md",
    "docs/engineering/DOCUMENTATION_POLICY.md",
    "docs/engineering/TRACEABILITY.md",
    "docs/adr/README.md",
)

RULES: tuple[tuple[str, tuple[str, ...]], ...] = (
    ("bootloader/", ("docs/BOOTLOADER.md",)),
    ("protocol/", ("docs/PROTOCOL.md",)),
    ("app/core/src/bms_protection", ("docs/PROTECTION.md",)),
    ("app/core/include/bms_protection", ("docs/PROTECTION.md",)),
    ("app/core/src/bms_mos", ("docs/PROTECTION.md",)),
    ("app/core/include/bms_mos", ("docs/PROTECTION.md",)),
    ("app/core/src/bms_state", ("docs/STATE_MACHINE.md",)),
    ("app/core/include/bms_state", ("docs/STATE_MACHINE.md",)),
    ("app/core/src/bms_parameter", ("docs/PARAMETERS.md",)),
    ("app/core/include/bms_parameter", ("docs/PARAMETERS.md",)),
    ("app/core/src/bms_nvm", ("docs/NVM.md",)),
    ("app/core/include/bms_nvm", ("docs/NVM.md",)),
    ("app/core/src/bms_event_log", ("docs/EVENT_LOG.md",)),
    ("app/core/include/bms_event_log", ("docs/EVENT_LOG.md",)),
    ("app/core/src/bms_soc", ("docs/SOC.md",)),
    ("app/core/include/bms_soc", ("docs/SOC.md",)),
    ("schema/parameters", ("docs/PARAMETERS.md",)),
    ("generated/parameters/", ("docs/PARAMETERS.md",)),
    ("schema/protections", ("docs/PROTECTION.md",)),
    ("generated/protections/", ("docs/PROTECTION.md",)),
    ("config/", ("docs/CONFIGURATION.md",)),
    ("tools/generate_target.py", ("docs/CONFIGURATION.md", "docs/FLASH_LAYOUT.md")),
    ("tools/generate_parameters.py", ("docs/PARAMETERS.md", "docs/engineering/DOCUMENTATION_POLICY.md")),
    ("tools/generate_protections.py", ("docs/PROTECTION.md", "docs/engineering/DOCUMENTATION_POLICY.md")),
    ("cmake/firmware.cmake", ("docs/FLASH_LAYOUT.md", "docs/CLOUD_CI.md")),
    ("platform/stm32f0/src/bms_nvm", ("docs/NVM.md", "docs/STM32F0_PORT.md")),
    ("platform/stm32f1/src/bms_nvm", ("docs/NVM.md", "docs/STM32F1_PORT.md")),
    ("platform/stm32f0/", ("docs/STM32F0_PORT.md",)),
    ("platform/stm32f1/", ("docs/STM32F1_PORT.md",)),
    ("pc/", ("docs/UPPER_COMPUTER.md",)),
    (".github/workflows/", ("docs/CLOUD_CI.md",)),
    ("tools/check.py", ("docs/CLOUD_CI.md", "docs/engineering/DOCUMENTATION_POLICY.md")),
    ("tools/check_docs.py", ("docs/CLOUD_CI.md", "docs/engineering/DOCUMENTATION_POLICY.md")),
    ("AGENTS.md", ("docs/engineering/AI_DEVELOPMENT.md", "docs/engineering/DOCUMENTATION_POLICY.md")),
)

SPECIAL_RULES: tuple[tuple[tuple[str, ...], tuple[str, ...]], ...] = (
    (("startup", "vector"), ("docs/BOOTLOADER.md", "docs/FLASH_LAYOUT.md")),
    (("flash",), ("docs/FLASH_LAYOUT.md",)),
    (("ble",), ("docs/BLE.md",)),
    (("release",), ("docs/RELEASE.md",)),
)


def git_changed(base: str) -> set[str]:
    result = subprocess.run(
        ["git", "diff", "--name-only", f"{base}...HEAD"],
        cwd=ROOT,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        print(result.stderr, end="")
        raise SystemExit("documentation gate could not compute git diff")
    return {line.strip() for line in result.stdout.splitlines() if line.strip()}


def structure_gate() -> None:
    failures: list[str] = []
    for rel in REQUIRED_DOCS:
        path = ROOT / rel
        if not path.is_file():
            failures.append(f"missing required document: {rel}")
        elif path.stat().st_size < 250:
            failures.append(f"owner document too small to be useful: {rel}")

    index = (ROOT / "docs/README.md").read_text(encoding="utf-8")
    for rel in REQUIRED_DOCS[1:]:
        if Path(rel).name not in index and rel not in index:
            failures.append(f"docs/README.md does not index {rel}")

    if failures:
        print("DOCUMENTATION_STRUCTURE_GATE_FAIL")
        for failure in failures:
            print(" -", failure)
        raise SystemExit(2)
    print(f"DOCUMENTATION_STRUCTURE_GATE_PASS ({len(REQUIRED_DOCS)} required documents)")


def change_coupling_gate(base: str) -> None:
    changed = git_changed(base)
    changed_docs = {path for path in changed if path.startswith("docs/") or path == "AGENTS.md"}
    failures: list[str] = []

    for changed_path in sorted(changed):
        required: set[str] = set()
        lower = changed_path.lower()
        for prefix, docs in RULES:
            if changed_path.startswith(prefix):
                required.update(docs)
        for keywords, docs in SPECIAL_RULES:
            if all(keyword in lower for keyword in keywords):
                required.update(docs)
        for owner in sorted(required):
            if owner not in changed_docs:
                failures.append(f"{changed_path} changed without synchronized owner document {owner}")

    if failures:
        print("DOCUMENTATION_CHANGE_COUPLING_FAIL")
        for failure in failures:
            print(" -", failure)
        raise SystemExit(3)
    print(f"DOCUMENTATION_CHANGE_COUPLING_PASS ({len(changed)} changed files)")


def main() -> int:
    structure_gate()
    base = os.environ.get("BMS_DOCS_BASE_SHA", "").strip()
    if base:
        change_coupling_gate(base)
    else:
        print("DOCUMENTATION_CHANGE_COUPLING_SKIP (BMS_DOCS_BASE_SHA not set)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
