#! /usr/bin/env python3
"""Run the Cosmos mode matrix against a node and keep every video it returns.

One row per mode: what the model is conditioned on (which controls, with which
weights, with which classes masked out) and what the prompt asks for.  Every row
is submitted through the Python API, waited for, and stored through
:class:`carla_cosmos.ResultStore` — ``<results>/<clip_id>/<job_id>/`` with
``job.json`` and ``manifest.json`` next to the videos — so the matrix is a
reproducible record, not a pile of downloads.

    export COSMOS_URL=http://<node>:8000
    python demos/showcase.py --token-file ~/.cosmos-token --clips ./clips
    python demos/showcase.py --list                      # the matrix, nothing else
    python demos/showcase.py --only t25-golden,av7-day   # a subset
    python demos/showcase.py --mock                      # everything against the mock server

It is **resumable**: a row whose stored directory already holds a video is
skipped, so an interrupted run continues where it stopped (``--force`` reruns).
Per-row timings (queue, scene rendering, generation, upload, download) are
written to ``<results>/showcase.json``, which is also what
``tools/showcase_sheets.py`` reads to build the comparison sheets.

Clips (capture them with ``demos/single_view_live.py --capture-only`` /
``demos/av7_world_scenario.py --capture-only``, or point ``--clip`` at your own):

===========  ==================================  ==========================================
role         what it must be                     used by
===========  ==================================  ==========================================
``t25``      93*k frames @ 16 fps, one camera,   Transfer 2.5 general rows.  Needs the
             a semantic AOV for the mask rows    ``semantic`` AOV (the capture default).
``wsm``      101*k frames @ 10/15/30 fps with    Cosmos 3 Nano world-scenario rows
             a ``scene/`` package
``av7``      (29+28k)*3 frames @ 30 fps,         Transfer 2.5 AV multiview rows
             ``nvidia_av7`` rig, ``scene/``
===========  ==================================  ==========================================
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from api_common import connect, load_clip, results_root, setup_logging  # puts ../client on sys.path

from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402
from carla_cosmos.results import ResultStore  # noqa: E402

GOLDEN = ("A city street at golden hour, low warm sunlight raking across the asphalt, long shadows "
          "from the buildings, photorealistic dashcam footage, cinematic colour grading")


@dataclass
class Row:
    """One cell of the matrix: a backend, a clip, a conditioning setup and a prompt."""

    id: str
    backend: str
    clip: str
    """Clip role: ``t25`` | ``wsm`` | ``av7``."""
    prompt: str
    controls: dict[str, str]
    shows: str
    weights: dict[str, float] | None = None
    mask_classes: list[str] | None = None
    resolution: str | None = None
    views: int | None = None
    seed: int = 7
    extra: dict[str, Any] = field(default_factory=dict)


MATRIX: list[Row] = [
    # ---- Transfer 2.5 general: same clip, same controls, three weathers ---------------------
    Row("t25-golden", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="480",
        shows="depth + seg + edge, golden hour (the reference row)"),
    Row("t25-rain", "transfer2.5", "t25",
        "The same street in heavy rain, wet asphalt mirroring the traffic lights and the headlights, "
        "spray thrown up behind the cars, raindrops on the windshield, low overcast sky, "
        "photorealistic dashcam footage",
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="480",
        shows="same controls, heavy rain and a wet road"),
    Row("t25-night", "transfer2.5", "t25",
        "The same street at night, street lamps and lit shop windows on the sidewalks, headlights and red "
        "tail lights on the road, deep blue sky above the rooftops, photorealistic dashcam footage",
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="480",
        shows="same controls, night"),
    # ---- control weights: what each branch contributes --------------------------------------
    Row("t25-w-depthseg", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip"}, weights={"depth": 0.7, "seg": 0.3}, resolution="480",
        shows="geometry-led: depth 0.7 / seg 0.3, no edge branch"),
    Row("t25-w-edge", "transfer2.5", "t25", GOLDEN,
        {"edge": "derive"}, weights={"edge": 1.0}, resolution="480",
        shows="edge branch alone at weight 1.0 (same prompt as t25-golden)"),
    # ---- mask-out classes: the model may re-imagine what was removed ------------------------
    Row("t25-mask-vehicles", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, mask_classes=["vehicle"], resolution="480",
        shows="vehicles removed from every pixel-derived input (and the ego bonnet with them)"),
    Row("t25-mask-vru", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, mask_classes=["vru"], resolution="480",
        shows="vulnerable road users removed (pedestrians, riders, bicycles, motorcycles), "
              "same prompt and controls as t25-golden"),
    # ---- Cosmos 3 Nano: the world-scenario map ----------------------------------------------
    Row("c3-wsm", "cosmos3-nano", "wsm",
        "A photorealistic city street in the early afternoon, clear sky, parked cars along the curb, "
        "pedestrians on the sidewalk, footage from a car-mounted camera",
        {"wsm": "scene"},
        shows="world-scenario map alone: layout from the scene package, appearance from the prompt"),
    Row("c3-wsm-depth-seg", "cosmos3-nano", "wsm",
        "A photorealistic city street in the early afternoon, clear sky, parked cars along the curb, "
        "pedestrians on the sidewalk, footage from a car-mounted camera",
        {"wsm": "scene", "depth": "clip", "seg": "clip"},
        shows="world-scenario map + depth and seg: layout and the captured appearance"),
    # ---- Transfer 2.5 AV multiview: seven cameras, one joint generation ---------------------
    Row("av7-day", "transfer2.5-av", "av7",
        "An urban street in the late afternoon, clear sky, dry asphalt, photorealistic footage from a "
        "seven-camera vehicle rig, consistent lighting across all cameras",
        {"hdmap_bbox": "scene"}, views=7,
        shows="7 cameras, hdmap+bbox rendered from the occlusion-filtered scene package"),
    Row("av7-rain", "transfer2.5-av", "av7",
        "The same urban street on a rainy evening, wet reflective asphalt, headlights and street lights "
        "reflected in the puddles, photorealistic footage from a seven-camera vehicle rig",
        {"hdmap_bbox": "scene"}, views=7,
        shows="same 7-camera scene, weather variation"),
]

CLIP_ROLES = ("t25", "wsm", "av7")
DEFAULT_CLIP_NAMES = {"t25": "showcase_t25", "wsm": "showcase_wsm", "av7": "showcase_av7"}


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--endpoint", "--url", dest="endpoint", default=None, help="node URL (default $COSMOS_URL)")
    ap.add_argument("--token", default=None, help="bearer token (default $COSMOS_TOKEN)")
    ap.add_argument("--token-file", default=None, help="file holding the bearer token (never logged)")
    ap.add_argument("--clips", default="./clips", help="directory holding the showcase clips")
    ap.add_argument("--clip", action="append", default=[], metavar="ROLE=DIR",
                    help=f"clip for one role ({'|'.join(CLIP_ROLES)}), overriding --clips")
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--only", default=None, help="comma-separated row ids (or backends) to run")
    ap.add_argument("--skip", default=None, help="comma-separated row ids to leave out")
    ap.add_argument("--force", action="store_true", help="rerun rows that are already stored")
    ap.add_argument("--list", action="store_true", help="print the matrix and exit")
    ap.add_argument("--dry-run", action="store_true", help="resolve clips and validate, submit nothing")
    ap.add_argument("--mock", action="store_true", help="run against an in-process mock server (no GPU)")
    ap.add_argument("--stop-on-error", action="store_true", help="stop at the first failed row")
    ap.add_argument("-v", "--verbose", action="store_true")
    return ap.parse_args(argv)


def select(only: str | None, skip: str | None) -> list[Row]:
    rows = list(MATRIX)
    if only:
        wanted = {w.strip() for w in only.split(",") if w.strip()}
        rows = [r for r in rows if r.id in wanted or r.backend in wanted or r.clip in wanted]
        unknown = wanted - {r.id for r in MATRIX} - {r.backend for r in MATRIX} - set(CLIP_ROLES)
        if unknown:
            raise SystemExit(f"--only: no such row/backend/clip {sorted(unknown)}; "
                             f"ids are {', '.join(r.id for r in MATRIX)}")
    if skip:
        drop = {w.strip() for w in skip.split(",")}
        rows = [r for r in rows if r.id not in drop]
    if not rows:
        raise SystemExit("nothing selected")
    return rows


def resolve_clips(args: argparse.Namespace, rows: list[Row]) -> dict[str, Path]:
    """``role -> clip directory`` for the roles the selected rows need."""
    override = {}
    for spec in args.clip:
        role, _, path = spec.partition("=")
        if role not in CLIP_ROLES:
            raise SystemExit(f"--clip {spec}: role must be one of {', '.join(CLIP_ROLES)}")
        override[role] = Path(path)
    out: dict[str, Path] = {}
    for role in {r.clip for r in rows}:
        path = override.get(role, Path(args.clips) / DEFAULT_CLIP_NAMES[role])
        if not (path / "manifest.json").exists():
            raise SystemExit(f"clip role '{role}': {path} is not a clip directory.\n"
                             f"  capture one (see the table in --help) or pass --clip {role}=<dir>")
        out[role] = path
    return out


def token_of(args: argparse.Namespace) -> str | None:
    if args.token_file:
        return Path(args.token_file).expanduser().read_text().strip()
    return args.token


def describe(row: Row) -> str:
    controls = " ".join(f"{n}={h}" + (f":{row.weights[n]:g}" if row.weights and n in row.weights else "")
                        for n, h in row.controls.items())
    mask = f" mask={','.join(row.mask_classes)}" if row.mask_classes else ""
    return f"{controls}{mask}"


def print_matrix(rows: list[Row]) -> None:
    print(f"{'id':<22} {'backend':<14} {'clip':<5} conditioning / what it shows")
    for r in rows:
        print(f"{r.id:<22} {r.backend:<14} {r.clip:<5} {describe(r)}")
        print(f"{'':<43} {r.shows}")


def stored_result(store: ResultStore, ledger: dict[str, Any], row: Row) -> Path | None:
    """Directory of an earlier run of ``row`` that already holds a video, if any."""
    entry = ledger.get(row.id)
    if not entry or not entry.get("directory"):
        return None
    d = Path(entry["directory"])
    return d if d.is_dir() and any(d.glob("*.mp4")) else None


def run_row(cosmos, row: Row, clip, results: str | None, log) -> dict[str, Any]:
    """Submit one row, wait, store; returns the ledger entry (never raises for a job failure)."""
    views = None
    if row.views:
        views = clip.manifest.camera_names[:row.views]
    rec: dict[str, Any] = {"id": row.id, "backend": row.backend, "clip_id": clip.manifest.clip_id,
                           "clip_dir": str(clip.path), "prompt": row.prompt, "controls": row.controls,
                           "weights": row.weights, "mask_classes": row.mask_classes, "shows": row.shows,
                           "resolution": row.resolution, "seed": row.seed}
    t0 = time.time()
    try:
        job = cosmos.submit_clip(clip, row.backend, row.prompt, dict(row.controls), views=views,
                                 weights=row.weights, mask_classes=row.mask_classes,
                                 resolution=row.resolution, seed=row.seed, extra=row.extra or None)
    except CosmosError as exc:
        rec.update(status="rejected", error=str(exc), errors=list(getattr(exc, "errors", []) or []))
        log.error("[%s] rejected: %s %s", row.id, exc, rec["errors"])
        return rec
    t_submit = time.time() - t0
    rec.update(job_id=job.id, upload_s=round(t_submit, 1))
    log.info("[%s] job %s queued (position %s) after %.1fs of masking+upload",
             row.id, job.id, job.info.queue_position, t_submit)

    last = [""]

    def progress(info) -> None:
        line = f"{info.status} {info.progress * 100:.0f}% {info.message}"
        if line != last[0]:
            log.info("[%s] %s", row.id, line)
            last[0] = line

    try:
        info = job.wait(poll=5.0, on_progress=progress)
    except JobFailed as exc:
        rec.update(status="failed", error=str(exc))
        log.error("[%s] %s", row.id, exc)
        return rec
    t_dl = time.time()
    stored = job.download(results)
    rec.update(status=info.status, worker=info.worker, directory=stored.directory,
               timings={k: round(v, 1) for k, v in stored.timings.items()},
               download_s=round(time.time() - t_dl, 1), wall_s=round(time.time() - t0, 1),
               files=len(stored.files), bytes=stored.bytes,
               videos=stored.videos)
    return rec


def line_for(rec: dict[str, Any]) -> str:
    t = rec.get("timings") or {}
    if rec.get("status") != "done":
        return f"{'FAIL':<4} {rec['id']:<22} {rec.get('status', '?'):<9} {str(rec.get('error'))[:90]}"
    return (f"{'ok':<4} {rec['id']:<22} {rec['backend']:<14} "
            f"queue {t.get('queued', 0):6.1f}s  render {t.get('rendering', 0):6.1f}s  "
            f"gen {t.get('running', 0):7.1f}s  up {rec.get('upload_s', 0):6.1f}s  "
            f"dl {rec.get('download_s', 0):5.1f}s  wall {rec.get('wall_s', 0):7.1f}s  "
            f"{rec.get('files', 0)} files {rec.get('bytes', 0) / 1e6:6.1f} MB")


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    log = setup_logging("showcase", args.verbose)
    rows = select(args.only, args.skip)
    if args.list:
        print_matrix(rows)
        return 0

    clips = resolve_clips(args, rows)
    root = results_root(args.results)
    store = ResultStore(root)
    ledger_path = root / "showcase.json"
    ledger: dict[str, Any] = {}
    if ledger_path.exists():
        ledger = json.loads(ledger_path.read_text())

    loaded = {role: load_clip(str(p)) for role, p in clips.items()}
    for role, clip in sorted(loaded.items()):
        log.info("clip %-4s %-28s %4d frames @ %2d fps, %d camera(s), scene=%s", role, clip.manifest.clip_id,
                 clip.manifest.frames, clip.manifest.fps, len(clip.manifest.camera_names),
                 clip.scene_dir is not None and clip.scene_dir.exists())
    log.info("results root %s", root)

    if args.dry_run:
        print_matrix(rows)
        print(f"\ndry run: {len(rows)} row(s) would be submitted; clips {[str(p) for p in clips.values()]}")
        return 0

    srv = None
    if args.mock:
        from carla_cosmos.serve import MockServer
        srv = MockServer(root / "_mock-state", delay=1.0, log_file=root / "_mock-state" / "server.log")
        cosmos = srv.start()
        log.info("mock server at %s", cosmos.url)
    else:
        cosmos = connect(args.endpoint, token_of(args))
        models = cosmos.models()
        log.info("node %s serves %s", cosmos.url,
                 ", ".join(sorted(m for m, i in models.items() if i.available)) or "nothing")
        missing = sorted({r.backend for r in rows if r.backend not in models or not models[r.backend].available})
        if missing:
            log.warning("not loaded on this node: %s — those rows will be reported as unavailable", missing)

    records: list[dict[str, Any]] = []
    try:
        for row in rows:
            done_dir = None if args.force else stored_result(store, ledger, row)
            if done_dir is not None:
                log.info("[%s] already stored in %s — skipping (--force to rerun)", row.id, done_dir)
                records.append(ledger[row.id])
                continue
            if not args.mock:
                info = cosmos.models().get(row.backend)
                if info is None or not info.available:
                    rec = {"id": row.id, "backend": row.backend, "status": "unavailable",
                           "error": f"{row.backend} is not loaded on {cosmos.url}", "shows": row.shows}
                    records.append(rec)
                    ledger[row.id] = rec
                    ledger_path.write_text(json.dumps(ledger, indent=2))
                    log.error("[%s] %s", row.id, rec["error"])
                    if args.stop_on_error:
                        break
                    continue
            rec = run_row(cosmos, row, loaded[row.clip], args.results, log)
            records.append(rec)
            ledger[row.id] = rec
            ledger_path.write_text(json.dumps(ledger, indent=2))
            print(line_for(rec), flush=True)
            if rec.get("status") != "done" and args.stop_on_error:
                log.error("stopping after %s (--stop-on-error)", row.id)
                break
    finally:
        if srv:
            srv.stop()

    print(f"\n{'':4} {'row':<22} {'backend':<14} timings")
    for rec in records:
        print(line_for(rec))
    bad = [r for r in records if r.get("status") != "done"]
    print(f"\n{len(records) - len(bad)}/{len(records)} row(s) done; ledger {ledger_path}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
