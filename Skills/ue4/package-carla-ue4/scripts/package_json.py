#!/usr/bin/env python3
"""Author and validate a <Name>.Package.json for a CARLA standalone asset package.

PrepareAssetsForCooking locates the file by RECURSIVE SEARCH under
Unreal/CarlaUE4/Content, so the package name is just the filename minus
".Package.json" — no matching directory is required. That is what lets stock
maps be exported as a package without moving any content.

Schema mirrors Util/BuildTools/Import.py. Both "maps" and "props" keys are
mandatory even when empty: the commandlet uses GetArrayField, which fails hard
on a missing key.

Examples
--------
  package_json.py OneTown --map Town02 --carla-materials
  package_json.py MyProps --prop /Game/MyPkg/Static/Bench/SM_Bench.SM_Bench
  package_json.py OneTown --check          # validate an existing package
  package_json.py OneTown --map Town02 --dry-run
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path


def carla_root() -> Path:
    """Resolve the CARLA checkout to package, mirroring scripts/env.sh precedence.

    carla-agentic-tools is a STANDALONE repo, so the target checkout is chosen at
    runtime, not derived from this file's location:

      1. $CARLA_UE4_ROOT if set (the explicit, canonical choice)
      2. $PWD if it looks like a CARLA checkout
      3. the path-derived guess (only meaningful when carla-agentic-tools was
         dropped INTO a checkout, e.g. .../<checkout>/carla-agentic-tools/skills/
         package-carla-ue4/scripts/ -> four up)
    """
    env_root = os.environ.get("CARLA_UE4_ROOT")
    if env_root:
        return Path(env_root).resolve()
    cwd = Path.cwd()
    if (cwd / "Util" / "BuildTools" / "Package.sh").is_file():
        return cwd
    return Path(__file__).resolve().parents[4]


def content_dir() -> Path:
    return carla_root() / "Unreal" / "CarlaUE4" / "Content"


def _walk_find(root: Path, filename: str) -> list[Path]:
    """Recursive search that FOLLOWS SYMLINKS.

    Content/Carla is commonly a symlink to a shared content checkout. Neither
    pathlib.rglob nor plain `find` descends into it, so both silently return
    nothing. os.walk(followlinks=True) does.
    """
    hits: list[Path] = []
    for dirpath, _dirnames, filenames in os.walk(root, followlinks=True):
        if filename in filenames:
            hits.append(Path(dirpath) / filename)
    return sorted(hits)


def find_existing(content: Path, package: str) -> Path | None:
    """Same lookup the commandlet does: first recursive match wins."""
    return next(iter(_walk_find(content, f"{package}.Package.json")), None)


def find_xodr(content: Path, map_name: str) -> Path | None:
    """Package.sh locates the OpenDRIVE file the same way (but without -L)."""
    return next(iter(_walk_find(content, f"{map_name}.xodr")), None)


def check(content: Path, package: str) -> int:
    """Validate an existing package definition. Returns an exit code."""
    path = find_existing(content, package)
    if path is None:
        print(f"FAIL  no {package}.Package.json under {content}")
        return 1

    print(f"PASS  {path}")
    try:
        doc = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        print(f"FAIL  invalid JSON: {exc}")
        return 1

    rc = 0
    for key in ("maps", "props"):
        if key not in doc:
            # GetArrayField fails hard, so a missing key breaks the cook.
            print(f"FAIL  missing required key '{key}' (may be an empty list)")
            rc = 1
        elif not isinstance(doc[key], list):
            print(f"FAIL  '{key}' must be a list")
            rc = 1

    for m in doc.get("maps", []):
        name = m.get("name", "?")
        for field in ("name", "path", "use_carla_materials"):
            if field not in m:
                print(f"FAIL  map '{name}' missing '{field}'")
                rc = 1
        # A map without OpenDRIVE packages cleanly but has no road network.
        if "name" in m:
            xodr = find_xodr(content, m["name"])
            if xodr:
                print(f"PASS  map {name}: {xodr.relative_to(content)}")
            else:
                print(f"WARN  map {name}: no {name}.xodr under Content/ — "
                      f"the map will import without a road network")

    for p in doc.get("props", []):
        if "path" not in p:
            print(f"FAIL  prop entry missing 'path': {p}")
            rc = 1

    if rc == 0:
        print(f"OK    {package}: {len(doc.get('maps', []))} map(s), "
              f"{len(doc.get('props', []))} prop(s)")
    return rc


def build(package: str, maps: list[str], props: list[str], carla_materials: bool,
          map_root: str) -> dict:
    return {
        # Import.py also records "size"; the cooking commandlet reads only "path".
        "props": [{"name": p.split("/")[-1].split(".")[0], "path": p, "size": ""}
                  for p in props],
        "maps": [{"name": m,
                  "path": f"{map_root.rstrip('/')}/{m}",
                  "use_carla_materials": carla_materials}
                 for m in maps],
    }


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("package", help="package name; becomes <name>.Package.json")
    ap.add_argument("--map", action="append", default=[], metavar="NAME",
                    help="map name (without .umap); repeatable")
    ap.add_argument("--prop", action="append", default=[], metavar="/Game/...",
                    help="full prop asset path; repeatable")
    ap.add_argument("--map-root", default="/Game/Carla/Maps",
                    help="content path holding the maps (default: %(default)s)")
    ap.add_argument("--carla-materials", action="store_true",
                    help="apply CARLA road materials to the maps")
    ap.add_argument("--out", metavar="DIR",
                    help="directory for the JSON (default: Content/Carla/Config)")
    ap.add_argument("--check", action="store_true",
                    help="validate the existing package instead of writing")
    ap.add_argument("--force", action="store_true", help="overwrite an existing file")
    ap.add_argument("--dry-run", action="store_true", help="print, do not write")
    args = ap.parse_args()

    content = content_dir()
    if not content.is_dir():
        print(f"ERROR: no Content dir at {content}", file=sys.stderr)
        return 1

    if args.check:
        return check(content, args.package)

    if args.package == "Carla":
        print("ERROR: 'Carla' is the base release package; pick another name.",
              file=sys.stderr)
        return 1
    if not args.map and not args.prop:
        ap.error("nothing to package: pass at least one --map or --prop")

    doc = build(args.package, args.map, args.prop, args.carla_materials, args.map_root)
    text = json.dumps(doc, indent=4)

    out_dir = Path(args.out) if args.out else content / "Carla" / "Config"
    out = out_dir / f"{args.package}.Package.json"

    if args.dry_run:
        print(f"# would write {out}")
        print(text)
        return 0

    existing = find_existing(content, args.package)
    if existing and not args.force:
        print(f"ERROR: {existing} already defines this package; "
              f"pass --force to overwrite.", file=sys.stderr)
        return 1

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text + "\n", encoding="utf-8")
    print(f"wrote {out}")

    # Surface missing OpenDRIVE now rather than after an hour of cooking.
    for m in args.map:
        if find_xodr(content, m) is None:
            print(f"WARNING: no {m}.xodr under Content/ — the packaged map will "
                  f"have no road network.", file=sys.stderr)

    print(f"next: PACKAGES={args.package} bash scripts/package.sh")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
