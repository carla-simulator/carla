#! /usr/bin/env python3
"""Python API, Transfer 2.5 with mask-out classes and per-control weights.

    python api_masking_and_weights.py --clip ./clips/carla_town10hd_semantic \\
        --mask-classes vehicle --weights depth=1.0,seg=0.5,edge=0.3

``mask_classes`` removes CARLA semantic classes from every input derived from
the captured pixels — the depth/seg/edge control videos and the RGB video that
is uploaded — so the model has no evidence there and may hallucinate whatever
the prompt asks for.  It never touches the world-scenario controls (``wsm``,
``hdmap_bbox``): those are geometric GT rendered from the scene package.
``weights`` says how hard each control branch pulls.

The clip must carry class information: a ``semantic_<camera>.mp4`` AOV, or a
``seg`` video captured with ``seg_mode="semantic"`` (``api_capture_to_job.py
--semantic``, or ``Capture(seg_mode="semantic")``).  An instance-coloured
``seg`` video holds instance ids only and is refused with an actionable
message.  ``carla-cosmos classes`` lists the class names.

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


def parse_weights(spec: str) -> dict[str, float]:
    """``"depth=1.0,seg=0.5"`` -> ``{"depth": 1.0, "seg": 0.5}``."""
    out: dict[str, float] = {}
    for item in (p for p in spec.split(",") if p.strip()):
        name, _, value = item.partition("=")
        try:
            out[name.strip()] = float(value)
        except ValueError:
            raise SystemExit(f"--weights: '{item}' is not NAME=NUMBER") from None
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", required=True, help="clip directory with a semantic (CityScapes) seg AOV")
    ap.add_argument("--prompt", default="An empty city street at golden hour, wet asphalt reflecting the low "
                                        "sun, photorealistic dashcam footage")
    ap.add_argument("--mask-classes", default="vehicle",
                    help="classes to remove, comma separated (carla-cosmos classes lists them); "
                         "empty string submits unmasked")
    ap.add_argument("--mask-dilate", type=int, default=None,
                    help="grow the mask by this many pixels so outlines do not leak (default 3)")
    ap.add_argument("--weights", default="depth=1.0,seg=0.5,edge=0.3", metavar="NAME=W[,NAME=W...]")
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--resolution", default=None, help="720 or 480 (backend default when omitted)")
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--endpoint", default=None)
    ap.add_argument("--token", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    log = setup_logging("api_masking", args.verbose)

    cosmos = connect(args.endpoint, args.token, backend=BACKEND)
    clip = load_clip(args.clip)
    mask_classes = [c for c in args.mask_classes.split(",") if c.strip()] or None
    weights = parse_weights(args.weights)
    log.info("clip %s: %d frames @ %d fps, videos %s", clip.manifest.clip_id, clip.manifest.frames,
             clip.manifest.fps, sorted({k.split("/")[0] for k in clip.manifest.videos}))
    log.info("masking %s (dilate %s px), weights %s", mask_classes or "nothing",
             args.mask_dilate if args.mask_dilate is not None else "default", weights)
    log.info("results will be stored under %s", results_root(args.results))

    controls = {"depth": "clip", "seg": "clip", "edge": "derive"}
    try:
        # The masked control videos are built and uploaded here; 'edge' is derived by the server
        # from the RGB, which is uploaded masked, so the hole is in the derived control as well.
        job = cosmos.submit_clip(clip, BACKEND, args.prompt, controls, weights=weights,
                                 mask_classes=mask_classes, mask_dilate=args.mask_dilate,
                                 seed=args.seed, resolution=args.resolution)
        log.info("job %s queued (position %s)", job.id, job.info.queue_position)
        stored = wait_and_store(job, args.results, log)
    except JobFailed as exc:
        log.error("the job did not finish: %s", exc)
        return 2
    except CosmosError as exc:
        log.error("%s", exc)
        return 2

    request = job.result().manifest.request
    print(f"\nreproducible from the result manifest: mask_classes={request.mask_classes} "
          f"dilate={request.mask_dilate} weights="
          f"{ {n: c.weight for n, c in request.controls.items()} }")
    print(f"generated videos in {stored.directory}:")
    for name in stored.videos:
        print(f"  {stored.path / name}")
    print(f"play them side by side with the input: python demos/viewer.py "
          f"--clip {clip.path} --result {stored.directory}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
