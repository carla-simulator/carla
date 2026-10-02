#!/usr/bin/env python3
"""Condition Cosmos on a NuRec scene: by the world map, by the neural RGB, or by both.

A NuRec artifact is twenty seconds of real driving reconstructed as a Gaussian splat.  This
demo turns one into a Cosmos clip package and submits it three different ways:

``--mode wm``
    **The world model of a real drive.**  The sample's own OpenDRIVE map and ego trajectory
    become the ClipGT scene package, and Cosmos Transfer 2.5 AV renders its ``hdmap_bbox``
    control from it across seven views.  No neural rendering is involved at all — the value is
    that the roads, the lanes and the drive are a real place's, not a CARLA town's.

``--mode rgb``
    **The photometry of a real place.**  The clip's RGB *is* the neural render, so the controls
    Cosmos Transfer 2.5 derives from it (``vis``, ``edge``) carry real-world structure and
    lighting instead of CARLA's renderer, with CARLA's depth and segmentation alongside.

``--mode both``
    Cosmos 3 is the one backend that accepts a world-scenario control and RGB-derived controls
    together (``wsm`` + ``edge`` + ``blur``), so ``both`` goes there.  Transfer 2.5 AV accepts
    ``hdmap_bbox`` and nothing else, which is why ``both`` is not simply ``wm`` with extras.

Each mode needs its own clip: the three backends disagree about frame rate and clip length
(87 frames at 30 fps, 93 at 16, 101 at 10), and a clip is captured for exactly one contract.

The traffic
-----------
``--actors artifact`` (the default) imports the drive's own recorded traffic out of the
sample's ``clipgt/obstacle.parquet`` into the clip's ClipGT obstacle layer, so the ``wsm`` /
``hdmap_bbox`` control carries the cars the neural RGB actually shows.  ``--actors carla``
exports the proxy world's own actors instead - which is nothing unless ``--vehicles`` spawned
some, and anything it spawns is invisible to the render engine.  CARLA's depth and
segmentation AOVs describe the proxy world either way and contain none of the recorded cars.

Without a render engine
-----------------------
``--fake-nurec`` substitutes CARLA's own RGB for the neural render and captures everything else
unchanged — the real map, the real trajectory, the real extrinsics, the real ClipGT export.
It is how the whole pipeline is exercised on a machine that runs no model inference, and it is
a genuinely useful mode for ``--mode wm``, which never wanted neural pixels in the first place.

Examples
--------
::

    # Everything but the model, here, no GPU node, no render engine:
    python nurec_to_cosmos.py --sample <uuid>.usdz --mode wm --fake-nurec --capture-only

    # The real thing, engine on the node, Cosmos on the node:
    export COSMOS_URL=http://cosmos-dev2:8000
    python nurec_to_cosmos.py --sample <uuid>.usdz --mode both --nre-endpoint cosmos-dev2:46435
"""

from __future__ import annotations

import argparse
import logging
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "client"))

import carla  # noqa: E402

from carla_cosmos import nurec  # noqa: E402
from carla_cosmos.client import CosmosClient  # noqa: E402
from carla_cosmos.clip import Clip  # noqa: E402
from carla_cosmos.contracts import BUILTIN_CONTRACTS  # noqa: E402

log = logging.getLogger("nurec_to_cosmos")

DEFAULT_PROMPT = (
    "A photorealistic dashcam drive through a northern European suburb on an overcast "
    "afternoon, wet asphalt, bare trees along the roadside, natural diffuse daylight.")


class Mode:
    """One conditioning mode: the backend it maps to and the clip that backend needs."""

    def __init__(self, name: str, backend: str, controls: dict[str, str], fps: int,
                 frames_for: callable, views: int, lens: str, shows: str) -> None:
        self.name = name
        self.backend = backend
        self.controls = controls
        self.fps = fps
        self.frames_for = frames_for
        self.views = views
        self.lens = lens
        self.shows = shows


#: ``frames_for(k)`` is each contract's own frame rule, spelled out at the clip's fps:
#: AV takes ``29 + 28*(k-1)`` at its model rate of 10 and the clip runs at 30, hence the x3;
#: Transfer 2.5 general takes ``93*k`` at 16; Cosmos 3 with ``wsm`` takes ``101*k`` at 10.
MODES = {
    "wm": Mode("wm", "transfer2.5-av", {"hdmap_bbox": "scene"}, 30,
               lambda k: 3 * (29 + 28 * (k - 1)), 7, "pinhole",
               "the real drive's map and trajectory as the AV world-scenario control"),
    "rgb": Mode("rgb", "transfer2.5", {"vis": "derive", "edge": "derive", "depth": "clip",
                                       "seg": "clip"}, 16,
                lambda k: 93 * k, 1, "ftheta",
                "controls derived from the neural render of a real place"),
    "both": Mode("both", "cosmos3-nano", {"wsm": "scene", "edge": "derive", "blur": "derive"},
                 10, lambda k: 101 * k, 1, "ftheta",
                 "world-scenario map and RGB-derived controls together (Cosmos 3)"),
}


