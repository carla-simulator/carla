#!/usr/bin/env python3
"""Read and set a running CARLA server's weather, then verify.

Commands:

    show                         print the current WeatherParameters
    list-presets                 list CARLA's built-in named presets
    preset ClearSunset           apply a named preset exactly
    set --base HardRainSunset --fog-density 40   preset as a base, then override
    set --cloudiness 80 --precipitation 60 ...   override fields on the current weather

Natural-language requests ("heavy rain at sunset", "light fog at night") are
turned into parameters two ways, both exact-valued:
  1. If the request is a standard condition x time-of-day, it IS a preset —
     apply it with `preset` (see references/weather.md for the full matrix).
  2. Otherwise start from the nearest preset with `set --base <preset>` and
     override the differing fields using the vocabulary table in the reference.

Every change reads the weather back (set_weather returns nothing), so the caller
can confirm the numbers match the intent.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os
import sys

import carla  # provided by the active interpreter; check_env.sh verifies this

# All 14 WeatherParameters fields. The 0-100 ones are clamped on set; angles and
# scattering scales are passed through (sun_altitude is -90..90, azimuth 0..360).
FIELDS = [
    "cloudiness", "precipitation", "precipitation_deposits", "wind_intensity",
    "sun_azimuth_angle", "sun_altitude_angle", "fog_density", "fog_distance",
    "fog_falloff", "wetness", "scattering_intensity", "mie_scattering_scale",
    "rayleigh_scattering_scale", "dust_storm",
]
PERCENT_FIELDS = {
    "cloudiness", "precipitation", "precipitation_deposits", "wind_intensity",
    "fog_density", "wetness", "dust_storm",
}


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


def _client() -> carla.Client:
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def _presets() -> "dict[str, str]":
    """Lowercase name -> real attribute name, for every built-in preset."""
    return {n.lower(): n for n in dir(carla.WeatherParameters)
            if isinstance(getattr(carla.WeatherParameters, n), carla.WeatherParameters)}


def _settle(world: carla.World) -> None:
    """Advance one frame so a just-applied set_weather is readable.

    set_weather takes effect on the next server tick, so get_weather() called
    immediately after returns the pre-change state. Advancing one frame first
    makes the read-back reflect the change: tick it ourselves in sync mode, or
    wait for the server's next tick in async mode (bounded so it can't hang).
    """
    try:
        if world.get_settings().synchronous_mode:
            world.tick()
        else:
            world.wait_for_tick(seconds=5.0)
    except RuntimeError:
        pass  # no tick within the bound — report what we can read anyway


def _report(world: carla.World, note: str) -> None:
    w = world.get_weather()
    print(f"\nVERIFY {note}")
    for f in FIELDS:
        print(f"  {f:26s}= {getattr(w, f)}")


def cmd_show(_: argparse.Namespace) -> None:
    _report(_client().get_world(), "(current weather)")


def cmd_list_presets(_: argparse.Namespace) -> None:
    names = sorted(_presets().values())
    print(f"{len(names)} presets:")
    for n in names:
        print(f"  {n}")


def cmd_preset(args: argparse.Namespace) -> None:
    presets = _presets()
    key = args.name.lower()
    if key not in presets:
        sys.exit(f"unknown preset {args.name!r}; run list-presets")
    world = _fresh(_client().get_world())
    world.set_weather(getattr(carla.WeatherParameters, presets[key]))
    _settle(world)
    _report(world, f"(preset {presets[key]})")


def cmd_set(args: argparse.Namespace) -> None:
    world = _fresh(_client().get_world())
    # Base to override: a named preset if --base given, else the live weather so
    # unspecified fields are left exactly as they are.
    if args.base:
        presets = _presets()
        if args.base.lower() not in presets:
            sys.exit(f"unknown preset {args.base!r}; run list-presets")
        w = getattr(carla.WeatherParameters, presets[args.base.lower()])
        base_note = f"base {presets[args.base.lower()]}"
    else:
        w = world.get_weather()
        base_note = "base current"

    changed = []
    for f in FIELDS:
        val = getattr(args, f)
        if val is None:
            continue
        if f in PERCENT_FIELDS and not (0.0 <= val <= 100.0):
            val = max(0.0, min(100.0, val))
            print(f"note: {f} clamped to {val} (valid range 0-100)")
        setattr(w, f, val)
        changed.append(f)
    if not args.base and not changed:
        sys.exit("set needs --base and/or at least one field flag (see --help)")

    world.set_weather(w)
    _settle(world)
    _report(world, f"({base_note}; set {', '.join(changed) or 'nothing'})")


def main() -> None:
    p = argparse.ArgumentParser(description="Read/set CARLA weather and verify.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("show", help="print current weather").set_defaults(func=cmd_show)
    sub.add_parser("list-presets", help="list built-in presets").set_defaults(func=cmd_list_presets)

    pp = sub.add_parser("preset", help="apply a named preset exactly")
    pp.add_argument("name", help="preset name, e.g. HardRainSunset (case-insensitive)")
    pp.set_defaults(func=cmd_preset)

    pt = sub.add_parser("set", help="override fields, optionally on top of a preset")
    pt.add_argument("--base", help="preset to start from (default: current weather)")
    for f in FIELDS:
        pt.add_argument(f"--{f.replace('_', '-')}", dest=f, type=float,
                        help=f"{f}" + (" (0-100)" if f in PERCENT_FIELDS else ""))
    pt.set_defaults(func=cmd_set)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
