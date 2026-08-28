"""``carla-cosmos`` command line.

    carla-cosmos health|status|models
    carla-cosmos submit --clip DIR --backend cosmos3-nano --prompt "..." --control depth=clip --control edge=derive [--wait [--out DIR] | --no-download]
    carla-cosmos jobs [--status running] [--mine] [--out DIR]
    carla-cosmos watch JOB [--out DIR]
    carla-cosmos result JOB [--out DIR]
    carla-cosmos cancel JOB
    carla-cosmos serve [--image IMG] [--port 8000] [--state DIR] [--gpus all] [--profile P] | --stop | --mock
    carla-cosmos classes
    carla-cosmos synthetic-clip --out DIR [--frames 93 --fps 16 | --av7]
    carla-cosmos preview --clip DIR [--cameras camera:front:wide:120fov] [--frames 0:60] [--out DIR] [--grid]
                         [--show-occluded] [--png-every N]

Connection: ``--url``/``--token`` or ``COSMOS_URL`` / ``COSMOS_TOKEN`` / ``COSMOS_TOKEN_FILE``.

Control weight is part of ``--control`` (``--control depth=clip:0.7``); there is
deliberately no second ``--weight`` flag.  ``submit --mask-classes vehicle,pedestrian``
blanks those CARLA semantic classes in every input derived from the captured
pixels (``carla-cosmos classes`` lists the names); it never touches the
world-scenario controls ``wsm`` / ``hdmap_bbox``.

Results are kept: whenever the CLI waits for a job it downloads every returned
file into ``<results root>/<clip_id>/<job_id>/`` next to a ``job.json`` (see
``results.py``).  The results root is ``--out``, else ``$COSMOS_RESULTS``, else
``./cosmos-results``; ``submit --wait --no-download`` opts out.  ``jobs`` says
which results are on this machine and where, and warns before the server
garbage-collects the ones that are not.
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
from .results import EXPIRY_WARN_HOURS, ResultStore, default_results_root
from .preview import DEFAULT_LAYERS as DEFAULT_PREVIEW_LAYERS  # numpy only, no cv2/carla at import


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
                        seed=args.seed, guidance=args.guidance, num_steps=args.steps, resolution=args.resolution,
                        priority=args.priority, extra=extra,
                        mask_classes=args.mask_classes or None, mask_dilate=args.mask_dilate)
    print(f"submitted {job.id} ({job.info.backend}, {job.info.priority}); queue position {job.info.queue_position}")
    ResultStore(args.out).note_submitted(job, clip_id=clip.manifest.clip_id)
    if args.wait or args.out:
        return _wait_and_fetch(job, args.out, download=not args.no_download)
    print(f"follow it with: carla-cosmos watch {job.id}"
          f"{'' if args.out is None else ' --out ' + args.out}")
    return 0


def _store_result(job, out: str | None) -> int:
    """Download every file of a finished job into the results root."""
    stored = job.download(out)
    print(f"stored {len(stored.files)} file(s) ({stored.bytes / 1e6:.1f} MB) in {stored.directory}")
    for f in stored.files:
        print(f"  {f.name}")
    _warn_expiry(stored.expires_in_hours(), job.id, stored=True)
    return 0


def _warn_expiry(hours: float | None, job_id: str, stored: bool) -> None:
    if hours is None or hours >= EXPIRY_WARN_HOURS:
        return
    where = "the local copy is the only one left after that" if stored else "download it before then"
    if hours <= 0:
        print(f"warning: the server copy of {job_id} is past its retention window; {where}", file=sys.stderr)
    else:
        print(f"warning: the server deletes {job_id} in {hours:.1f} h; {where}", file=sys.stderr)


def _wait_and_fetch(job, out: str | None, download: bool = True) -> int:
    try:
        job.wait(on_progress=_print_progress)
    except JobFailed as exc:
        print(f"\n{exc}", file=sys.stderr)
        return 1
    if not download:
        root = default_results_root(out)
        print(f"not downloaded (--no-download); fetch it later with: "
              f"carla-cosmos result {job.id} --out {root}")
        ttl = (job.client.retention() or {}).get("job_ttl_hours")
        if ttl:
            print(f"the server keeps it for {ttl:g} h", file=sys.stderr)
        return 0
    return _store_result(job, out)


def cmd_jobs(args) -> int:
    client = _client(args)
    jobs = client.jobs(status=args.status, backend=args.backend, mine=args.mine, limit=args.limit)
    store = ResultStore(args.out)
    local = {e.job_id: e for e in store.index()}
    if args.json:
        rows = []
        for j in jobs:
            e = local.get(j.id)
            rows.append({**j.model_dump(), "stored": bool(e and e.stored),
                         "directory": e.directory if e else None})
        rows += [{**e.model_dump(), "on_server": False} for e in local.values() if e.job_id not in {j.id for j in jobs}]
        print(json.dumps(rows, indent=2))
        return 0
    print(f"results root: {store.root}")
    print(f"{'id':<20} {'backend':<16} {'prio':<12} {'status':<10} {'prog':>5}  {'created':<20} message")
    for j in jobs:
        print(f"{j.id:<20} {j.backend:<16} {j.priority:<12} {j.status:<10} {j.progress * 100:4.0f}%  "
              f"{j.created[:19]:<20} {j.error or j.message}")
        e = local.get(j.id)
        if e is not None and e.stored:
            print(f"    stored: {e.directory} ({e.files} file(s), {e.bytes / 1e6:.1f} MB)")
        elif j.status == "done":
            print("    not stored locally — carla-cosmos result "
                  f"{j.id}{'' if args.out is None else ' --out ' + args.out}")
            _warn_expiry(_expiry_hours(j, client), j.id, stored=False)
    orphans = [e for e in local.values() if e.job_id not in {j.id for j in jobs}]
    if orphans:
        print(f"\nknown locally but not listed by the server ({len(orphans)}):")
        for e in sorted(orphans, key=lambda e: e.downloaded or ""):
            where = e.directory if e.stored else "not downloaded"
            print(f"{e.job_id:<20} {e.backend:<16} {e.status:<10} {where}")
    return 0


def _expiry_hours(info: JobInfo, client) -> float | None:
    """Hours until the server garbage-collects this job's output."""
    from .results import parse_time

    ttl = (client.retention() or {}).get("job_ttl_hours")
    finished = parse_time(info.finished)
    if not ttl or finished is None:
        return None
    import datetime as _dt

    return ttl - (_dt.datetime.now(_dt.timezone.utc) - finished).total_seconds() / 3600.0


