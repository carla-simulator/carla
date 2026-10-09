#!/usr/bin/env python3
"""What blueprints may a generated scenario name?

Two different answers, and a scenario is only safe where they agree:

  build inventory   what the CARLA build can actually spawn. Read from
                    $CARLA_ROOT/.../Content/Carla/Config/*Parameters.json - the
                    definitive per-build list, no server needed.
  Scenic's table    what Scenic will hand out for a *category* (`new Bicycle`),
                    from scenic/simulators/carla/_blueprintData.py, keyed on the
                    installed CLIENT version.

An id in the build but not in Scenic's table is still usable — name it explicitly
with `with blueprint "..."`. An id in Scenic's table but not in the build spawns
nothing. An empty Scenic category makes `new <Type>` fail at sample time.

  python3 blueprint_table.py                 summary + gaps
  python3 blueprint_table.py --category car  ids for one Scenic category
  python3 blueprint_table.py --list vehicles build inventory of one kind
  python3 blueprint_table.py --check ID ...  is each id spawnable in this build
"""
from __future__ import annotations
import argparse, glob, json, os, sys
from pathlib import Path


def config_dir() -> Path:
    """Content/Carla/Config, from CARLA_ROOT or the usual sibling layouts."""
    cands = []
    for var in ("CARLA_ROOT", "CARLA_UE5_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT"):
        v = os.environ.get(var)
        if v:
            cands += [Path(v) / "Unreal/CarlaUnreal/Content/Carla/Config",
                      Path(v) / "Unreal/CarlaUE4/Content/Carla/Config",
                      Path(v) / "CarlaUnreal/Content/Carla/Config"]
    cands.append(Path.cwd() / "Unreal/CarlaUnreal/Content/Carla/Config")
    for c in cands:
        if (c / "VehicleParameters.json").is_file():
            return c
    raise SystemExit(
        "cannot find Content/Carla/Config — set CARLA_ROOT to a CARLA checkout.\n"
        "Looked in:\n  " + "\n  ".join(str(c) for c in cands))


def inventory(cfg: Path) -> dict[str, set[str]]:
    """id -> lowercase blueprint ids, per kind. Mirrors how the C++ factories
    build ids: vehicle.<Make>.<Model>, walker.pedestrian.<Id>, static.prop.<Name>."""
    j = lambda f, k: json.load(open(cfg / f))[k]
    veh = {f"vehicle.{e['Make']}.{e['Model']}".lower() for e in j("VehicleParameters.json", "Vehicles")}
    wal = {f"walker.pedestrian.{e['Id']}".lower() for e in j("WalkerParameters.json", "Walkers")}
    pro = {f"static.prop.{e['Name']}".lower() for e in j("PropParameters.json", "Props")}
    return {"vehicles": veh, "walkers": wal, "props": pro}


def scenic_table() -> tuple[str, dict[str, list[str]]]:
    from importlib.metadata import version
    try:
        cv = version("carla")
    except Exception:
        import carla
        cv = getattr(carla, "__version__", "unknown")
    from scenic.simulators.carla import _blueprintData as bd
    if cv not in bd._IDS:
        raise SystemExit(f"Scenic has no blueprint table for client {cv} "
                         f"(has: {', '.join(sorted(bd._IDS))})")
    return cv, bd._IDS[cv]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--category")
    ap.add_argument("--list", choices=["vehicles", "walkers", "props"])
    ap.add_argument("--check", nargs="+")
    a = ap.parse_args()

    cfg = config_dir()
    inv = inventory(cfg)
    real = set().union(*inv.values())

    if a.check:
        rc = 0
        for i in a.check:
            hit = i.lower() in real
            print("  %-6s %s" % ("OK" if hit else "ABSENT", i))
            rc |= 0 if hit else 1
        return rc

    if a.list:
        for i in sorted(inv[a.list]):
            print(" ", i)
        return 0

    cv, tab = scenic_table()
    if a.category:
        key = a.category if a.category.endswith("Models") else a.category + "Models"
        if key not in tab:
            print("no such category. Available:\n  " + ", ".join(sorted(tab)))
            return 1
        for i in tab[key]:
            mark = "" if i.lower() in real else "   <- NOT IN THIS BUILD"
            print(" ", i, mark)
        return 0

    print("build inventory (%s)" % cfg)
    for k, v in inv.items():
        print("  %-9s %d" % (k, len(v)))
    print("\nScenic table for client %s: %d ids, %d categories" %
          (cv, sum(len(v) for v in tab.values()), len(tab)))

    declared = {x.lower() for c in tab.values() for x in c}
    ghosts = sorted(declared - real)
    empty = sorted(k for k, v in tab.items() if not v)
    print("\n== Scenic offers, build cannot spawn (%d) ==" % len(ghosts))
    for g in ghosts:
        print("   ", g)
    print("\n== Scenic categories that are EMPTY (%d) ==" % len(empty))
    print("   " + (", ".join(empty) or "-"))
    if empty:
        print("   `new <Type>` for these fails at sample time. Name an id explicitly instead.")
    print("\n== build has, Scenic's categories omit ==")
    for k, v in inv.items():
        miss = sorted(v - declared)
        print("   %-9s %3d  %s" % (k, len(miss), ", ".join(miss)[:150] or "-"))
    print("\n   Omitted ids are spawnable — reference them with `with blueprint \"<id>\"`.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
