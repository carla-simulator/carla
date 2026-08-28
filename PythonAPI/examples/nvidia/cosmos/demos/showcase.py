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

Every stored row also gets ``viewer_<layout>.mp4`` — the side-by-side viewer of
that row (input | control | result, or the per-camera grid) rendered to a video
without a display, so a matrix run leaves something watchable behind and not
only raw per-camera clips.  ``--no-viewer-video`` turns it off.

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

NIGHT = ("The same street at night, street lamps and lit shop windows on the sidewalks, headlights and red "
         "tail lights on the road, deep blue sky above the rooftops, photorealistic dashcam footage")

CITY_DAY = ("A photorealistic city street in the early afternoon, clear sky, parked cars along the curb, "
            "pedestrians on the sidewalk, footage from a car-mounted camera")

# The Cosmos 3 tuning rows: name what is actually in the frame instead of describing a generic
# street, and say in the negative prompt what the untuned c3-wsm-depth-seg render looked like.
TUNE_PROMPT = ("dark red sedan directly ahead, white ambulance on the right, cyclists, tall downtown "
               "buildings, crisp sharp midday light, high contrast, photorealistic dashcam footage")
TUNE_NEGATIVE = "hazy, foggy, washed out, low contrast, blurry, overexposed, dull colors"

# The NuRec rows all run this one prompt: the reconstruction is a suburban retail strip outside
# Stockholm recorded after dark, and every row asks for the same place in daylight.  Holding the
# prompt fixed is what makes the trio a comparison of the *conditioning* and nothing else.
NUREC_DAY = ("A photorealistic dashcam drive along a wide suburban retail street outside "
             "Stockholm on a clear day, bright midday sun, blue sky, low-rise shops with "
             "signage set back behind their parking, bare trees along the roadside, painted "
             "lane markings on grey asphalt, a steel guardrail on the right")

# vLLM-Omni's Cosmos 3 transfer path (pinned ref d3c990dc,
# ``vllm_omni/diffusion/models/cosmos3/{transfer,pipeline_cosmos3}.py``) applies its per-hint
# preset (``TRANSFER_DEFAULTS``: wsm 1.0/3.0/10, depth+edge+blur 3.0/1.5/10, seg 3.0/2.0/10 for
# guidance / control_guidance / flow_shift) **only when the request carries exactly one hint**.
# A multi-hint request such as ``c3-wsm-depth-seg`` silently falls back to
# ``COSMOS3_T2V_DEFAULT_GUIDANCE_SCALE = 6.0`` with ``control_guidance`` 1.0 and the engine's flow
# shift -- measured on 2026-08-28 that is what makes it hazy and low-contrast next to the sharp
# single-hint ``c3-wsm``.  The tuning rows put the single-hint regime back on by hand;
# ``control_guidance`` and ``flow_shift`` travel in ``JobRequest.extra``, which the cosmos3 worker
# passes to vLLM-Omni verbatim.
PRESET_REGIME: dict[str, Any] = {"control_guidance": 2.0, "flow_shift": 10.0}


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
    negative_prompt: str | None = None
    guidance: float | None = None
    """Text CFG scale.  ``None`` leaves the backend default (see the tuning rows)."""
    skip: str | None = None
    """Why this row is defined but not submitted.  Kept in the matrix as a record of the
    experiment and the reason it is not worth GPU time; ``--force`` runs it anyway."""
    extra: dict[str, Any] = field(default_factory=dict)


