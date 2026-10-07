#!/usr/bin/env python3
"""Inventory the .scenic scenarios reachable from this environment.

Reads each file's header statically — no Scenic import, no server — so it works
before anything is installed and cannot be broken by a bad model import.

  python3 list_scenic.py                 every scenario, its model and map
  python3 list_scenic.py --check-maps    also load each road network and report
                                         the features scenarios select on
  python3 list_scenic.py --map Town05    only scenarios targeting that carla_map

Sources: $SCENARIO_RUNNER_ROOT/srunner/scenic and $SCENIC_EXAMPLES.
"""
from __future__ import annotations
import argparse, os, re, sys
from pathlib import Path

MODEL_RE = re.compile(r"^\s*model\s+([\w.]+)", re.M)
# `param map = localPath('...')` and `param carla_map = '...'`
PMAP_RE = re.compile(r"""^\s*param\s+map\s*=\s*localPath\(\s*['"]([^'"]+)['"]""", re.M)
CMAP_RE = re.compile(r"""^\s*param\s+carla_map\s*=\s*['"]([^'"]+)['"]""", re.M)
# hardcoded blueprint constants, e.g. EGO_MODEL = "vehicle.lincoln.mkz"
BP_RE = re.compile(r"""['"]((?:vehicle|walker\.pedestrian|static\.prop)\.[\w.\-]+)['"]""")


def sources() -> list[tuple[str, Path]]:
    out = []
    sr = os.environ.get("SCENARIO_RUNNER_ROOT", "")
    if sr and (Path(sr) / "srunner/scenic").is_dir():
        out.append(("srunner", Path(sr) / "srunner/scenic"))
    ex = os.environ.get("SCENIC_EXAMPLES", "")
    if ex and Path(ex).is_dir():
        out.append(("scenic", Path(ex)))
    return out


def scan(root: Path) -> list[dict]:
    rows = []
    for f in sorted(root.rglob("*.scenic")):
        # models are libraries, not runnable scenarios
        if f.parent.name in {"models", "model"} or f.name in {"model.scenic", "behaviors.scenic"}:
            continue
        text = f.read_text(errors="replace")
        m, pm, cm = MODEL_RE.search(text), PMAP_RE.search(text), CMAP_RE.search(text)
        rows.append({
            "path": f, "name": f.name,
            "model": m.group(1) if m else "(none)",
            "xodr": Path(pm.group(1)).name if pm else "",
            "carla_map": cm.group(1) if cm else "",
            "blueprints": sorted(set(BP_RE.findall(text))),
        })
    return rows


def network_features(xodr: Path) -> str:
    """Intersection counts scenarios filter on. Import is local so --check-maps is
    the only mode that needs Scenic installed."""
    import warnings
    warnings.filterwarnings("ignore")
    from scenic.domains.driving.roads import Network
    n = Network.fromFile(str(xodr), useCache=True)
    i = n.intersections
    f4u = sum(1 for x in i if x.is4Way and not x.isSignalized)
    f3u = sum(1 for x in i if x.is3Way and not x.isSignalized)
    return "lanes=%d ints=%d 4way=%d(uns %d) 3way=%d(uns %d) signalized=%d" % (
        len(n.lanes), len(i), sum(1 for x in i if x.is4Way), f4u,
        sum(1 for x in i if x.is3Way), f3u, sum(1 for x in i if x.isSignalized))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check-maps", action="store_true")
    ap.add_argument("--map", default=None)
    a = ap.parse_args()

    srcs = sources()
    if not srcs:
        print("no scenario sources: set SCENARIO_RUNNER_ROOT and/or SCENIC_EXAMPLES", file=sys.stderr)
        return 1

    seen_xodr: dict[Path, str] = {}
    for label, root in srcs:
        rows = [r for r in scan(root) if not a.map or r["carla_map"] == a.map]
        print("\n== %s (%d scenarios) %s" % (label, len(rows), root))
        print("   %-34s %-30s %-16s %s" % ("file", "model", "carla_map", "hardcoded blueprints"))
        for r in rows:
            print("   %-34s %-30s %-16s %s" % (
                r["name"], r["model"].replace("scenic.simulators.carla", "scenic..carla")
                                     .replace("srunner.scenic.models", "srunner..models"),
                r["carla_map"] or "-", ",".join(b.split(".")[-1] for b in r["blueprints"]) or "-"))
            if a.check_maps and r["xodr"]:
                p = (r["path"].parent / Path(next(iter(PMAP_RE.findall(r["path"].read_text(errors="replace"))), ""))).resolve()
                if p.is_file() and p not in seen_xodr:
                    try:
                        seen_xodr[p] = network_features(p)
                    except Exception as e:
                        seen_xodr[p] = "network load failed: %s" % str(e)[:70]

    if seen_xodr:
        print("\n== road networks ==")
        for p, feat in sorted(seen_xodr.items()):
            print("   %-18s %s" % (p.name, feat))
        print("\n   A scenario filtering for a feature with count 0 fails at sample time with")
        print("   'discrete distribution over empty domain' — that is the map, not the scenario.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
