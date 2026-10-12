#!/usr/bin/env python3
"""List the scenarios a scenario_runner checkout can actually run.

Parses srunner/examples/*.xml directly, which is what `scenario_runner.py --list`
reads — but without importing carla or building a client, so it answers with no
server running and no PythonAPI on the path.

    list_scenarios.py                 every config: name, type, town
    list_scenarios.py --town Town04   only configs for one map
    list_scenarios.py --here          only configs for the *running* server's map
    list_scenarios.py --types         group by type; flag route-only classes
    list_scenarios.py --check         report configs whose class or town is broken
"""
from __future__ import annotations

import argparse
import os
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# Maps that exist in a stock CARLA install, by engine. Used only to flag configs
# pointing at a town that cannot resolve — e.g. master's HighwayCutIn.xml, which
# says town="Highway".
UE4_MAPS = {
    "Town01", "Town02", "Town03", "Town04", "Town05", "Town06", "Town07",
    "Town10HD", "Town10HD_Opt", "Town11", "Town12", "Town13", "Town15",
    "Town01_Opt", "Town02_Opt", "Town03_Opt", "Town04_Opt", "Town05_Opt",
    "Town06_Opt", "Town07_Opt",
}
UE5_MAPS = {"Town10HD_Opt"}


def sr_root(explicit: str | None) -> Path:
    for c in [explicit, os.environ.get("SCENARIO_RUNNER_ROOT"), os.getcwd(),
              str(Path.home() / "scenario_runner")]:
        if c and (Path(c) / "srunner" / "examples").is_dir():
            return Path(c).resolve()
    sys.exit("no scenario_runner checkout found — export SCENARIO_RUNNER_ROOT "
             "or run the install-scenario-runner skill")


def branch(root: Path) -> str:
    import subprocess

    p = subprocess.run(["git", "-C", str(root), "rev-parse", "--abbrev-ref", "HEAD"],
                       capture_output=True, text=True)
    return p.stdout.strip() or "unknown"


def configs(root: Path) -> list[dict]:
    out = []
    for f in sorted((root / "srunner" / "examples").glob("*.xml")):
        try:
            tree = ET.parse(f)
        except ET.ParseError as e:
            print(f"# WARNING {f.name} is not valid XML: {e}", file=sys.stderr)
            continue
        for s in tree.getroot().iter("scenario"):
            out.append({
                "file": f.name,
                "name": s.attrib.get("name", "?"),
                "type": s.attrib.get("type", "?"),
                "town": s.attrib.get("town", "?"),
                "egos": len(s.findall("ego_vehicle")),
            })
    return out


def classes(root: Path) -> set[str]:
    found = set()
    for f in (root / "srunner" / "scenarios").glob("*.py"):
        found |= set(re.findall(r"^class (\w+)\(", f.read_text(errors="replace"), re.M))
    return found


def server_map(host: str, port: int) -> str | None:
    try:
        import carla

        c = carla.Client(host, port)
        c.set_timeout(4.0)
        return c.get_world().get_map().name.split("/")[-1]
    except Exception:
        return None


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root")
    ap.add_argument("--town", help="filter to this map")
    ap.add_argument("--here", action="store_true", help="filter to the running server's map")
    ap.add_argument("--types", action="store_true", help="group by scenario type")
    ap.add_argument("--check", action="store_true", help="report broken configs")
    ap.add_argument("--host", default=os.environ.get("CARLA_HOST", "127.0.0.1"))
    ap.add_argument("--port", type=int, default=int(os.environ.get("CARLA_PORT", 2000)))
    args = ap.parse_args()

    root = sr_root(args.root)
    br = branch(root)
    cfg = configs(root)
    cls = classes(root)
    valid_maps = UE5_MAPS if br == "ue5-master" else UE4_MAPS

    town = args.town
    if args.here:
        town = server_map(args.host, args.port)
        if town is None:
            sys.exit(f"--here: no server at {args.host}:{args.port}")
        print(f"# server map: {town}")
    print(f"# {root}  (branch {br})")

    if args.check:
        problems = 0
        for c in cfg:
            if c["type"] not in cls:
                print(f"BROKEN  {c['name']:38} type {c['type']!r} has no class in srunner/scenarios/"
                      f"  [{c['file']}]")
                problems += 1
            if c["town"] not in valid_maps:
                print(f"BROKEN  {c['name']:38} town {c['town']!r} is not a CARLA map  [{c['file']}]")
                problems += 1
        # Route-only classes are not a problem, but knowing they exist explains
        # why `--scenario <name>` cannot reach most of the scenario library.
        infra = {"BasicScenario", "RouteScenario", "OpenScenario", "OSC2Scenario",
                 "BackgroundActivity", "BackgroundBehavior", "Source", "Junction",
                 "ClearBlackboardVariablesStartingWith", "StoryElementStatusToBlackboard"}
        route_only = sorted(cls - {c["type"] for c in cfg} - infra)
        print(f"\n{len(route_only)} classes are route-only (no standalone config; use --route):")
        for name in route_only:
            print(f"  {name}")
        print(f"\n{problems} broken config(s)")
        sys.exit(1 if problems else 0)

    if args.types:
        by: dict[str, list[dict]] = {}
        for c in cfg:
            by.setdefault(c["type"], []).append(c)
        for t in sorted(by):
            names = [c["name"] for c in by[t]]
            towns = sorted({c["town"] for c in by[t]})
            missing = "" if t in cls else "   <- NO CLASS: these configs cannot run"
            print(f"{t:38} {len(names):2} config(s)  towns: {','.join(towns)}{missing}")
            print(f"{'':38}   group:{t}")
        # A type present in the XML but with no class behind it is not runnable.
        # ScenarioRunner may still *find* a same-named object — an atomic behaviour
        # leaked into a scenario module's namespace — and fail constructing it.
        broken = sorted(t for t in by if t not in cls)
        print(f"\n{len(cfg)} configs, {len(by) - len(broken)} runnable types"
              + (f", {len(broken)} with no class: {', '.join(broken)}" if broken else ""))
        print("  --check lists the affected configs")
        return

    shown = [c for c in cfg if town is None or c["town"] == town]
    if not shown:
        print(f"# no configs for town {town!r} on branch {br}")
        if town and town not in valid_maps:
            print(f"# note: {town!r} is not a map this branch knows about")
        return
    print(f"{'CONFIG NAME':38} {'TYPE':34} {'TOWN':16} EGOS")
    for c in shown:
        print(f"{c['name']:38} {c['type']:34} {c['town']:16} {c['egos']}")
    print(f"\n{len(shown)} config(s)"
          + (f" for {town}" if town else f" across {len({c['town'] for c in shown})} towns"))
    print("run one with:  bash scripts/run_scenario.sh <CONFIG NAME>")


if __name__ == "__main__":
    main()