# Every row names its resolution, and every row is 720 — the videos are meant to be watched, and a
# 480p run is money spent on something nobody will show.  Naming it is not decoration: a row that
# leaves it out silently inherits the *worker's* default (720 for Cosmos 3, from
# `--default-resolution`), so the timing it produces cannot be compared with anything.
# `check_matrix()` refuses a row without one.
MATRIX: list[Row] = [
    # ---- Transfer 2.5 general: same clip, same controls, three weathers ---------------------
    Row("t25-golden", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="720",
        shows="depth + seg + edge, golden hour (the reference row)"),
    Row("t25-rain", "transfer2.5", "t25",
        "The same street in heavy rain, wet asphalt mirroring the traffic lights and the headlights, "
        "spray thrown up behind the cars, raindrops on the windshield, low overcast sky, "
        "photorealistic dashcam footage",
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="720",
        shows="same controls, heavy rain and a wet road"),
    Row("t25-night", "transfer2.5", "t25", NIGHT,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, resolution="720",
        shows="same controls, night"),
    # ---- control weights: what each branch contributes --------------------------------------
    Row("t25-w-depthseg", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip"}, weights={"depth": 0.7, "seg": 0.3}, resolution="720",
        shows="geometry-led: depth 0.7 / seg 0.3, no edge branch"),
    Row("t25-w-edge", "transfer2.5", "t25", GOLDEN,
        {"edge": "derive"}, weights={"edge": 1.0}, resolution="720",
        shows="edge branch alone at weight 1.0 (same prompt as t25-golden)"),
    # ---- mask-out classes: the model may re-imagine what was removed ------------------------
    Row("t25-mask-vehicles", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, mask_classes=["vehicle"], resolution="720",
        shows="vehicles removed from every pixel-derived input (and the ego bonnet with them)"),
    Row("t25-mask-vru", "transfer2.5", "t25", GOLDEN,
        {"depth": "clip", "seg": "clip", "edge": "derive"}, mask_classes=["vru"], resolution="720",
        shows="vulnerable road users removed (pedestrians, riders, bicycles, motorcycles), "
              "same prompt and controls as t25-golden"),
    # ---- vis: the "just RGB" restyle mode ---------------------------------------------------
    # No control video is uploaded at all: the only input is the RGB, and Transfer 2.5 derives the
    # `vis` (blurred-colour) control from it server-side.  Compared with the depth+seg+edge rows
    # this keeps the *appearance* of the capture (colours, layout, lighting direction) and lets the
    # prompt do the restyling, instead of handing the model an explicit geometry branch.
    Row("t25-vis-golden", "transfer2.5", "t25", GOLDEN,
        {"vis": "derive"}, resolution="720",
        shows="vis derived from the RGB alone, golden hour (same prompt as t25-golden)"),
    Row("t25-vis-night", "transfer2.5", "t25", NIGHT,
        {"vis": "derive"}, resolution="720",
        shows="vis derived from the RGB alone, night (same prompt as t25-night)"),
    # The mask reaches a derived control twice over: the uploaded RGB is blanked, so the server-side
    # derivation sees the hole, *and* Transfer 2.5 takes the mask video itself as ControlConfig
    # mask_path -- the vis branch has zero weight inside the hole (contracts.py, "Masking").
    Row("t25-vis-mask-vehicles", "transfer2.5", "t25", GOLDEN,
        {"vis": "derive"}, mask_classes=["vehicle"], resolution="720",
        shows="vis from a masked RGB + the mask as a control-weight map: no evidence of the traffic"),
    Row("t25-vis-mask-vru", "transfer2.5", "t25", GOLDEN,
        {"vis": "derive"}, mask_classes=["vru"], resolution="720",
        shows="same as t25-vis-mask-vehicles for the vulnerable road users"),
    # ---- Cosmos 3 Nano: the world-scenario map ----------------------------------------------
    Row("c3-wsm", "cosmos3-nano", "wsm",
        CITY_DAY,
        {"wsm": "scene"}, resolution="720",
        shows="world-scenario map alone: layout from the scene package, appearance from the prompt"),
    Row("c3-wsm-depth-seg", "cosmos3-nano", "wsm",
        CITY_DAY,
        {"wsm": "scene", "depth": "clip", "seg": "clip"}, resolution="720",
        shows="world-scenario map + depth and seg: layout and the captured appearance"),
    # ---- Cosmos 3 Nano: the "just RGB" restyle mode, the A/B against the wsm rows -----------
    # Same clip, prompt, seed and resolution as ``c3-wsm``; the only difference is what the model is
    # conditioned on -- ``blur`` derived from the RGB instead of the world-scenario map.  Cosmos 3
    # takes no mask video (``accepts_mask`` is false for every cosmos3 control), so on the masked row
    # the hole exists only in the pixels the derivation reads.
    Row("c3-blur", "cosmos3-nano", "wsm", CITY_DAY,
        {"blur": "derive"}, resolution="720", seed=7,
        shows="blur derived from the RGB alone -- the Nano restyle, no world-scenario map"),
    # Ran once on 2026-08-28 and answered the question for good: Cosmos 3 has no mask input, so the
    # hole exists only in the pixels the blur hint is derived from - and the model reproduced the
    # black silhouettes frame for frame instead of re-imagining them.  The client now refuses
    # mask_classes for a backend with no maskable control (contracts.mask_support_errors), so this
    # row is kept as the record of the measurement and not resubmitted.
    Row("c3-blur-mask-vehicles", "cosmos3-nano", "wsm", CITY_DAY,
        {"blur": "derive"}, mask_classes=["vehicle"], resolution="720", seed=7,
        skip="Cosmos 3 has no mask input: the stored run came back with the masked vehicles as "
             "black holes, and the client now refuses the request (2026-08-28)",
        shows="blur from a masked RGB: pixels-only masking, and the black hole survives into the "
              "output - the reason cosmos3 + mask_classes is now a validation error"),
    # Can the Nano restyle through a hint derived from the RGB?  Not through ``blur``: the control
    # is a bilateral-filtered, down-up-sampled copy of the capture (``make_blur_control``), so it
    # carries every colour and every luminance and the palette of the output is the palette of the
    # input whatever the prompt says.  Measured on frame 50 of showcase_wsm with the vendor kernel
    # ported to ``carla_cosmos.controls.blur_control``: mean |diff| to the RGB is 20.5/255 at the
    # default preset "medium" and only 22.7/255 at the strongest, "very_high"
    # (``_clips/showcase_wsm/checks/blur_presets.png``).  The row stays here as the record.
    Row("c3-blur-night", "cosmos3-nano", "wsm",
        "the same street at night in heavy rain, wet asphalt reflecting neon signs and headlights, "
        "photorealistic dashcam footage",
        {"blur": "derive"}, weights={"blur": 0.5}, resolution="720", seed=7, guidance=4.0,
        extra={"preset_blur_strength": "very_high", **PRESET_REGIME},
        skip="the blur hint pins the palette by construction: even the strongest preset is 22.7/255 "
             "from the RGB, so a night prompt cannot win against it (measured 2026-08-28)",
        shows="strongest blur preset + guidance 4.0 against a contradicting prompt - not run"),
    # ``edge`` is the same question asked of a control that carries no colour at all: Canny lines
    # from the RGB fix the geometry and leave the entire appearance to the prompt.  Run at the
    # vendor's own single-hint edge preset (TRANSFER_DEFAULTS["edge"]: guidance 3.0,
    # control_guidance 1.5, flow_shift 10) written out explicitly, so the record says what ran.
    Row("c3-edge-night", "cosmos3-nano", "wsm",
        "the same street at night in heavy rain, wet asphalt reflecting neon signs and headlights, "
        "photorealistic dashcam footage",
        {"edge": "derive"}, weights={"edge": 0.5}, resolution="720", seed=7, guidance=3.0,
        extra={"control_guidance": 1.5, "flow_shift": 10.0},
        shows="edge derived from the RGB (structure only, no colours) against a night+rain prompt: "
              "the restyle test the blur branch cannot pass"),
    # The RGB *complemented* by geometry rather than replaced by it: the blur hint carries the
    # captured appearance, depth and seg carry the structure, and the weights lean on the geometry
    # (normalised to 0.16 / 0.53 / 0.32).  The night prompt is deliberate -- it is the only way to
    # see how much of the palette the blur branch is still pinning once other hints pull.
    # ``preset_blur_strength`` is not named here: the client defaults a derived blur/vis hint to
    # ``carla_cosmos.client.DEFAULT_BLUR_PRESET`` ("very_high").
    Row("c3-rgb-depth-seg-night", "cosmos3-nano", "wsm",
        "the same street at night in heavy rain, wet asphalt reflecting neon signs and headlights, "
        "photorealistic dashcam footage",
        {"blur": "derive", "depth": "clip", "seg": "clip"},
        weights={"blur": 0.3, "depth": 1.0, "seg": 0.6}, resolution="720", seed=7,
        guidance=2.0, extra=dict(PRESET_REGIME),
        shows="RGB (as the blur hint) complemented by captured depth and seg, multi-hint preset "
              "regime, night+rain prompt"),
    # c3-edge-night restyled freely (new facade colours, lit brake lights) but stayed at dusk, so
    # the lighting change is what the edge preset's guidance 3.0 could not buy.  One variable:
    # guidance 6.0, and a prompt that spells the night out instead of alluding to it.
    Row("c3-edge-night-g6", "cosmos3-nano", "wsm",
        "nighttime, pitch-black sky, street lamps and shop signs lit, car headlights and tail lights "
        "on, heavy rain, wet reflective asphalt, photorealistic dashcam footage at night",
        {"edge": "derive"}, weights={"edge": 0.5}, resolution="720", seed=7, guidance=6.0,
        negative_prompt="daytime, daylight, sunny, bright sky, dry road",
        extra={"control_guidance": 1.5, "flow_shift": 10.0},
        shows="c3-edge-night at guidance 6.0 with an explicit night prompt and a daylight negative "
              "prompt: how much lighting change the Nano edge path will accept"),
    # ---- Cosmos 3 Nano tuning: undo the multi-hint guidance fallback -----------------------
    # Same clip and seed (7) as c3-wsm-depth-seg throughout; one variable added per row.
    Row("c3-tune-presets", "cosmos3-nano", "wsm", CITY_DAY,
        {"wsm": "scene", "depth": "clip", "seg": "clip"}, resolution="720", seed=7,
        guidance=2.0, extra=dict(PRESET_REGIME),
        shows="c3-wsm-depth-seg with the single-hint preset regime put back by hand "
              "(guidance 2.0 / control_guidance 2.0 / flow_shift 10) and nothing else changed"),
    Row("c3-tune-weights", "cosmos3-nano", "wsm", TUNE_PROMPT,
        {"wsm": "scene", "depth": "clip", "seg": "clip"},
        weights={"wsm": 0.4, "depth": 1.0, "seg": 0.6}, resolution="720", seed=7,
        guidance=2.0, extra=dict(PRESET_REGIME), negative_prompt=TUNE_NEGATIVE,
        shows="the preset regime plus depth-led control weights, a prompt naming what is in the "
              "frame and a negative prompt naming the haze"),
    Row("c3-tune-depthseg", "cosmos3-nano", "wsm", TUNE_PROMPT,
        {"depth": "clip", "seg": "clip"}, resolution="720", seed=7,
        guidance=3.0, extra=dict(PRESET_REGIME), negative_prompt=TUNE_NEGATIVE,
        shows="captured geometry only, no world-scenario map, at the depth/seg preset "
              "(guidance 3.0): is the haze coming from wsm?"),
    # ---- Transfer 2.5 AV multiview: seven cameras, one joint generation ---------------------
    Row("av7-day", "transfer2.5-av", "av7",
        "An urban street in the late afternoon, clear sky, dry asphalt, photorealistic footage from a "
        "seven-camera vehicle rig, consistent lighting across all cameras",
        {"hdmap_bbox": "scene"}, views=7, resolution="720",
        shows="7 cameras, hdmap+bbox rendered from the occlusion-filtered scene package"),
    Row("av7-rain", "transfer2.5-av", "av7",
        "The same urban street on a rainy evening, wet reflective asphalt, headlights and street lights "
        "reflected in the puddles, photorealistic footage from a seven-camera vehicle rig",
        {"hdmap_bbox": "scene"}, views=7, resolution="720",
        shows="same 7-camera scene, weather variation"),
    # ---- NuRec: a real place, conditioned three ways ---------------------------------------
    # One clip, captured by ``demos/nurec_to_cosmos.py`` from an NVIDIA NuRec artifact: the map,
    # the ego trajectory, the camera extrinsics and the RGB are a real drive's.  The three rows
    # differ only in what Cosmos is conditioned on -- its world model, its pixels, or both --
    # which is the comparison worth making, and it is only a fair one on one clip.
    #
    # All three are Cosmos 3 Nano.  ``nurec-wm`` was a Transfer 2.5 AV seven-view row on paper,
    # but that backend is not in the node's ``auto`` layout (Nano + Transfer 2.5 general), and
    # Cosmos 3 is in any case the only backend that takes a world-scenario control *together*
    # with an RGB-derived one, which ``nurec-both`` needs.  So the clip is captured once to the
    # Cosmos 3 ``wsm`` rule -- 202 frames at 30 fps, 101*k, 6.7 s -- with all seven views, and
    # each row submits view 0.
    #
    # ``edge``, not ``blur``, is the RGB-derived control here: the blur hint pins the palette
    # (even the strongest preset is 22.7/255 from the RGB, measured 2026-08-28 on c3-blur-night),
    # and the whole point of this trio is a real *night* street answering a daylight prompt.
    # Edge carries the structure and no colour at all, so the prompt owns the appearance.
    #
    # The prompts describe the place the reconstruction shows -- a suburban retail strip outside
    # Stockholm, recorded after dark -- as it would look in daylight.
    Row("nurec-wm", "cosmos3-nano", "nurec", NUREC_DAY,
        {"wsm": "scene"}, views=1, resolution="720", seed=7,
        guidance=1.0, extra={"control_guidance": 3.0, "flow_shift": 10.0},
        shows="the world model of a REAL drive: real road network, real lane geometry and a "
              "real vehicle's trajectory, at the single-hint wsm preset (1.0 / 3.0 / 10)"),
    Row("nurec-rgb", "cosmos3-nano", "nurec", NUREC_DAY,
        {"edge": "derive"}, views=1, resolution="720", seed=7,
        guidance=3.0, extra={"control_guidance": 1.5, "flow_shift": 10.0},
        shows="edge derived from the NEURAL RGB of a real place -- real photometry's structure, "
              "no colour -- at the single-hint edge preset (3.0 / 1.5 / 10)"),
    Row("nurec-both", "cosmos3-nano", "nurec", NUREC_DAY,
        {"wsm": "scene", "edge": "derive"}, views=1, resolution="720", seed=7,
        guidance=2.0, extra=dict(PRESET_REGIME),
        shows="both halves at once: the real drive's world map AND the structure of its neural "
              "RGB, at the multi-hint regime (2.0 / 2.0 / 10)"),
]


