#! /usr/bin/env python3
"""Python API, Transfer 2.5 on a clip that is already on disk.

    python api_transfer25_basic.py --clip ./clips/carla_town10_1700000000 \\
        --prompt "the same street at night, wet asphalt" [--results ./cosmos-results]

Sends the clip's own depth and segmentation control videos and asks the server
to derive the edge control from the RGB (which is uploaded for that reason).
Nothing is shelled out to the CLI: this is the ``carla_cosmos`` package.
Connection: ``--endpoint``/``--token`` or ``COSMOS_URL`` / ``COSMOS_TOKEN``.
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

BACKEND = "transfer2.5"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", required=True, help="clip directory written by a capture demo")
    ap.add_argument("--prompt", default="The same street at night after rain, wet asphalt reflecting the "
                                        "street lights, photorealistic dashcam footage")
    ap.add_argument("--negative-prompt", default=None)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--resolution", default=None, help="720 or 480 (backend default when omitted)")
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--no-viewer-video", action="store_true",
                    help="do not render the result's side-by-side viewer to viewer_<layout>.mp4")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    log = setup_logging("api_transfer25", args.verbose)

    cosmos = connect(args.endpoint, args.token, backend=BACKEND)
    clip = load_clip(args.clip)
    log.info("clip %s: %d frames @ %d fps, cameras %s", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, ", ".join(clip.manifest.camera_names))
    log.info("results will be stored under %s", results_root(args.results))

    controls = {"depth": "clip", "seg": "clip", "edge": "derive"}
    try:
        job = cosmos.submit_clip(clip, BACKEND, args.prompt, controls, seed=args.seed,
                                 negative_prompt=args.negative_prompt, resolution=args.resolution)
        log.info("job %s queued (position %s); controls %s", job.id, job.info.queue_position, list(controls))
        stored = wait_and_store(job, args.results, log, clip=clip,
                                viewer_video=False if args.no_viewer_video else None)
    except JobFailed as exc:
        log.error("the job did not finish: %s", exc)
        return 2
    except CosmosError as exc:
        log.error("%s", exc)
        return 2

    print(f"\ngenerated videos in {stored.directory}:")
    for name in stored.videos:
        print(f"  {stored.path / name}")
    print(f"request, timings and checksums: {stored.path / 'job.json'}")
    print(f"play them side by side with the input: python demos/viewer.py "
          f"--clip {clip.path} --result {stored.directory}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