def resolve_sample(spec: str) -> Path:
    """Accept a path to a ``.usdz`` or a bare scene uuid under ``$NUREC_SAMPLES``."""
    path = Path(spec)
    if path.is_file():
        return path
    root = Path(os.environ.get("NUREC_SAMPLES", "nurec_samples")) / "sample_set/26.04_release"
    candidate = root / spec / f"{spec}.usdz"
    if candidate.is_file():
        return candidate
    matches = sorted(root.glob(f"{spec}*/*.usdz"))
    if len(matches) == 1:
        return matches[0]
    raise SystemExit(f"no NuRec artifact for '{spec}' (looked at {path} and under {root})")


def resolve_rig(spec: str | None, mode: Mode) -> tuple[list[str] | None, bool]:
    """``(cameras, complete_av7)`` for ``--cameras``.

    ``av7`` is the seven-view AV rig (the sample's six calibrated cameras plus the nominal
    ``camera:rear:tele:30fov``); ``sample`` is every camera the artifact calibrates; ``front``
    is the front wide alone; anything else is a comma-separated list of underscore names.
    The default is the rig the mode's own backend needs.
    """
    if spec is None:
        spec = "av7" if mode.views == 7 else "front"
    if spec == "av7":
        return None, True
    if spec == "sample":
        return None, False
    if spec == "front":
        return ["camera_front_wide_120fov"], False
    return [c.strip() for c in spec.split(",") if c.strip()], False


def capture(args, mode: Mode) -> tuple[Clip, nurec.Alignment]:
    """Capture the clip this mode's backend needs."""
    sample = nurec.NurecSample.load(resolve_sample(args.sample))
    lens = "pinhole" if args.fake_nurec else mode.lens
    fps = args.fps or mode.fps
    frames = args.frames or mode.frames_for(args.k)
    clip_id = args.clip_id or f"nurec_{sample.scene_id[-8:]}_{mode.name}_{fps}fps"
    contract = BUILTIN_CONTRACTS[args.backend or mode.backend]
    cameras, complete_av7 = resolve_rig(args.cameras, mode)

    log.info("%s: %d frames at %d fps (%s), %d view(s), lens=%s, rgb=%s, actors=%s", mode.name,
             frames, fps, contract.id, mode.views, lens,
             "CARLA (fake-nurec)" if args.fake_nurec else f"NRE @ {args.nre_endpoint}", args.actors)

    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    return nurec.capture(
        client, sample, args.out, clip_id,
        frames=frames, fps=fps, lens=lens, cameras=cameras,
        complete_av7=complete_av7, nre_endpoint=args.nre_endpoint,
        fake_nurec=args.fake_nurec, start_s=args.start, vehicles=args.vehicles,
        seed=args.seed, contract=contract, visibility=args.visibility,
        weather=args.weather, actors=args.actors,
        force_shared_server=args.force_shared_server,
        restore_map=not args.keep_map,
        progress=lambda i, n: (print(f"\r  {i}/{n} frames", end="", flush=True),
                               print() if i == n else None))


