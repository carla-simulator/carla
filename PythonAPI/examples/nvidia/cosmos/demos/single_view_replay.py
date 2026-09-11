#! /usr/bin/env python3
"""Deterministic clip from a CARLA recorder log, then a batch of prompts x seeds.

    python single_view_replay.py --port 2000 --log /abs/path/drive.log --start 5 --duration 3 \\
        --batch batch.yaml --out ./clips --results ./results

The recorder log is replayed in sync mode (``client.replay_file``), the hero is
found by ``role_name=hero``, the clip is captured frame-exactly and validated,
then one job per (prompt, seed) of ``batch.yaml`` is submitted with priority
``batch``; results are downloaded as they finish.  ``--capture-only`` stops after
the capture; ``--clip DIR`` skips the capture and reuses an existing clip.
"""

from __future__ import annotations

import argparse
import logging
import sys
import time
from pathlib import Path

import yaml

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "client"))
from carla_cosmos import BUILTIN_CONTRACTS, Clip, CosmosClient  # noqa: E402
from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402

log = logging.getLogger("replay_demo")


def capture(args) -> Clip:
    import carla

    from carla_cosmos import Capture, ReplayTicks, Rig

    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)
    world = client.get_world()
    contract = BUILTIN_CONTRACTS[args.backend] if args.backend else None
    rig = Rig.single() if args.rig == "single" else Rig.nvidia_av7() if args.rig == "nvidia_av7" else Rig.load(args.rig)
    settings = world.get_settings()
    try:
        s = world.get_settings()
        s.synchronous_mode = True
        s.fixed_delta_seconds = 1.0 / args.fps
        world.apply_settings(s)
        with ReplayTicks(client, str(Path(args.log).resolve()), start=args.start, duration=args.duration) as ticks:
            hero = ticks.find_ego()
            log.info("replaying %s from %.1fs; hero %d (%s)", args.log, args.start, hero.id, hero.type_id)
            clip_id = args.clip_id or f"replay_{Path(args.log).stem}_{int(args.start)}s"
            cap = Capture(world, hero, rig, contract, frames=args.frames, fps=args.fps, edge=args.edge, ticks=ticks)
            clip = cap.run(args.out, clip_id, seed=args.seed, carla_version=client.get_server_version())
    finally:
        world.apply_settings(settings)
    problems = clip.validate()
    if problems:
        raise SystemExit("clip validation failed:\n  " + "\n  ".join(problems))
    log.info("clip %s captured (%d frames @ %d fps) -> %s", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, clip.path)
    return clip


def run_batch(clip: Clip, spec: dict, args) -> int:
    cosmos = CosmosClient(args.endpoint, token=args.token)
    backend = spec.get("backend", args.backend or "cosmos3-nano")
    controls = spec.get("controls", {"depth": "clip", "seg": "clip"})
    contract = cosmos.contract(backend)
    jobs = []
    for name, prompt in spec["prompts"].items():
        for seed in spec.get("seeds", [0]):
            try:
                job = cosmos.submit_clip(clip, backend, prompt, controls, negative_prompt=spec.get("negative_prompt"),
                                         seed=int(seed), guidance=spec.get("guidance"), num_steps=spec.get("num_steps"),
                                         resolution=spec.get("resolution"), priority=spec.get("priority", "batch"),
                                         extra=spec.get("extra"), contract=contract)
            except CosmosError as exc:
                log.error("%s/seed %s: %s", name, seed, exc)
                return 2
            jobs.append((name, seed, job))
            log.info("queued %s seed=%s -> %s (position %s)", name, seed, job.id, job.info.queue_position)
    pending = dict((j.id, (n, s, j)) for n, s, j in jobs)
    failed = 0
    t0 = time.time()
    while pending:
        for jid, (name, seed, job) in list(pending.items()):
            info = job.refresh()
            if not info.terminal:
                continue
            del pending[jid]
            if info.status == "done":
                dest = Path(args.results) / clip.manifest.clip_id / f"{name}_seed{seed}_{jid}"
                paths = job.result().download(dest)
                log.info("done  %s seed=%s -> %s (%d files, %.0fs)", name, seed, dest, len(paths), time.time() - t0)
            else:
                failed += 1
                log.error("%s %s seed=%s: %s", info.status, name, seed, info.error)
        if pending:
            running = [f"{n}/{s}:{j.info.status} {j.info.progress * 100:.0f}%" for n, s, j in pending.values()
                       if j.info.status in ("preparing", "running")]
            log.info("%d job(s) pending%s", len(pending), f" — {', '.join(running)}" if running else "")
            time.sleep(args.poll)
    log.info("batch finished: %d ok, %d failed", len(jobs) - failed, failed)
    return 1 if failed else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--log", help="recorder log (absolute path on the CARLA server host)")
    ap.add_argument("--start", type=float, default=0.0)
    ap.add_argument("--duration", type=float, default=0.0, help="0 = to the end of the log")
    ap.add_argument("--frames", type=int, default=93)
    ap.add_argument("--fps", type=int, default=16)
    ap.add_argument("--rig", default="single")
    ap.add_argument("--edge", action="store_true")
    ap.add_argument("--backend", default=None, help="validate the capture against this contract")
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--clip-id", default=None)
    ap.add_argument("--clip", default=None, help="reuse this clip directory instead of capturing")
    ap.add_argument("--out", default="./clips")
    ap.add_argument("--results", default="./results")
    ap.add_argument("--batch", default=str(Path(__file__).with_name("batch.yaml")))
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("--poll", type=float, default=5.0)
    ap.add_argument("--capture-only", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO,
                        format="%(asctime)s %(levelname)-7s %(name)s: %(message)s")
    spec = yaml.safe_load(Path(args.batch).read_text())
    if args.backend is None:
        args.backend = spec.get("backend")
    if args.clip:
        clip = Clip.load(args.clip)
    elif args.log:
        clip = capture(args)
    else:
        ap.error("pass --log (capture from a recorder log) or --clip (existing clip)")
    if args.capture_only:
        return 0
    try:
        return run_batch(clip, spec, args)
    except JobFailed as exc:
        log.error("%s", exc)
        return 1


if __name__ == "__main__":
    sys.exit(main())
