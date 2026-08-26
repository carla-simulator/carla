"""``carla-cosmos`` command line.

    carla-cosmos health|status|models
    carla-cosmos submit --clip DIR --backend cosmos3-nano --prompt "..." --control depth=clip --control edge=derive [--wait --out DIR]
    carla-cosmos jobs [--status running] [--mine]
    carla-cosmos watch JOB [--out DIR]
    carla-cosmos result JOB --out DIR
    carla-cosmos cancel JOB
    carla-cosmos serve [--image IMG] [--port 8000] [--state DIR] [--gpus all] [--profile P] | --stop | --mock
    carla-cosmos synthetic-clip --out DIR [--frames 93 --fps 16 | --av7]

Connection: ``--url``/``--token`` or ``COSMOS_URL`` / ``COSMOS_TOKEN`` / ``COSMOS_TOKEN_FILE``.
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from pathlib import Path

from .client import CosmosClient, CosmosError, JobFailed
from .clip import Clip
from .contracts import JobInfo


def _client(args) -> CosmosClient:
    token = args.token
    if args.token_file:
        token = Path(args.token_file).read_text().strip()
    return CosmosClient(args.url, token=token)


def _print_progress(info: JobInfo) -> None:
    bar = "#" * int(info.progress * 30)
    print(f"\r  {info.id} {info.status:<10} [{bar:<30}] {info.progress * 100:5.1f}%  {info.message[:50]:<50}",
          end="", flush=True)
    if info.terminal:
        print()


def cmd_health(args) -> int:
    c = _client(args)
    live = c.health()
    ok, body = c.ready()
    print(f"live: {live.get('status')}  version {live.get('version')}")
    print(f"ready: {'yes' if ok else 'no'}  profile {body.get('profile')}  backends {body.get('backends')}")
    for w in body.get("workers", []):
        print(f"  worker {w['name']:<16} {w['state']:<8} smoke={w['smoke_ok']}  gpus={w['gpus']}  {w.get('error') or ''}")
    return 0 if ok else 1


def cmd_status(args) -> int:
    print(json.dumps(_client(args).status(), indent=2))
    return 0


def cmd_models(args) -> int:
    models = _client(args).models()
    if args.json:
        print(json.dumps({k: v.model_dump() for k, v in models.items()}, indent=2))
        return 0
    print(f"{'backend':<16} {'available':<10} {'queued':<7} controls")
    for mid, m in models.items():
        ctrls = ", ".join(c.name + ("*" if c.required else "") for c in m.contract.controls)
        print(f"{mid:<16} {'yes' if m.available else 'no':<10} {m.queued:<7} {ctrls}")
    return 0


def cmd_submit(args) -> int:
    c = _client(args)
    clip = Clip.load(args.clip)
    controls = {}
    for spec in args.control:
        name, _, how = spec.partition("=")
        how = how or "clip"
        if ":" in how:
            how, w = how.split(":", 1)
            controls[name] = (how, float(w))
        else:
            controls[name] = how
    extra = {}
    for kv in args.extra:
        k, _, v = kv.partition("=")
        try:
            extra[k] = json.loads(v)
        except json.JSONDecodeError:
            extra[k] = v
    job = c.submit_clip(clip, args.backend, args.prompt, controls, views=args.view or None,
                        rgb=None if args.rgb == "auto" else args.rgb == "yes", negative_prompt=args.negative_prompt,
                        seed=args.seed, guidance=args.guidance, num_steps=args.steps, priority=args.priority,
                        extra=extra)
    print(f"submitted {job.id} ({job.info.backend}, {job.info.priority}); queue position {job.info.queue_position}")
    if args.wait or args.out:
        return _wait_and_fetch(job, args.out)
    return 0


def _wait_and_fetch(job, out: str | None) -> int:
    try:
        job.wait(on_progress=_print_progress)
    except JobFailed as exc:
        print(f"\n{exc}", file=sys.stderr)
        return 1
    if out:
        paths = job.result().download(Path(out) / job.id)
        print(f"downloaded {len(paths)} file(s) to {Path(out) / job.id}")
    return 0


def cmd_jobs(args) -> int:
    jobs = _client(args).jobs(status=args.status, backend=args.backend, mine=args.mine, limit=args.limit)
    if args.json:
        print(json.dumps([j.model_dump() for j in jobs], indent=2))
        return 0
    print(f"{'id':<20} {'backend':<16} {'prio':<12} {'status':<10} {'prog':>5}  {'created':<20} message")
    for j in jobs:
        print(f"{j.id:<20} {j.backend:<16} {j.priority:<12} {j.status:<10} {j.progress * 100:4.0f}%  "
              f"{j.created[:19]:<20} {j.error or j.message}")
    return 0


def cmd_watch(args) -> int:
    return _wait_and_fetch(_client(args).job(args.job), args.out)


def cmd_result(args) -> int:
    job = _client(args).job(args.job)
    paths = job.result().download(Path(args.out) / job.id)
    for name, p in paths.items():
        print(f"{name}  ->  {p}")
    return 0


def cmd_cancel(args) -> int:
    print(json.dumps(_client(args).cancel(args.job)))
    return 0


def cmd_serve(args) -> int:
    from . import serve

    if args.stop:
        for c in serve.find_containers():
            if args.port is None or c.port == args.port:
                print(f"stopping {c.name} ({c.image}, port {c.port})")
                serve.stop_container(c.name, remove=args.rm)
        return 0
    if args.mock:
        state = Path(args.state or "~/.carla-cosmos/mock-state").expanduser()
        srv = serve.MockServer(state, port=args.port or 8000, token=args.token)
        client = srv.start()
        print(f"mock server ready at {srv.url}\n  export COSMOS_URL={srv.url}\n  export COSMOS_TOKEN={srv.token}")
        try:
            while srv.proc and srv.proc.poll() is None:
                time.sleep(1)
        except KeyboardInterrupt:
            pass
        finally:
            srv.stop()
        return 0
    c, client = serve.ensure_server(image=args.image, port=args.port or 8000,
                                    state_dir=args.state or "~/.carla-cosmos/state", gpus=args.gpus,
                                    profile=args.profile, token=args.token)
    print(f"server ready: {c.url}  (container {c.name}, image {c.image})")
    print(f"  export COSMOS_URL={c.url}\n  export COSMOS_TOKEN={client.token}")
    return 0


def cmd_synthetic(args) -> int:
    from .synthetic import av7_clip, make_clip

    clip = av7_clip(args.out, seconds=args.seconds) if args.av7 else make_clip(
        args.out, frames=args.frames, fps=args.fps, scene=args.scene)
    print(clip.path)
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="carla-cosmos", description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--url", default=None, help="server URL (COSMOS_URL, default http://localhost:8000)")
    p.add_argument("--token", default=None, help="bearer token (COSMOS_TOKEN)")
    p.add_argument("--token-file", default=None, help="file with the bearer token (COSMOS_TOKEN_FILE)")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("health", help="liveness/readiness and worker states").set_defaults(fn=cmd_health)
    sub.add_parser("status", help="server status JSON").set_defaults(fn=cmd_status)
    m = sub.add_parser("models", help="backend contracts and availability")
    m.add_argument("--json", action="store_true")
    m.set_defaults(fn=cmd_models)

    s = sub.add_parser("submit", help="submit a clip")
    s.add_argument("--clip", required=True, help="clip directory (manifest.json)")
    s.add_argument("--backend", required=True)
    s.add_argument("--prompt", required=True)
    s.add_argument("--negative-prompt", default=None)
    s.add_argument("--control", action="append", default=[], metavar="NAME=SOURCE[:WEIGHT]",
                   help="depth=clip | edge=derive | wsm=scene | hdmap_bbox=scene:0.8 (repeatable)")
    s.add_argument("--view", action="append", default=[], help="camera name(s) for multi-camera clips")
    s.add_argument("--rgb", choices=["auto", "yes", "no"], default="auto", help="upload the RGB video")
    s.add_argument("--seed", type=int, default=0)
    s.add_argument("--guidance", type=float, default=None)
    s.add_argument("--steps", type=int, default=None)
    s.add_argument("--priority", choices=["interactive", "batch"], default="interactive")
    s.add_argument("--extra", action="append", default=[], metavar="KEY=JSON", help="backend pass-through")
    s.add_argument("--wait", action="store_true")
    s.add_argument("--out", default=None, help="download the result here (implies --wait)")
    s.set_defaults(fn=cmd_submit)

    j = sub.add_parser("jobs", help="list jobs")
    j.add_argument("--status", default=None)
    j.add_argument("--backend", default=None)
    j.add_argument("--mine", action="store_true", help="only jobs submitted with this token")
    j.add_argument("--limit", type=int, default=50)
    j.add_argument("--json", action="store_true")
    j.set_defaults(fn=cmd_jobs)

    w = sub.add_parser("watch", help="follow a job until it finishes")
    w.add_argument("job")
    w.add_argument("--out", default=None)
    w.set_defaults(fn=cmd_watch)

    r = sub.add_parser("result", help="download the result files of a finished job")
    r.add_argument("job")
    r.add_argument("--out", required=True)
    r.set_defaults(fn=cmd_result)

    cnl = sub.add_parser("cancel", help="cancel (or delete a finished) job")
    cnl.add_argument("job")
    cnl.set_defaults(fn=cmd_cancel)

    sv = sub.add_parser("serve", help="start or adopt a server (docker), or run the mock server")
    sv.add_argument("--image", default=None)
    sv.add_argument("--port", type=int, default=None)
    sv.add_argument("--state", default=None, help="host directory mounted at /state")
    sv.add_argument("--gpus", default="all", help="docker --gpus value ('' for none)")
    sv.add_argument("--profile", default=None, help="COSMOS_PROFILE for the container")
    sv.add_argument("--mock", action="store_true", help="plain-Python mock server (no docker/GPU)")
    sv.add_argument("--stop", action="store_true", help="stop the container(s) we started")
    sv.add_argument("--rm", action="store_true", help="with --stop: also remove the container")
    sv.set_defaults(fn=cmd_serve)

    sy = sub.add_parser("synthetic-clip", help="write an ffmpeg test-pattern clip (smoke tests)")
    sy.add_argument("--out", required=True)
    sy.add_argument("--frames", type=int, default=93)
    sy.add_argument("--fps", type=int, default=16)
    sy.add_argument("--scene", action="store_true")
    sy.add_argument("--av7", action="store_true", help="7-camera 30 fps clip with a scene package")
    sy.add_argument("--seconds", type=int, default=3)
    sy.set_defaults(fn=cmd_synthetic)
    return p


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.cmd == "serve":
        args.image = args.image or __import__("carla_cosmos.serve", fromlist=["DEFAULT_IMAGE"]).DEFAULT_IMAGE
    try:
        return args.fn(args)
    except CosmosError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