def submit(args, mode: Mode, clip: Clip) -> None:
    """Prepare and (when an endpoint is configured) submit this mode's job."""
    contract = BUILTIN_CONTRACTS[args.backend or mode.backend]
    views = clip.manifest.camera_names[:mode.views]
    log.info("request: backend=%s controls=%s views=%d resolution=%s",
             contract.id, mode.controls, len(views), args.resolution)
    if not args.endpoint:
        print(f"\nprepared but not submitted (no COSMOS_URL / --endpoint).  To run it:\n"
              f"  carla-cosmos submit --clip {clip.path} --backend {contract.id} \\\n"
              f"    " + " ".join(f"--control {k}={v}" for k, v in mode.controls.items())
              + f" \\\n    --resolution {args.resolution} --prompt {args.prompt!r} --wait")
        return
    with CosmosClient(args.endpoint, token=args.token) as cosmos:
        job = cosmos.submit_clip(clip, contract.id, args.prompt, dict(mode.controls),
                                 views=views, resolution=args.resolution, seed=args.seed)
        print(f"submitted {job.id}")
        if args.wait:
            job.wait(poll=3.0, on_progress=lambda i: log.info("  %s", i.status))
            stored = job.download(clip=clip)
            print(f"stored in {stored.directory}")


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--sample", required=True, help="path to a NuRec .usdz, or a scene uuid")
    p.add_argument("--mode", default="wm", choices=sorted(MODES),
                   help="conditioning mode (default: wm)")
    p.add_argument("--backend", help="override the backend this mode maps to")
    p.add_argument("--fake-nurec", action="store_true",
                   help="substitute CARLA RGB for the neural render (no engine needed)")
    p.add_argument("--nre-endpoint", default=os.environ.get("NUREC_ENDPOINT"),
                   help="NuRec Render Engine as host:port ($NUREC_ENDPOINT)")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, default=2000)
    p.add_argument("--timeout", type=float, default=120.0)
    p.add_argument("--k", type=int, default=1, help="clip-length multiple for the frame rule")
    p.add_argument("--fps", type=int, help="override the clip rate (must suit the backend)")
    p.add_argument("--cameras", help="rig to capture: av7 | sample | front | a comma-separated "
                                     "list of underscore camera names (default: what the mode's "
                                     "backend needs)")
    p.add_argument("--frames", type=int, help="override the frame count (must suit the backend)")
    p.add_argument("--start", type=float, default=0.0, help="seconds into the recording")
    p.add_argument("--vehicles", type=int, default=0,
                   help="CARLA traffic to add; visible in the scene package and the AOVs but "
                        "NOT in the neural RGB (the engine only re-poses its own tracks)")
    p.add_argument("--actors", default="artifact", choices=sorted(nurec.ACTOR_SOURCES),
                   help="where the ClipGT obstacle layer comes from: 'artifact' (default) imports "
                        "the traffic the reconstruction recorded, from the sample's own "
                        "clipgt/obstacle.parquet, so the world-scenario control describes the "
                        "cars the neural RGB actually shows; 'carla' exports only the proxy "
                        "world's own actors, which is empty unless --vehicles spawned some")
    p.add_argument("--visibility", default="depth", choices=("depth", "none"))
    p.add_argument("--weather", default="ClearNoon",
                   help="carla.WeatherParameters preset; the generated OpenDRIVE world "
                        "comes up with a 10-degree sun, which is too dark to derive from")
    p.add_argument("--clip-id")
    p.add_argument("--out", default="cosmos-results/_clips")
    p.add_argument("--resolution", default="720", help="720 only, for anything Cosmos will see")
    p.add_argument("--prompt", default=DEFAULT_PROMPT)
    p.add_argument("--seed", type=int, default=7)
    p.add_argument("--force-shared-server", action="store_true",
                   help="capture even though the server is in synchronous mode; only for a stale "
                        "flag left by a crashed run — while a real capture holds it, loading this "
                        "sample's map destroys that capture's ego and sensors")
    p.add_argument("--keep-map", action="store_true",
                   help="leave the NuRec map loaded instead of restoring the previous one")
    p.add_argument("--capture-only", action="store_true")
    p.add_argument("--wait", action="store_true", help="wait for the job and download it")
    p.add_argument("--endpoint", default=os.environ.get("COSMOS_URL"))
    p.add_argument("--token", default=os.environ.get("COSMOS_TOKEN"))
    p.add_argument("-v", "--verbose", action="store_true")
    args = p.parse_args(argv)

    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO,
                        format="%(levelname)s %(name)s: %(message)s")
    if args.resolution != "720":
        log.warning("resolution %s: everything meant for Cosmos is captured at 720p",
                    args.resolution)
    mode = MODES[args.mode]
    clip, alignment = capture(args, mode)
    print(f"\nclip: {clip.path}")
    print(f"  {clip.manifest.frames} frames, {len(clip.manifest.camera_names)} cameras, "
          f"map {clip.manifest.map}")
    print(f"  alignment: {alignment.max_translation_m * 1000:.2f} mm / "
          f"{alignment.max_rotation_deg:.4f} deg over {alignment.frames} frames "
          f"({'ok' if alignment.ok else 'FAILED'})")
    problems = clip.validate()
    if problems:
        raise SystemExit("clip is not well formed: " + "; ".join(problems))
    if args.capture_only:
        print(f"\npreview it locally with:\n  carla-cosmos preview --clip {clip.path} --grid")
        return 0
    submit(args, mode, clip)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
