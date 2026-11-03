#!/usr/bin/env python3
"""Load, reload, and reshape CARLA maps through the Python API, then verify.

One entry point for every map-loading operation this skill covers:

    list                              get_available_maps
    load    --map Town03             load_world (settings reset to default)
    load    --map Town03 --keep      load_world (current WorldSettings preserved)
    reload                           reload_world (settings reset to default)
    reload  --keep                   reload_world (current WorldSettings preserved)
    opendrive --xodr road.xodr       generate_opendrive_world from a .xodr file
    layer   --load Buildings,Foliage load map layers on the current world
    layer   --unload ParkedVehicles  unload map layers on the current world

Every mutating command re-reads the world afterwards and prints the resulting
map name + settings, because a load can silently land you on a different map or
reset the sync/rendering settings out from under a pipeline.

Connection comes from the environment (see env.sh): CARLA_HOST, CARLA_PORT,
CARLA_TIMEOUT. Layers only take effect on layered ("_Opt" / large) maps; on a
fully-baked map the server ignores layer ops (documented, not an error).
"""
from __future__ import annotations

import argparse
import os
import re
import sys

import carla  # provided by the active interpreter; check_env.sh verifies this


def _fresh(world):
    """A world handle that already holds a snapshot.

    In synchronous mode a freshly connected client has not seen a frame yet, so
    `get_actors()` comes back EMPTY and every --id/--filter lookup reports "no
    matching actor" while the scene is full of them. Observed against a live
    world holding 48 vehicles. One frame of waiting is the whole fix, and it
    must be a wait rather than a tick: another client owns that clock.
    """
    if world.get_settings().synchronous_mode:
        world.wait_for_tick()
    return world


def resolve_map(name: str, available: "list[str]") -> str:
    """Turn a friendly town name into the actual map the server hosts.

    CARLA's town naming is not uniform, so a bare "Town10" and "Town2" resolve
    differently (confirmed against the running server):
      - Town10 has no plain map that is used — its canonical map is the layered
        HD one, Town10HD_Opt (plain Town10HD exists but is effectively unused).
      - Every other numbered town defaults to the NON-opt map: Town2 -> Town02.
    An exact (or case-insensitive) match always wins, so an explicit "Town02_Opt"
    or "Town10HD" is honoured as typed. `available` is the stripped name list
    from get_available_maps.
    """
    if name in available:
        return name
    ci = {m.lower(): m for m in available}
    if name.lower() in ci:
        return ci[name.lower()]
    # bare number or Town<N>, optionally with HD/_Opt suffixes already stripped
    m = re.fullmatch(r"(?:town)?0*(\d+)", name.lower())
    if m:
        num = int(m.group(1))
        cand = "Town10HD_Opt" if num == 10 else f"Town{num:02d}"
        if cand in available:
            return cand
    sys.exit(f"map {name!r} not found; available: {', '.join(sorted(available))}")


def _client() -> carla.Client:
    host = os.environ.get("CARLA_HOST", "127.0.0.1")
    port = int(os.environ.get("CARLA_PORT", "2000"))
    # A map load (especially generate_opendrive_world, which builds geometry) can
    # take many seconds, so the working timeout is deliberately longer than the
    # 4s used for the liveness probe in check_env.sh.
    timeout = float(os.environ.get("CARLA_TIMEOUT", "60.0"))
    client = carla.Client(host, port)
    client.set_timeout(timeout)
    return client


def _parse_layers(spec: str) -> "carla.MapLayer":
    """Comma-separated MapLayer names -> a single OR-combined MapLayer.

    Accepts any of: All, NONE, Buildings, Decals, Foliage, Ground,
    ParkedVehicles, Particles, Props, StreetLights, Walls (case-insensitive).
    """
    valid = {n.lower(): getattr(carla.MapLayer, n)
             for n in ("All", "NONE", "Buildings", "Decals", "Foliage", "Ground",
                       "ParkedVehicles", "Particles", "Props", "StreetLights", "Walls")}
    layers = None
    for raw in spec.split(","):
        key = raw.strip().lower()
        if not key:
            continue
        if key not in valid:
            sys.exit(f"unknown map layer {raw!r}; valid: {', '.join(sorted(valid))}")
        layers = valid[key] if layers is None else layers | valid[key]
    if layers is None:
        sys.exit("no layers given")
    return layers


