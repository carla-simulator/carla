#!/usr/bin/env python3

import argparse
import filecmp
import json
import shutil
from pathlib import Path


def merge_json_value(base, overlay):
    if isinstance(base, dict) and isinstance(overlay, dict):
        merged = dict(base)
        for key, value in overlay.items():
            merged[key] = merge_json_value(base[key], value) if key in base else value
        return merged

    if isinstance(base, list) and isinstance(overlay, list):
        return base + overlay

    return overlay


def read_json(path):
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def merge_json_file(base_file, overlay_file):
    base_json = read_json(base_file)
    overlay_json = read_json(overlay_file)
    merged_json = merge_json_value(base_json, overlay_json)

    if merged_json == base_json:
        return "unchanged"

    with base_file.open("w", encoding="utf-8") as output:
        json.dump(merged_json, output, indent=4, ensure_ascii=False)
        output.write("\n")
    return "json_merged"


def merge_overlay(base_dir, overlay_dir):
    counters = {
        "copied": 0,
        "overwritten": 0,
        "json_merged": 0,
        "unchanged": 0,
    }

    for overlay_file in sorted(path for path in overlay_dir.rglob("*") if path.is_file()):
        relative_path = overlay_file.relative_to(overlay_dir)
        base_file = base_dir / relative_path
        base_file.parent.mkdir(parents=True, exist_ok=True)

        if not base_file.exists():
            shutil.copy2(overlay_file, base_file)
            counters["copied"] += 1
            continue

        if base_file.suffix.lower() == ".json":
            counters[merge_json_file(base_file, overlay_file)] += 1
            continue

        if filecmp.cmp(overlay_file, base_file, shallow=False):
            counters["unchanged"] += 1
            continue

        shutil.copy2(overlay_file, base_file)
        counters["overwritten"] += 1

    return counters


def main():
    parser = argparse.ArgumentParser(description="Merge a content overlay into a base content tree.")
    parser.add_argument("--base", required=True, type=Path, help="Base content directory")
    parser.add_argument("--overlay", required=True, type=Path, help="Overlay content directory")
    args = parser.parse_args()

    if not args.base.is_dir():
        raise SystemExit(f"Base content directory not found: {args.base}")
    if not args.overlay.is_dir():
        raise SystemExit(f"Overlay content directory not found: {args.overlay}")

    counters = merge_overlay(args.base, args.overlay)
    print(
        "Content overlay merge complete: "
        f"{counters['copied']} copied, "
        f"{counters['overwritten']} overwritten, "
        f"{counters['json_merged']} JSON merged, "
        f"{counters['unchanged']} unchanged."
    )


if __name__ == "__main__":
    main()
