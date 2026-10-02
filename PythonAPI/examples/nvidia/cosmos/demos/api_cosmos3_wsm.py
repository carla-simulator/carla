#! /usr/bin/env python3
"""Python API, single-view Cosmos 3 with the world-scenario (``wsm``) control.

    python api_cosmos3_wsm.py --clip ./clips/carla_town10_1700000000 \\
        --prompt "heavy rain, headlights on" [--backend cosmos3-super] [--results ./cosmos-results]

The clip's ClipGT scene package (``<clip>/scene``) is uploaded once and NVIDIA's
world-scenario renderer draws the control server-side, so the clip must carry a
scene package and run at 10, 15 or 30 fps (the renderer produces 30 and
decimates).  Cosmos 3 also wants 101*k frames at its fps.  The rendered control
comes back with the result as ``control_wsm.mp4`` — look at it when the output
does not follow the scene.
"""

from __future__ import annotations

import argparse
import sys

from api_common import (  # puts ../client on sys.path
    connect,
    load_clip,
    results_root,
    setup_logging,
    wait_and_store,
)

from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", required=True, help="clip directory with a scene/ package (10, 15 or 30 fps)")
    ap.add_argument("--backend", default="cosmos3-nano", choices=["cosmos3-nano", "cosmos3-super"])
    ap.add_argument("--prompt", default="A rainy evening in the city, headlights on, wet asphalt, "
                                        "photorealistic dashcam footage")
    ap.add_argument("--weight", type=float, default=None, help="control weight (backend default when omitted)")
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--steps", type=int, default=None)
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--no-viewer-video", action="store_true",
                    help="do not render the result's side-by-side viewer to viewer_<layout>.mp4")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    log = setup_logging("api_cosmos3_wsm", args.verbose)

    cosmos = connect(args.endpoint, args.token, backend=args.backend)
    clip = load_clip(args.clip)
    if clip.scene_dir is None or not clip.scene_dir.exists():
        raise SystemExit(f"{clip.path} has no scene package — capture it with "
                         f"single_view_live.py / api_capture_to_job.py, which export scene/ by default")
    if clip.manifest.fps not in (10, 15, 30):
        raise SystemExit(f"clip is {clip.manifest.fps} fps; a server-rendered wsm control needs 10, 15 or 30")
    view = clip.manifest.camera_names[0]
    if not cosmos.models()[args.backend].scene_rendering:
        raise SystemExit(f"{cosmos.url} has no world-scenario renderer in its profile — 'wsm' cannot be rendered "
                         f"there; send a pre-rendered control video instead ({{'wsm': 'clip'}})")
    log.info("clip %s (%d frames @ %d fps), view %s, scene %s", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, view, clip.scene_dir)
    log.info("results will be stored under %s", results_root(args.results))

    control = ("scene", args.weight) if args.weight is not None else "scene"
    try:
        job = cosmos.submit_clip(clip, args.backend, args.prompt, {"wsm": control}, views=[view],
                                 seed=args.seed, num_steps=args.steps)
        log.info("job %s queued on %s (position %s)", job.id, args.backend, job.info.queue_position)
        stored = wait_and_store(job, args.results, log, clip=clip,
                                viewer_video=False if args.no_viewer_video else None)
    except JobFailed as exc:
        log.error("the job did not finish: %s", exc)
        return 2
    except CosmosError as exc:
        log.error("%s", exc)
        return 2

    rendered = [n for n in stored.videos if n.startswith("control_")]
    print(f"\ngenerated video(s): {[str(stored.path / n) for n in stored.videos if n not in rendered]}")
    print(f"world-scenario control the model saw: {[str(stored.path / n) for n in rendered]}")
    print(f"everything, with the request and checksums: {stored.path / 'job.json'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