def cmd_watch(args) -> int:
    return _wait_and_fetch(_client(args).job(args.job), args.out, download=not args.no_download)


def cmd_result(args) -> int:
    return _store_result(_client(args).job(args.job), args.out)


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


def cmd_classes(args) -> int:
    from .mask import GROUPS, TAG_NAMES, class_table

    if args.json:
        print(json.dumps({"tags": {n: i for i, n in enumerate(TAG_NAMES)},
                          "groups": {g: list(t) for g, t in GROUPS.items()}}, indent=2))
        return 0
    print(f"{'name':<16} {'id':>4}  also accepted")
    for name, tag, aliases in class_table():
        print(f"{name:<16} {tag if tag >= 0 else '':>4}  {', '.join(aliases)}")
    print("\nUse with: carla-cosmos submit ... --mask-classes vehicle,pedestrian")
    return 0


def cmd_synthetic(args) -> int:
    from .synthetic import av7_clip, make_clip

    clip = av7_clip(args.out, seconds=args.seconds) if args.av7 else make_clip(
        args.out, frames=args.frames, fps=args.fps, scene=args.scene)
    print(clip.path)
    return 0


def cmd_preview(args) -> int:
    from .preview import DEFAULT_LAYERS, preview_clip

    cameras = [c for spec in args.cameras for c in spec.split(",") if c]
    layers = [l for spec in args.layers for l in spec.split(",") if l] or list(DEFAULT_LAYERS)
    state = {"camera": None}

    def progress(camera: str, done: int, total: int) -> None:
        if state["camera"] != camera:
            if state["camera"] is not None:
                print()
            state["camera"] = camera
        print(f"\r  {camera:<28} {done:>5}/{total}", end="", flush=True)

    try:
        written = preview_clip(args.clip, cameras=cameras or None, frames=args.frames, out_dir=args.out,
                               grid=args.grid, layers=layers, dim=args.dim, thickness=args.thickness,
                               progress=None if args.quiet else progress,
                               show_occluded=args.show_occluded, png_every=args.png_every)
    except (FileNotFoundError, ValueError) as exc:
        print(f"\nerror: {exc}", file=sys.stderr)
        return 1
    if not args.quiet and state["camera"] is not None:
        print()
    for name, path in written.items():
        print(f"{name}  ->  {path}")
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
    s.add_argument("--resolution", default=None, help="resolution bucket, e.g. 480 or 720")
    s.add_argument("--priority", choices=["interactive", "batch"], default="interactive")
    s.add_argument("--extra", action="append", default=[], metavar="KEY=JSON", help="backend pass-through")
    s.add_argument("--mask-classes", action="append", default=[], metavar="NAME[,NAME...]",
                   help="CARLA semantic classes to remove from the pixel-derived inputs "
                        "(depth/seg/edge/vis and the uploaded RGB), e.g. vehicle,pedestrian; "
                        "'carla-cosmos classes' lists them")
    s.add_argument("--mask-dilate", type=int, default=None, metavar="PX",
                   help="grow the mask by this many pixels so object outlines do not leak (default 3)")
    s.add_argument("--wait", action="store_true")
    s.add_argument("--out", default=None,
                   help="results root (default $COSMOS_RESULTS or ./cosmos-results); implies --wait. "
                        "Files land in <root>/<clip_id>/<job_id>/")
    s.add_argument("--no-download", action="store_true", help="with --wait: do not store the result")
    s.set_defaults(fn=cmd_submit)

    j = sub.add_parser("jobs", help="list jobs")
    j.add_argument("--status", default=None)
    j.add_argument("--backend", default=None)
    j.add_argument("--mine", action="store_true", help="only jobs submitted with this token")
    j.add_argument("--limit", type=int, default=50)
    j.add_argument("--out", default=None, help="results root to look for stored results in")
    j.add_argument("--json", action="store_true")
    j.set_defaults(fn=cmd_jobs)

    w = sub.add_parser("watch", help="follow a job until it finishes (and store its result)")
    w.add_argument("job")
    w.add_argument("--out", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    w.add_argument("--no-download", action="store_true", help="do not store the result")
    w.set_defaults(fn=cmd_watch)

    r = sub.add_parser("result", help="download the result files of a finished job")
    r.add_argument("job")
    r.add_argument("--out", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
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

    cls = sub.add_parser("classes", help="semantic class names accepted by --mask-classes")
    cls.add_argument("--json", action="store_true")
    cls.set_defaults(fn=cmd_classes)

    sy = sub.add_parser("synthetic-clip", help="write an ffmpeg test-pattern clip (smoke tests)")
    sy.add_argument("--out", required=True)
    sy.add_argument("--frames", type=int, default=93)
    sy.add_argument("--fps", type=int, default=16)
    sy.add_argument("--scene", action="store_true")
    sy.add_argument("--av7", action="store_true", help="7-camera 30 fps clip with a scene package")
    sy.add_argument("--seconds", type=int, default=3)
    sy.set_defaults(fn=cmd_synthetic)

    pv = sub.add_parser("preview", help="draw the exported ClipGT scene on the clip's RGB (local GT check)")
    pv.add_argument("--clip", required=True, help="clip directory (manifest.json + scene/)")
    pv.add_argument("--cameras", action="append", default=[], metavar="NAME[,NAME...]",
                    help="cameras to draw (default: all in the clip)")
    pv.add_argument("--frames", default=None, metavar="A:B", help="frame range, e.g. 0:60 or 100:")
    pv.add_argument("--out", default=None, help="output directory (default: <clip>/preview)")
    pv.add_argument("--grid", action="store_true", help="also write one labelled grid video (4 cameras per row)")
    pv.add_argument("--layers", action="append", default=[], metavar="NAME[,NAME...]",
                    help="tables to draw: " + ", ".join(DEFAULT_PREVIEW_LAYERS))
    pv.add_argument("--show-occluded", action="store_true",
                    help="also draw, dashed and grey, the obstacles the capture's occlusion filter "
                         "dropped (needs the exporter's <clip>.visibility.json sidecar)")
    pv.add_argument("--png-every", type=int, default=0, metavar="N",
                    help="also write every Nth drawn frame as a PNG next to the videos")
    pv.add_argument("--dim", type=float, default=0.6, help="darken the RGB by this factor (1.0 = off)")
    pv.add_argument("--thickness", type=int, default=2)
    pv.add_argument("--quiet", action="store_true")
    pv.set_defaults(fn=cmd_preview)
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
