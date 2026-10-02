#! /usr/bin/env python3
"""Python API, end to end: capture a fresh clip from a local CARLA server, then generate.

    python api_capture_to_job.py --port 2000 --frames 93 --fps 16 --vehicles 20 \\
        --backend transfer2.5 [--rig nvidia_av7] [--results ./cosmos-results]

The Cosmos node is checked *before* CARLA is touched, so an unreachable node
costs nothing.  Traffic spawning and the rig come from ``single_view_live.py``
(the same code paths the CLI demos use); the clip is captured in sync mode,
validated, then submitted and stored.
"""

from __future__ import annotations

import argparse
import sys
import time

from api_common import connect, results_root, setup_logging, wait_and_store  # puts ../client on sys.path

from carla_cosmos import BUILTIN_CONTRACTS  # noqa: E402  (contracts only: no carla, no OpenCV)
from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402


def capture(args, log):
    """Spawn traffic, capture ``--frames`` frames, restore the world settings."""
    import carla

    from carla_cosmos import Capture
    from single_view_live import load_rig, spawn_traffic  # reuse the capture code paths of the CLI demos

    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)
    world = client.get_world()
    tm = client.get_trafficmanager(args.tm_port)
    tm.set_random_device_seed(args.seed)
    settings, actors = world.get_settings(), []
    try:
        s = world.get_settings()
        s.synchronous_mode, s.fixed_delta_seconds = True, 1.0 / args.fps
        world.apply_settings(s)
        tm.set_synchronous_mode(True)
        hero, actors = spawn_traffic(world, tm, args.vehicles, args.walkers, args.seed)
        for _ in range(2 * args.fps):  # let the traffic get moving
            world.tick()
        clip_id = args.clip_id or f"api_{world.get_map().name.split('/')[-1].lower()}_{int(time.time())}"
        cap = Capture(world, hero, load_rig(args.rig), BUILTIN_CONTRACTS[args.backend],
                      frames=args.frames, fps=args.fps)
        clip = cap.run(args.out, clip_id, seed=args.seed, carla_version=client.get_server_version(),
                       progress=lambda i, n: log.info("frame %d/%d", i, n) if i % args.fps == 0 or i == n else None)
    finally:
        tm.set_synchronous_mode(False)
        world.apply_settings(settings)
        for a in actors:
            if a.type_id.startswith("controller."):
                a.stop()
        client.apply_batch([carla.command.DestroyActor(a) for a in actors])
    problems = clip.validate()
    if problems:
        raise SystemExit("clip validation failed:\n  " + "\n  ".join(problems))
    return clip


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--tm-port", type=int, default=8000)
    ap.add_argument("--frames", type=int, default=93)
    ap.add_argument("--fps", type=int, default=16)
    ap.add_argument("--vehicles", type=int, default=20)
    ap.add_argument("--walkers", type=int, default=0)
    ap.add_argument("--rig", default="single", help="single | nvidia_av7 | path to a rig YAML")
    ap.add_argument("--backend", default="transfer2.5", choices=sorted(BUILTIN_CONTRACTS))
    ap.add_argument("--control", action="append", default=None, metavar="NAME=SOURCE[:WEIGHT]",
                    help="default: depth=clip seg=clip")
    ap.add_argument("--prompt", default="The same street on a foggy morning, photorealistic dashcam footage")
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--clip-id", default=None)
    ap.add_argument("--out", default="./clips", help="where the captured clip is written")
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--no-viewer-video", action="store_true",
                    help="do not render the result's side-by-side viewer to viewer_<layout>.mp4")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    log = setup_logging("api_capture_to_job", args.verbose)

    cosmos = connect(args.endpoint, args.token, backend=args.backend)  # fail fast, before CARLA
    log.info("node %s ready; results will be stored under %s", cosmos.url, results_root(args.results))
    clip = capture(args, log)
    log.info("captured %s (%d frames @ %d fps) -> %s", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, clip.path)

    from single_view_live import parse_controls

    controls = parse_controls(args.control, edge=False)
    try:
        job = cosmos.submit_clip(clip, args.backend, args.prompt, controls, seed=args.seed)
        log.info("job %s queued (position %s); controls %s", job.id, job.info.queue_position, list(controls))
        stored = wait_and_store(job, args.results, log, clip=clip,
                                viewer_video=False if args.no_viewer_video else None)
    except JobFailed as exc:
        log.error("the job did not finish: %s", exc)
        return 2
    except CosmosError as exc:
        log.error("%s", exc)
        return 2
    print(f"\nclip:   {clip.path}\nresult: {stored.directory}\nvideos: {stored.videos}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