def _is_ue5_line(ver: str) -> bool:
    """True for the UE5 CARLA line: 0.10.x before the 1.0 release, 1.x after it.

    The line was renamed, not changed, so anything gated on it must accept both
    labels — a check for "0.10" alone goes silent against a 1.0 server.
    """
    if ver.startswith("0.10"):
        return True
    major = ver.split(".")[0]
    return major.isdigit() and int(major) >= 1


def _report(world: "carla.World", note: str = "") -> None:
    """Print the post-operation map + settings so the caller can verify."""
    s = world.get_settings()
    name = world.get_map().name
    print(f"\nVERIFY {note}".rstrip())
    print(f"  map               = {name}")
    print(f"  synchronous_mode  = {s.synchronous_mode}")
    print(f"  fixed_delta_seconds = {s.fixed_delta_seconds}")
    print(f"  no_rendering_mode = {s.no_rendering_mode}")


def cmd_list(_: argparse.Namespace) -> None:
    client = _client()
    maps = [m.replace("/Game/Carla/Maps/", "") for m in client.get_available_maps()]
    print(f"{len(maps)} maps available:")
    for m in sorted(maps):
        print(f"  {m}")


def cmd_load(args: argparse.Namespace) -> None:
    client = _client()
    layers = _parse_layers(args.layers) if args.layers else carla.MapLayer.All
    available = [m.replace("/Game/Carla/Maps/", "") for m in client.get_available_maps()]
    target = resolve_map(args.map, available)
    if target != args.map:
        print(f"resolved {args.map!r} -> {target!r}")
    # reset_settings is the inverse of --keep: default True resets the new world
    # to async/default; --keep passes False so sync mode, fixed_delta_seconds and
    # no_rendering_mode carry across the load.
    world = client.load_world(target, reset_settings=not args.keep, map_layers=layers)
    _report(world, f"(loaded {target}, keep_settings={args.keep})")
    if args.keep and world.get_settings().synchronous_mode:
        print("  note: synchronous_mode preserved — tick the world to advance it")


def cmd_reload(args: argparse.Namespace) -> None:
    client = _client()
    world = client.reload_world(reset_settings=not args.keep)
    _report(world, f"(reloaded, keep_settings={args.keep})")


def cmd_opendrive(args: argparse.Namespace) -> None:
    if not os.path.isfile(args.xodr):
        sys.exit(f"xodr file not found: {args.xodr}")
    with open(args.xodr, encoding="utf-8") as f:
        data = f.read()
    # Defaults match CARLA's stock util/config.py so behaviour is unsurprising;
    # all are overridable via flags. Units are metres.
    params = carla.OpendriveGenerationParameters(
        vertex_distance=args.vertex_distance,
        max_road_length=args.max_road_length,
        wall_height=args.wall_height,
        additional_width=args.additional_width,
        smooth_junctions=True,
        enable_mesh_visibility=True,
    )
    client = _client()
    world = client.generate_opendrive_world(data, params, reset_settings=not args.keep)
    _report(world, f"(opendrive {os.path.basename(args.xodr)}, keep_settings={args.keep})")


