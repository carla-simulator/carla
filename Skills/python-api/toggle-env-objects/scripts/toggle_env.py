#!/usr/bin/env python3
"""List and toggle a map's environment objects (buildings, vegetation, poles...).

Commands:

    labels                                       list valid CityObjectLabel names
    list    [--label Buildings] [--name PATTERN] count/sample matching objects
    disable --label Buildings [--name P] [--limit N] [--dry-run]   hide them
    enable  --label Buildings [--name P] [--limit N]               show them again

Environment objects are the static, map-baked assets (not spawned actors).
`get_environment_objects(label)` returns them (id, name, transform, bbox, type);
`enable_environment_objects({ids}, enable)` shows/hides a set — affecting both
rendering and collision. Toggling is per-world and resets on a map reload.

Natural language maps to a CityObjectLabel (see references/env-objects.md):
"buildings" -> Buildings, "trees" -> Vegetation, "street poles" -> Poles,
"traffic signs" -> TrafficSigns, "fences" -> Fences, "guard rails" -> GuardRail.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os
from collections import Counter

import carla  # provided by the active interpreter; check_env.sh verifies this


def _world():
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client.get_world()


def _label(name: str) -> "carla.CityObjectLabel":
    valid = {n.lower(): getattr(carla.CityObjectLabel, n)
             for n in dir(carla.CityObjectLabel) if n[:1].isupper()}
    key = name.lower()
    if key not in valid:
        raise SystemExit(f"unknown label {name!r}; run `labels` for the list")
    return valid[key]


def _select(world, args):
    """Environment objects for a label, optionally name-substring filtered."""
    label = _label(args.label) if args.label else carla.CityObjectLabel.Any
    objs = world.get_environment_objects(label)
    if getattr(args, "name", None):
        needle = args.name.lower()
        objs = [o for o in objs if needle in o.name.lower()]
    if getattr(args, "limit", None):
        objs = objs[:args.limit]
    return objs


def cmd_labels(_):
    names = sorted(n for n in dir(carla.CityObjectLabel) if n[:1].isupper())
    print("CityObjectLabel values:")
    print("  " + ", ".join(names))


def cmd_list(args):
    world = _world()
    objs = _select(world, args)
    scope = args.label or "Any"
    print(f"{len(objs)} environment object(s) for label={scope}"
          + (f" name~{args.name!r}" if args.name else "") + ":")
    if not args.label:  # summarise the whole map by type
        by_type = Counter(str(o.type).split(".")[-1] for o in objs)
        for t, n in by_type.most_common():
            print(f"  {t:16s} {n}")
    else:
        for o in objs[:20]:
            l = o.transform.location
            print(f"  id={o.id:6d}  {o.name:40s}  ({l.x:.0f},{l.y:.0f},{l.z:.0f})")
        if len(objs) > 20:
            print(f"  ... and {len(objs)-20} more")


def _toggle(args, enable: bool):
    world = _world()
    if not args.label:
        raise SystemExit("need --label (run `labels`)")
    objs = _select(world, args)
    ids = {o.id for o in objs}
    verb = "enable" if enable else "disable"
    if not ids:
        print(f"no objects matched label={args.label}"
              + (f" name~{args.name!r}" if args.name else "") + " — nothing to " + verb)
        return
    if getattr(args, "dry_run", False):
        print(f"[dry-run] would {verb} {len(ids)} object(s) of label={args.label}")
        for o in objs[:10]:
            print(f"    {o.name}")
        return
    world.enable_environment_objects(ids, enable)
    print(f"{verb}d {len(ids)} object(s) of label={args.label}"
          + (f" name~{args.name!r}" if args.name else ""))
    print("  (applies to rendering + collision; view on a rendered server; resets on map reload)")


def cmd_disable(args): _toggle(args, False)
def cmd_enable(args):  _toggle(args, True)


def main() -> None:
    p = argparse.ArgumentParser(description="List/toggle CARLA environment objects.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("labels", help="list CityObjectLabel names").set_defaults(func=cmd_labels)

    pl = sub.add_parser("list", help="count/sample environment objects")
    pl.add_argument("--label", help="CityObjectLabel (default: whole-map summary)")
    pl.add_argument("--name", help="substring filter on object name")
    pl.add_argument("--limit", type=int)
    pl.set_defaults(func=cmd_list)

    pd = sub.add_parser("disable", help="hide matching objects")
    pd.add_argument("--label", required=True)
    pd.add_argument("--name"); pd.add_argument("--limit", type=int)
    pd.add_argument("--dry-run", action="store_true")
    pd.set_defaults(func=cmd_disable)

    pe = sub.add_parser("enable", help="show matching objects again")
    pe.add_argument("--label", required=True)
    pe.add_argument("--name"); pe.add_argument("--limit", type=int)
    pe.set_defaults(func=cmd_enable)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
