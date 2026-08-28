#! /usr/bin/env python3
"""Python API, Transfer 2.5 AV over the seven NVIDIA cameras with an hdmap+bbox scene.

    python api_av_multiview.py --clip ./clips/av7_town10_30fps [--views 3] [--no-view]

The clip must come from the ``nvidia_av7`` rig at 30 fps with a scene package
(``av7_world_scenario.py --capture-only`` or ``api_capture_to_job.py --rig
nvidia_av7``).  The scene is uploaded once and rendered server-side into the
``hdmap_bbox`` control for every view; the rendered controls come back next to
the generated videos (``control_hdmap_bbox_<camera>.mp4``) together with the
3x3 ``grid.mp4``, and everything is stored under the results root.  The viewer
is then opened on the result as a per-camera grid.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from api_common import (  # puts ../client on sys.path
    connect,
    load_clip,
    results_root,
    setup_logging,
    wait_and_store,
)

from carla_cosmos.client import CosmosError, JobFailed  # noqa: E402

BACKEND = "transfer2.5-av"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", required=True, help="7-camera 30 fps clip directory with scene/")
    ap.add_argument("--views", type=int, default=7, help="how many of the clip's cameras to send (1..7)")
    ap.add_argument("--prompt", default="An urban street in the late afternoon, clear sky, photorealistic "
                                        "footage from a car-mounted camera rig")
    ap.add_argument("--weight", type=float, default=None)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("--no-view", action="store_true", help="do not open demos/viewer.py on the result")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    log = setup_logging("api_av_multiview", args.verbose)

    cosmos = connect(args.endpoint, args.token, backend=BACKEND)
    clip = load_clip(args.clip)
    if clip.scene_dir is None or not clip.scene_dir.exists():
        raise SystemExit(f"{clip.path} has no scene package; 'hdmap_bbox' is rendered from it")
    views = clip.manifest.camera_names[:max(1, min(args.views, 7))]
    if not cosmos.models()[BACKEND].scene_rendering:
        raise SystemExit(f"{cosmos.url} cannot render scene packages (no world-scenario renderer in its profile)")
    log.info("clip %s: %d frames @ %d fps, sending %d view(s)", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, len(views))
    log.info("results will be stored under %s", results_root(args.results))

    control = ("scene", args.weight) if args.weight is not None else "scene"
    try:
        job = cosmos.submit_clip(clip, BACKEND, args.prompt, {"hdmap_bbox": control}, views=views, seed=args.seed)
        log.info("job %s queued (position %s)", job.id, job.info.queue_position)
        stored = wait_and_store(job, args.results, log)
    except JobFailed as exc:
        log.error("the job did not finish: %s", exc)
        return 2
    except CosmosError as exc:
        log.error("%s", exc)
        return 2

    controls = [f.name for f in stored.files if f.kind == "control"]
    print(f"\n{len(stored.videos)} video(s) in {stored.directory}")
    print(f"  rendered controls the model saw: {controls or 'none returned by this worker'}")
    print(f"  contact sheet: {stored.path / 'grid.mp4'}" if "grid.mp4" in stored.videos else "  (no grid.mp4)")
    if args.no_view:
        return 0
    viewer = Path(__file__).with_name("viewer.py")
    log.info("opening the viewer grid (q to quit)")
    return subprocess.call([sys.executable, str(viewer), "--clip", str(clip.path),
                            "--result", stored.directory, "--grid"])


if __name__ == "__main__":
    sys.exit(main())