def check_matrix(rows: list[Row]) -> None:
    """Refuse a row without an explicit resolution (see the note above ``MATRIX``)."""
    missing = [r.id for r in rows if not r.resolution]
    if missing:
        raise SystemExit(f"rows without an explicit resolution: {missing} — a job would silently "
                         f"inherit the worker's default and its timing would not be comparable")


CLIP_ROLES = ("t25", "wsm", "av7", "nurec")
DEFAULT_CLIP_NAMES = {"t25": "showcase_t25", "wsm": "showcase_wsm", "av7": "showcase_av7",
                      # written by demos/nurec_to_cosmos.py; all three nurec rows share it
                      "nurec": "nurec_av7_30fps"}


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
    ap.add_argument("--no-viewer-video", action="store_true",
                    help="do not render each result's side-by-side viewer to viewer_<layout>.mp4")
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


def run_row(cosmos, row: Row, clip, results: str | None, log, viewer_video: bool | None = None) -> dict[str, Any]:
    """Submit one row, wait, store; returns the ledger entry (never raises for a job failure)."""
    views = None
    if row.views:
        views = clip.manifest.camera_names[:row.views]
    rec: dict[str, Any] = {"id": row.id, "backend": row.backend, "clip_id": clip.manifest.clip_id,
                           "clip_dir": str(clip.path), "prompt": row.prompt, "controls": row.controls,
                           "weights": row.weights, "mask_classes": row.mask_classes, "shows": row.shows,
                           "resolution": row.resolution, "seed": row.seed,
                           "negative_prompt": row.negative_prompt, "guidance": row.guidance,
                           "extra": dict(row.extra)}
    t0 = time.time()
    try:
        job = cosmos.submit_clip(clip, row.backend, row.prompt, dict(row.controls), views=views,
                                 weights=row.weights, mask_classes=row.mask_classes,
                                 resolution=row.resolution, seed=row.seed, extra=row.extra or None,
                                 negative_prompt=row.negative_prompt, guidance=row.guidance)
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
    stored = job.download(results, clip=clip, viewer_video=viewer_video)
    rec.update(status=info.status, worker=info.worker, directory=stored.directory,
               timings={k: round(v, 1) for k, v in stored.timings.items()},
               download_s=round(time.time() - t_dl, 1), wall_s=round(time.time() - t0, 1),
               files=len(stored.files), bytes=stored.bytes,
               videos=stored.videos,
               viewer_video=stored.viewer_video)
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
    check_matrix(rows)
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
            if row.skip and not args.force:
                rec = {"id": row.id, "backend": row.backend, "status": "skipped",
                       "error": row.skip, "shows": row.shows}
                records.append(rec)
                ledger[row.id] = rec
                ledger_path.write_text(json.dumps(ledger, indent=2))
                log.info("[%s] not submitted: %s (--force to run it anyway)", row.id, row.skip)
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
            rec = run_row(cosmos, row, loaded[row.clip], args.results, log,
                          viewer_video=False if args.no_viewer_video else None)
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
