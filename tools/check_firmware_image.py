#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--target", required=True)
    args = parser.parse_args()

    build_dir = Path(args.build_dir)
    summary_path = build_dir / "generated" / args.target / "target-summary.json"
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    flash = summary["resolved"]["flash"]

    images = (
        ("bootloader", build_dir / "bms_bootloader.bin", int(flash["boot_size"])),
        ("app", build_dir / "bms_app.bin", int(flash["app_size"])),
    )
    failed = False
    for name, path, budget in images:
        if not path.is_file():
            raise SystemExit(f"missing firmware image: {path}")
        actual = path.stat().st_size
        remaining = budget - actual
        percent = (actual * 100.0) / budget
        print(f"{name}: {actual}/{budget} bytes ({percent:.1f}%), remaining={remaining}")
        if actual > budget:
            failed = True
    if failed:
        print("FIRMWARE_IMAGE_BUDGET_FAIL")
        return 2
    print("FIRMWARE_IMAGE_BUDGET_PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
