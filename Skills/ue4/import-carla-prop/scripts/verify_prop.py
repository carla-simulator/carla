#!/usr/bin/env python3
"""Prove an imported prop is spawnable on a running CARLA server.

This is the acceptance test for import_prop.py, and it is deliberately the same
thing Docs/content_authoring_props.md tells a user to type:

    for bp in bp_lib.filter('*windmill*'):
        print(bp.id)                       # -> static.prop.windmill
    world.spawn_actor(bp_lib.find('static.prop.windmill'), spawn_loc)

Assets on disk prove nothing: the registry might not have picked the package up,
and a blueprint whose StaticMesh failed to load still *lists* while being
impossible to spawn. So each prop is filtered, found, spawned and destroyed.

    python3 verify_prop.py --name Windmill
    python3 verify_prop.py --package MyProps
    python3 verify_prop.py --name Windmill --keep      # leave it in the world

Blueprint ids are LOWERCASE
---------------------------
FillIdAndTags builds the id with `.ToLower()` (ActorBlueprintFunctionLibrary.cpp:207)
and BlueprintLibrary::Find is an exact map lookup with no case folding
(LibCarla/source/carla/actors/BlueprintLibrary.cpp:67). So a prop named
`Windmill` is `static.prop.windmill`, and looking it up under its original
casing fails on a perfectly good import. This script always lowercases.

Run with the CARLA client env active, against a server started by the
run-carla-server skill. The default -nullrhi mode is enough — spawning a prop
needs no rendering. Use a windowed server only if you want to look at it.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

try:
    import carla
except ImportError:
    sys.exit(
        "ERROR: cannot 'import carla'.\n"
        "       Activate the environment holding the CARLA wheel."
    )

# Props are spawned this far above a road spawn point: high enough that the road
# surface never blocks the spawn, low enough to stay inside the streamed area.
SPAWN_Z_OFFSET_M = 5.0


def carla_root() -> Path:
    """Resolve the target checkout: $CARLA_UE4_ROOT, then $PWD, then a path guess."""
    env = os.environ.get("CARLA_UE4_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    cwd = Path.cwd()
    if (cwd / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject").is_file():
        return cwd
    guess = Path(__file__).resolve().parents[4]
    if (guess / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject").is_file():
        return guess
    sys.exit(
        "ERROR: cannot locate a carla checkout to read the package from.\n"
        "       export CARLA_UE4_ROOT, or pass --name instead of --package."
    )


def names_from_package(package: str) -> list[str]:
    """Read the prop names a package registered.

    `--package Carla` means the stock content set, whose registry file is named
    Default.Package.json rather than Carla.Package.json.
    """
    config = carla_root() / "Unreal" / "CarlaUE4" / "Content" / package / "Config"
    path = config / ("Default.Package.json" if package == "Carla" else f"{package}.Package.json")
    if not path.is_file():
        sys.exit(f"ERROR: no {path} — was the prop imported? (import_prop.py)")
    props = json.loads(path.read_text()).get("props", [])
    if not props:
        sys.exit(f"ERROR: {path} declares no props.")
    return [p["name"] for p in props]


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--name", action="append", default=[], help="prop name to verify (repeatable)")
    parser.add_argument("--package", help="verify every prop a package registered ('Carla' = the stock set)")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--timeout", type=float, default=20.0, help="client timeout in seconds")
    parser.add_argument("--no-spawn", action="store_true", help="only check the blueprint library")
    parser.add_argument("--keep", action="store_true", help="leave the spawned prop in the world")
    args = parser.parse_args()

    names = list(args.name)
    if args.package:
        names += names_from_package(args.package)
    if not names:
        parser.error("pass --name or --package")

    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    try:
        world = client.get_world()
    except RuntimeError as exc:
        sys.exit(
            f"ERROR: no CARLA server at {args.host}:{args.port} ({exc}).\n"
            "       Start one with the run-carla-server skill, then re-run."
        )

    library = world.get_blueprint_library()
    carla_map = world.get_map()
    print(f"[verify] server {client.get_server_version()} on {args.host}:{args.port}, map {carla_map.name}")

    failures = 0
    for name in names:
        prop_id = f"static.prop.{name.lower()}"

        # Filter first, exactly as the docs do. Tags are lowercased at definition
        # time, so the pattern must be too.
        matches = [bp.id for bp in library.filter(f"*{name.lower()}*")]
        if prop_id not in matches:
            print(f"FAIL  {prop_id} is not in the blueprint library")
            if matches:
                print(f"      filter('*{name.lower()}*') found instead: {', '.join(matches)}")
            else:
                print("      nothing matched — the .Package.json entry is missing, or its")
                print("      'path' points at an asset that does not exist (references/props.md P3)")
            failures += 1
            continue

        blueprint = library.find(prop_id)
        size = blueprint.get_attribute("size").as_str() if blueprint.has_attribute("size") else "?"
        if size == "unknown":
            print(f"WARN  {prop_id} has size 'unknown' — the JSON 'size' did not match EPropSize")

        if args.no_spawn:
            print(f"PASS  {prop_id}  size={size}  (listed; spawn not attempted)")
            continue

        spawn_points = carla_map.get_spawn_points()
        if not spawn_points:
            print(f"WARN  {prop_id}  size={size}  (listed; map has no spawn points to test against)")
            continue
        transform = spawn_points[0]
        transform.location.z += SPAWN_Z_OFFSET_M

        actor = world.try_spawn_actor(blueprint, transform)
        if actor is None:
            print(f"FAIL  {prop_id} lists but will not spawn — its StaticMesh failed to load")
            print("      check the 'path' in the .Package.json resolves to a real .uasset")
            failures += 1
            continue
        if args.keep:
            loc = transform.location
            print(f"PASS  {prop_id}  size={size}  spawned at ({loc.x:.1f}, {loc.y:.1f}, {loc.z:.1f}) and kept")
        else:
            actor.destroy()
            print(f"PASS  {prop_id}  size={size}  spawned and destroyed")

    print()
    if failures:
        print(f"{failures} of {len(names)} prop(s) FAILED — the import did not register them")
        return 1
    print(f"all {len(names)} prop(s) verified spawnable")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
