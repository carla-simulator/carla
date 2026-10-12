#!/usr/bin/env python3
"""Replay a recorded CARLA .log, control its speed, follow an actor, and stop.

Commands:

    play  --file run.log                     replay the whole log
    play  --file run.log --start 5 --duration 10 --follow 87
    play  --file run.log --time-factor 2.0   replay at 2x
    play  --file run.log --replay-sensors --replay-weather
    speed --factor 0.5                       change speed of the running replay
    stop  [--keep-actors]                    stop replaying

`play` re-creates the recorded scene on the server and prints the server's replay
summary. `--follow` moves the spectator with that actor id (0 = no follow).
`--start` seconds from the beginning (negative = from the end), `--duration` 0 =
to the end. `--replay-sensors` regenerates sensor data from re-attached sensors;
`--replay-weather` restores the recorded weather. `--map-override` replays onto a
different map (for OpenDRIVE-only logs).

Time factor is a replayer setting, so it is applied before `play` and can be
changed live with `speed`.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


def _client() -> carla.Client:
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def _stop_holders():
    """SIGINT any local spawn skill that is holding actors in the world.

    The spawn skills stay resident on purpose (their Traffic Manager and walker
    controllers die with the client), and each one cleans up and restores the
    clock in a `finally`. So the right way to end them is SIGINT, not a kill:
    they then remove their own actors and hand the world back asynchronous,
    which is what a replay wants -- the replayer runs SERVER-side, so nothing
    needs to own the clock during playback, and a client that does own it just
    paces the replay to its own tick rate.

    Matching is done on /proc argv, not on a command-line substring: a shell
    whose own command line merely *quotes* "vehicles.py spawn --hold" matches a
    substring search and gets signalled, which is a good way to kill the caller
    by accident (observed). argv[0] has to be a python interpreter and argv[1]
    the script itself.

    Local processes only: a holder on another machine cannot be signalled from
    here.
    """
    import signal
    scripts = ("vehicles.py", "walkers.py", "sensors.py")
    stopped = []
    for entry in os.listdir("/proc"):
        if not entry.isdigit():
            continue
        try:
            with open(f"/proc/{entry}/cmdline", "rb") as handle:
                argv = handle.read().split(b"\0")
        except OSError:
            continue
        argv = [a.decode("utf-8", "replace") for a in argv if a]
        if len(argv) < 3:
            continue
        if os.path.basename(argv[0]).split(".")[0] not in ("python", "python3"):
            continue
        if os.path.basename(argv[1]) not in scripts:
            continue
        if "spawn" not in argv[2:] or "--hold" not in argv:
            continue
        pid = int(entry)
        if pid == os.getpid():
            continue
        try:
            os.kill(pid, signal.SIGINT)
        except OSError:
            continue
        stopped.append((pid, os.path.basename(argv[1])))
        print(f"stopped holder pid {pid}: {os.path.basename(argv[1])} "
              "(SIGINT, so it cleans up its own actors)")
    # Let each one run its finally block: destroy its actors, restore the clock.
    waited = 0
    while any(_alive(pid) for pid, _ in stopped) and waited < 2000:
        waited += 1
    return stopped


def _alive(pid):
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False


def _release_clock_if_stalled(client):
    """Put the world back to asynchronous if nothing is ticking it.

    After the holders exit there is no clock owner, and a synchronous world
    with no ticker does not advance -- the replay would sit there doing
    nothing. The replayer is server-driven, so asynchronous is the correct
    mode for it.
    """
    world = client.get_world()
    settings = world.get_settings()
    if not settings.synchronous_mode:
        return
    settings.synchronous_mode = False
    settings.fixed_delta_seconds = None
    world.apply_settings(settings)
    print("world set back to asynchronous: the replayer is server-side and "
          "needs no client to tick it")


def _clear_scene(client):
    """Empty the world before replaying.

    A replay RECREATES every actor the log holds, so anything already in the
    world is duplicated rather than replaced: replaying a 50-car log into a
    live 50-car scene gives 100 vehicles driving through each other, which is
    what made the first replay here unreadable. Clearing first means what you
    see is the recording and nothing else.

    Traffic signs and the spectator stay -- they belong to the map, not to the
    scene.
    """
    world = client.get_world()
    world.wait_for_tick()
    doomed = [a for a in world.get_actors()
              if a.type_id.startswith(("vehicle.", "walker.", "controller.",
                                       "sensor."))]
    if not doomed:
        return 0
    # Controllers before their walkers, or the walkers leave ghost actors.
    doomed.sort(key=lambda a: 0 if a.type_id.startswith("controller.") else 1)
    for a in doomed:
        if a.type_id.startswith("controller."):
            try:
                a.stop()
            except RuntimeError:
                pass
    try:
        responses = client.apply_batch_sync(
            [carla.command.DestroyActor(a.id) for a in doomed], False)
        gone = sum(1 for r in responses if not r.error)
    except RuntimeError:
        gone = 0
        for a in doomed:
            try:
                if a.destroy():
                    gone += 1
            except RuntimeError:
                pass
    print(f"cleared the scene: removed {gone} actor(s) before replaying "
          "(--keep-scene to replay on top of what is there)")
    return gone


def cmd_play(args: argparse.Namespace) -> None:
    client = _client()
    if not args.keep_scene:
        _stop_holders()
        _release_clock_if_stalled(client)
        _clear_scene(client)
    if args.ignore_hero:
        client.set_replayer_ignore_hero(True)
    if args.ignore_spectator:
        client.set_replayer_ignore_spectator(True)
    client.set_replayer_time_factor(args.time_factor)  # applied before replay starts
    ret = client.replay_file(
        args.file, args.start, args.duration, args.follow,
        args.replay_sensors, args.replay_weather,
        carla.Transform(), args.map_override,
    )
    print(f"replaying {args.file}  start={args.start}s duration="
          f"{'all' if args.duration == 0 else str(args.duration)+'s'} "
          f"follow={args.follow} time_factor={args.time_factor}")
    if ret:
        print(ret)  # server's summary (or an error string if the file is bad)


def cmd_speed(args: argparse.Namespace) -> None:
    _client().set_replayer_time_factor(args.factor)
    print(f"replay time_factor set to {args.factor} "
          f"({'faster' if args.factor > 1 else 'slower' if args.factor < 1 else 'real-time'})")


def cmd_stop(args: argparse.Namespace) -> None:
    _client().stop_replayer(args.keep_actors)
    print(f"replay stopped (keep_actors={args.keep_actors})")


def main() -> None:
    p = argparse.ArgumentParser(description="Replay a CARLA recording.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pp = sub.add_parser("play", help="replay a .log")
    pp.add_argument("--file", required=True, help="server-side .log path")
    pp.add_argument("--start", type=float, default=0.0, help="start time s (negative = from end)")
    pp.add_argument("--duration", type=float, default=0.0, help="seconds to replay (0 = all)")
    pp.add_argument("--follow", type=int, default=0, help="actor id for the spectator to follow (0 = none)")
    pp.add_argument("--time-factor", type=float, default=1.0, help="playback speed (1.0 = real time)")
    pp.add_argument("--replay-sensors", action="store_true", help="regenerate sensor data")
    pp.add_argument("--replay-weather", action="store_true", help="restore recorded weather")
    pp.add_argument("--map-override", default="", help="replay onto a different map")
    pp.add_argument("--ignore-hero", action="store_true", help="do not replay the hero actor")
    pp.add_argument("--ignore-spectator", action="store_true", help="do not move the spectator")
    pp.add_argument("--keep-scene", action="store_true",
                    help="do not clear the world first. The replay recreates "
                         "the log's actors, so anything already there is "
                         "duplicated — 50 live cars plus a 50-car log is 100 "
                         "vehicles sharing the same road")
    pp.set_defaults(func=cmd_play)

    psp = sub.add_parser("speed", help="change the running replay's speed")
    psp.add_argument("--factor", type=float, required=True, help="time factor (2.0 = 2x, 0.5 = half)")
    psp.set_defaults(func=cmd_speed)

    pst = sub.add_parser("stop", help="stop replaying")
    pst.add_argument("--keep-actors", action="store_true", help="leave replayed actors in the world")
    pst.set_defaults(func=cmd_stop)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
