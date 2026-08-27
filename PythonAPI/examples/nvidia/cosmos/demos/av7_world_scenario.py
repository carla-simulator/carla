#! /usr/bin/env python3
"""7-camera world-scenario demo: capture with the NVIDIA AV rig, export the ClipGT scene
package, submit it to Transfer 2.5 auto/multiview (``hdmap_bbox``) and, optionally, the
front camera to Cosmos 3 as a ``wsm`` control.

    python av7_world_scenario.py --port 2000 --seconds 3 --out ./clips --results ./results \\
        [--also-cosmos3] [--capture-only]

Frame budget: Transfer 2.5 AV consumes 10 fps and needs 29 + 28*(k-1) frames per view,
so the 30 fps capture is 3*(29 + 28*(k-1)) frames (k = ``--seconds``); Cosmos 3 ``wsm``
needs 101*k frames at its fps, so ``--also-cosmos3`` captures a second, 10 fps clip.
Both scene packages are rendered on the server with NVIDIA's renderer, which produces
30 fps and decimates: a scene-rendered control needs a 10, 15 or 30 fps clip (never 16).
"""

from __future__ import annotations

import argparse
import logging
import random
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "client"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from carla_cosmos import BUILTIN_CONTRACTS, Clip, CosmosClient  # noqa: E402
from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402

log = logging.getLogger("av7_demo")


def capture_clip(args, backend: str, frames: int, fps: int, suffix: str) -> Clip:
    import carla

    from carla_cosmos import Capture, Rig
    from single_view_live import spawn_traffic

    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)
    world = client.get_world()
    tm = client.get_trafficmanager(args.tm_port)
    tm.set_random_device_seed(args.seed)
    settings = world.get_settings()
    actors = []
    try:
        s = world.get_settings()
        s.synchronous_mode = True
        s.fixed_delta_seconds = 1.0 / fps
        world.apply_settings(s)
        tm.set_synchronous_mode(True)
        hero, actors = spawn_traffic(world, tm, args.vehicles, args.walkers, args.seed)
        for _ in range(2 * fps):
            world.tick()
        clip_id = f"{args.clip_id or 'av7_' + world.get_map().name.split('/')[-1].lower()}_{suffix}"
        cap = Capture(world, hero, Rig.nvidia_av7(), BUILTIN_CONTRACTS[backend], frames=frames, fps=fps,
                      aovs=("rgb", "depth", "semantic", "instance"))
        t0 = time.time()
        clip = cap.run(args.out, clip_id, seed=args.seed, carla_version=client.get_server_version(),
                       progress=lambda i, n: log.info("frame %d/%d", i, n) if i % fps == 0 or i == n else None)
        log.info("captured %s (%d frames @ %d fps, 7 cameras) in %.0fs", clip_id, frames, fps, time.time() - t0)
    finally:
        tm.set_synchronous_mode(False)
        world.apply_settings(settings)
        for a in actors:
            try:
                if a.type_id.startswith("controller."):
                    a.stop()
            except RuntimeError:
                pass
        client.apply_batch([carla.command.DestroyActor(a) for a in actors])
    problems = clip.validate()
    if problems:
        raise SystemExit("clip validation failed:\n  " + "\n  ".join(problems))
    return clip


def submit(cosmos: CosmosClient, clip: Clip, backend: str, prompt: str, controls: dict, views, args, tag: str,
           rgb: bool | None = None):
    job = cosmos.submit_clip(clip, backend, prompt, controls, views=views, seed=args.seed, num_steps=args.steps,
                             guidance=args.guidance, negative_prompt=args.negative_prompt, rgb=rgb)
    log.info("[%s] job %s queued on %s (%d view(s))", tag, job.id, backend, len(job.info.views))
    return job


def wait_and_download(job, clip: Clip, args, tag: str) -> Path:
    last = [""]

    def progress(info):
        msg = f"{info.status} {info.progress * 100:5.1f}% {info.message}"
        if msg != last[0]:
            log.info("[%s]   %s", tag, msg)
            last[0] = msg

    job.wait(on_progress=progress, poll=3.0)
    res = job.result()
    dest = Path(args.results) / clip.manifest.clip_id / job.id
    paths = res.download(dest)
    log.info("[%s] done: %d file(s) in %s; timings %s", tag, len(paths), dest,
             {k: round(v, 1) for k, v in res.manifest.timings.items()})
    return dest


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--tm-port", type=int, default=8000)
    ap.add_argument("--seconds", type=int, default=3, help="chunks k for Transfer 2.5 AV (29+28(k-1) frames @10 fps)")
    ap.add_argument("--vehicles", type=int, default=30)
    ap.add_argument("--walkers", type=int, default=10)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--clip-id", default=None)
    ap.add_argument("--out", default="./clips")
    ap.add_argument("--results", default="./results")
    ap.add_argument("--prompt", default="An urban street in the late afternoon, clear sky, photorealistic footage from "
                                        "a car-mounted camera rig")
    ap.add_argument("--negative-prompt", default=None)
    ap.add_argument("--steps", type=int, default=None)
    ap.add_argument("--guidance", type=float, default=None)
    ap.add_argument("--views", default="all", help="'all' (7) or comma-separated camera names")
    ap.add_argument("--with-rgb", action="store_true", help="also send RGB (num_conditional_frames=1)")
    ap.add_argument("--also-cosmos3", action="store_true", help="capture a 10 fps clip and run Cosmos 3 wsm too")
    ap.add_argument("--cosmos3-backend", default="cosmos3-nano")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("--capture-only", action="store_true")
    ap.add_argument("--no-view", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO,
                        format="%(asctime)s %(levelname)-7s %(name)s: %(message)s")
    random.seed(args.seed)

    av_frames = 3 * (29 + 28 * (args.seconds - 1))
    av_clip = capture_clip(args, "transfer2.5-av", av_frames, 30, "30fps")
    c3_clip = capture_clip(args, args.cosmos3_backend, 101 * args.seconds, 10, "10fps") if args.also_cosmos3 else None
    if args.capture_only:
        return 0

    try:
        cosmos = CosmosClient(args.endpoint, token=args.token)
        models = cosmos.models()
        if not models["transfer2.5-av"].scene_rendering:
            log.error("server cannot render scene packages (no wsm renderer in its profile)")
            return 2
        views = None if args.views == "all" else [v.strip() for v in args.views.split(",")]
        jobs = [("av", av_clip, submit(cosmos, av_clip, "transfer2.5-av", args.prompt,
                                       {"hdmap_bbox": "scene"}, views, args, "av", rgb=args.with_rgb or None))]
        if c3_clip is not None:
            jobs.append(("c3", c3_clip, submit(cosmos, c3_clip, args.cosmos3_backend, args.prompt, {"wsm": "scene"},
                                               ["camera:front:wide:120fov"], args, "c3")))
        results = []
        for tag, clip, job in jobs:
            results.append((clip, wait_and_download(job, clip, args, tag)))
    except (JobFailed, CosmosError) as exc:
        log.error("%s", exc)
        return 1
    if not args.no_view:
        from viewer import view_result

        for clip, dest in results:
            view_result(clip, dest)
    return 0


if __name__ == "__main__":
    sys.exit(main())