def cmd_osm(args: argparse.Namespace) -> None:
    if not os.path.isfile(args.osm):
        sys.exit(f"osm file not found: {args.osm}")
    with open(args.osm, encoding="utf-8") as f:
        osm_data = f.read()
    # Convert OpenStreetMap -> OpenDRIVE (default way-types), then build the world.
    # Same pipeline as util/osm_to_xodr.py + generate_opendrive_world.
    xodr = carla.Osm2Odr.convert(osm_data, carla.Osm2OdrSettings())
    params = carla.OpendriveGenerationParameters(
        vertex_distance=args.vertex_distance, max_road_length=args.max_road_length,
        wall_height=args.wall_height, additional_width=args.additional_width,
        smooth_junctions=True, enable_mesh_visibility=True,
    )
    client = _client()
    world = client.generate_opendrive_world(xodr, params, reset_settings=not args.keep)
    _report(world, f"(osm {os.path.basename(args.osm)}, keep_settings={args.keep})")


def cmd_layer(args: argparse.Namespace) -> None:
    if bool(args.load) == bool(args.unload):
        sys.exit("layer needs exactly one of --load or --unload")
    client = _client()
    world = _fresh(client.get_world())
    if args.load:
        world.load_map_layer(_parse_layers(args.load))
        action = f"loaded layers {args.load}"
    else:
        world.unload_map_layer(_parse_layers(args.unload))
        action = f"unloaded layers {args.unload}"
    _report(world, f"({action})")
    print("  note: layer ops are no-ops on fully-baked (non-'_Opt') maps")
    # On the UE5 line they are no-ops on EVERY map: the UE5 conversion flattened
    # the per-layer sublevels into the persistent level, so the mask matches
    # nothing in World->GetStreamingLevels(). The call still returns success.
    try:
        if _is_ue5_line(client.get_server_version()):
            print("  WARNING UE5 line: layer ops do nothing on ANY map — the layer "
                  "sublevels were baked into the persistent level.")
            print("          Hide geometry with enable_environment_objects "
                  "(toggle-env-objects) instead.")
    except Exception:
        pass


def main() -> None:
    p = argparse.ArgumentParser(description="Load/reshape CARLA maps and verify.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("list", help="list available maps").set_defaults(func=cmd_list)

    pl = sub.add_parser("load", help="load a named map")
    pl.add_argument("--map", required=True, help="map name, e.g. Town03 or Town10HD_Opt")
    pl.add_argument("--keep", action="store_true", help="preserve current WorldSettings (reset_settings=False)")
    pl.add_argument("--layers", help="comma-separated MapLayer names to load with (default All)")
    pl.set_defaults(func=cmd_load)

    pr = sub.add_parser("reload", help="reload the current map")
    pr.add_argument("--keep", action="store_true", help="preserve current WorldSettings (reset_settings=False)")
    pr.set_defaults(func=cmd_reload)

    po = sub.add_parser("opendrive", help="generate a world from a .xodr file")
    po.add_argument("--xodr", required=True, help="path to an OpenDRIVE .xodr file")
    po.add_argument("--keep", action="store_true", help="preserve current WorldSettings")
    po.add_argument("--vertex-distance", type=float, default=2.0)
    po.add_argument("--max-road-length", type=float, default=500.0)
    po.add_argument("--wall-height", type=float, default=1.0)
    po.add_argument("--additional-width", type=float, default=0.6)
    po.set_defaults(func=cmd_opendrive)

    pm = sub.add_parser("osm", help="generate a world from an OpenStreetMap .osm file")
    pm.add_argument("--osm", required=True, help="path to a .osm (OpenStreetMap) file")
    pm.add_argument("--keep", action="store_true", help="preserve current WorldSettings")
    pm.add_argument("--vertex-distance", type=float, default=2.0)
    pm.add_argument("--max-road-length", type=float, default=500.0)
    pm.add_argument("--wall-height", type=float, default=1.0)
    pm.add_argument("--additional-width", type=float, default=0.6)
    pm.set_defaults(func=cmd_osm)

    py = sub.add_parser("layer", help="load/unload map layers on the current world")
    py.add_argument("--load", help="comma-separated MapLayer names to load")
    py.add_argument("--unload", help="comma-separated MapLayer names to unload")
    py.set_defaults(func=cmd_layer)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
