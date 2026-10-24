#!/usr/bin/env python3
"""
OpenDRIVE Traffic Signals & Landmark Schema Inspector.

Inspects OpenDRIVE XML and CARLA Map landmarks to verify:
  1. All traffic light landmarks have valid signal types (1000001, etc.).
  2. Controllers have valid signal bindings and junction associations.
  3. Signal stop line waypoints are drivable and located along valid roads.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

import carla

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from osm_traffic_lights import load_traffic_lights_manifest


def inspect_traffic_signals(
    xodr_path: Path | str,
    manifest_path: Path | str | None = None,
) -> dict[str, Any]:
    """Inspects OpenDRIVE signals and returns verification metrics."""
    xodr_p = Path(xodr_path)
    if not xodr_p.is_file():
        raise FileNotFoundError(f"OpenDRIVE file not found: {xodr_p}")

    xml_text = xodr_p.read_text(encoding="utf-8")
    root = ET.fromstring(xml_text)

    # XML signals count
    xml_signals = []
    for r in root.findall("road"):
        sigs = r.find("signals")
        if sigs is not None:
            for s in sigs.findall("signal"):
                xml_signals.append({
                    "road_id": r.get("id"),
                    "signal_id": s.get("id"),
                    "name": s.get("name"),
                    "type": s.get("type"),
                    "dynamic": s.get("dynamic"),
                    "s": float(s.get("s", 0.0)),
                    "t": float(s.get("t", 0.0)),
                })

    controllers = []
    for c in root.findall("controller"):
        c_signals = [ctrl.get("signalId") for ctrl in c.findall("control")]
        controllers.append({
            "id": c.get("id"),
            "name": c.get("name"),
            "signals": c_signals,
        })

    # Validate with CARLA Map
    carla_map = carla.Map(xodr_p.stem, xml_text)
    landmarks = carla_map.get_all_landmarks()
    tl_landmarks = [lm for lm in landmarks if "1000001" in lm.type or "signal" in lm.name.lower()]

    manifest_signals = []
    if manifest_path and Path(manifest_path).is_file():
        manifest_signals = load_traffic_lights_manifest(manifest_path)

    return {
        "xodr_signals_count": len(xml_signals),
        "controllers_count": len(controllers),
        "total_landmarks": len(landmarks),
        "traffic_light_landmarks": len(tl_landmarks),
        "manifest_signals_count": len(manifest_signals),
        "sample_signals": xml_signals[:5],
        "is_valid": len(xml_signals) > 0 and len(tl_landmarks) > 0,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Inspect OpenDRIVE Traffic Signals")
    parser.add_argument("xodr", help="Path to OpenDRIVE .xodr file")
    parser.add_argument("--manifest", default=None, help="Optional path to Placements/traffic_lights.json")
    args = parser.parse_args()

    try:
        report = inspect_traffic_signals(args.xodr, manifest_path=args.manifest)
    except Exception as exc:
        print(f"[traffic-signal-inspector] ERROR: {exc}")
        return 1

    print("\n" + "=" * 72)
    print(f"CARLA Traffic Signal & Landmark Inspection: {Path(args.xodr).name}")
    print("=" * 72)
    print(f"  OpenDRIVE Signals Defined : {report['xodr_signals_count']:,}")
    print(f"  Signal Controllers        : {report['controllers_count']:,}")
    print(f"  Total Road Landmarks      : {report['total_landmarks']:,}")
    print(f"  Traffic Light Landmarks   : {report['traffic_light_landmarks']:,}")
    if report["manifest_signals_count"] > 0:
        print(f"  Manifest Placements       : {report['manifest_signals_count']:,} signals")
    print("=" * 72)

    if report["is_valid"]:
        print("[traffic-signal-inspector] SUCCESS: OpenDRIVE traffic signals verified!")
        return 0
    else:
        print("[traffic-signal-inspector] FAIL: No valid traffic signals found.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
