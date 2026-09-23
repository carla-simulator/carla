"""Side-by-side viewer: input RGB | control(s) | Cosmos result, per camera, with scrubbing — and mp4 export.

    python viewer.py --clip clips/<id> --result results/<id>/<job>      # or: viewer.py --result DIR (clip from manifest)
    python viewer.py --clip clips/<id>                                  # no result yet: the local GT preview
    python viewer.py --result DIR --export out.mp4                      # no window, no DISPLAY needed

Multi-camera results (Transfer 2.5 AV) open as a grid: per camera the control the model saw (or the input
RGB) over the result, GRID_COLUMNS cameras per row.  Single-camera results open as input | control | result.

Without a result directory the viewer opens the clip's local ground-truth preview instead (written by
``carla-cosmos preview --clip ...`` into ``<clip>/preview``, or ``--preview DIR``): the exported ClipGT
scene drawn on the captured RGB stands in as the "control" stream, so the same grid/scrub/save keys work
before anything has been generated.

``--export FILE`` writes the whole layout as an mp4 instead of opening a window — the exporter runs the
same compositor the window does, so the file holds exactly the frames the viewer draws.  It implies
``--headless``: no display is touched, so it runs over ssh or under ``systemd-run``.  Every downloaded
result also gets one automatically (``viewer_grid.mp4`` / ``viewer_single.mp4`` next to its videos).

Keys: space play/pause · ←/→ step · home/end · g grid/single · tab next camera (single) · c cycle
control/input · o control over input RGB on/off · s save frame · v export this layout as mp4 · q quit.
The window is resizable; tiles re-fit keeping their aspect ratio.
Needs the ``viewer`` extra (pygame) and OpenCV (``capture`` extra).
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

from carla_cosmos import Clip
from carla_cosmos.contracts import ResultManifest
from carla_cosmos.viewer import (  # noqa: F401  (re-exported: demos and tests import them from here)
    DEFAULT_HEIGHT,
    GAP,
    GRID_COLUMNS,
    LINE_H,
    PREVIEW_TITLE,
    PROMPT_MAX_LINES,
    CameraPanels,
    Composer,
    Video,
    ViewState,
    control_overlay,
    export_video,
    fit_panels,
    fit_rows,
    footer_height,
    preview_video,
    short_name,
    view_result,
    viewer_video_path,
    wrap_text,
)


def export(clip: Clip, result_dir: Path | None, preview_dir: Path | None, args) -> int:
    """Render the requested layout(s) to mp4 without opening a window."""
    os.environ.setdefault("SDL_VIDEODRIVER", "dummy")  # belt and braces: nothing here opens a window
    manifest = (ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
                if result_dir else None)
    composer = Composer(clip, result_dir, manifest, preview_dir)
    auto = ("grid" if (len(composer.cameras) > 1 if args.grid is None else args.grid) else "single")
    layouts = (["grid", "single"] if args.export_layout == "both"
               else [args.export_layout] if args.export_layout in ("grid", "single") else [auto])
    out = Path(args.export)
    for layout in layouts:
        dest = out if len(layouts) == 1 else out.with_name(f"{out.stem}_{layout}{out.suffix}")
        state = ViewState(grid=layout == "grid", cam_i=0, overlay=args.export_overlay == "on")
        n = composer.n_frames(state)
        print(f"exporting {n} frame(s) of the {layout} layout to {dest} …", flush=True)
        export_video(composer, dest, state, fps=args.export_fps, height=args.height,
                     progress=lambda i, n: print(f"\r  {i}/{n}", end="", flush=True))
        print(f"\rwrote {dest} ({dest.stat().st_size / 1e6:.1f} MB)          ")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--result", default=None, help="downloaded result directory (with manifest.json)")
    ap.add_argument("--clip", default=None, help="clip directory (default: <result>/../../<clip_id> or manifest)")
    ap.add_argument("--preview", default=None,
                    help="local GT preview directory (default <clip>/preview); shown when there is no result")
    ap.add_argument("--height", type=int, default=DEFAULT_HEIGHT,
                    help="tile height (single view) / max tile height (grid)")
    ap.add_argument("--grid", dest="grid", action="store_true", default=None, help="force the per-camera grid")
    ap.add_argument("--single", dest="grid", action="store_false", help="force the single-camera strip")
    ap.add_argument("--export", default=None, metavar="OUT.MP4",
                    help="write the layout to an mp4 instead of opening a window (implies --headless)")
    ap.add_argument("--export-fps", type=float, default=None,
                    help="frame rate of the export (default: the result's own fps)")
    ap.add_argument("--export-layout", choices=["grid", "single", "both", "auto"], default="auto",
                    help="auto: grid for a multi-camera result, single otherwise; both writes two files")
    ap.add_argument("--export-overlay", choices=["on", "off"], default="off",
                    help="draw the control over the input RGB in the export (the 'o' key)")
    ap.add_argument("--headless", action="store_true",
                    help="never touch a display (implied by --export)")
    args = ap.parse_args()
    if not args.result and not args.clip:
        print("pass --result (a downloaded result) or --clip (its local GT preview)", file=sys.stderr)
        return 1
    if args.headless and not args.export:
        print("--headless only makes sense with --export", file=sys.stderr)
        return 1
    result_dir = Path(args.result) if args.result else None
    if args.clip:
        clip = Clip.load(args.clip)
    else:
        m = json.loads((result_dir / "manifest.json").read_text())
        candidates = [result_dir.parent.parent.parent / "clips" / m["clip"]["clip_id"],
                      result_dir.parent.parent / "_clips" / m["clip"]["clip_id"], result_dir.parent]
        found = next((c for c in candidates if (c / "manifest.json").exists()), None)
        if found is None:
            print("pass --clip: could not locate the clip directory", file=sys.stderr)
            return 1
        clip = Clip.load(found)
    preview_dir = Path(args.preview) if args.preview else clip.path / "preview"
    if not preview_dir.is_dir():
        if result_dir is None:
            print(f"no preview in {preview_dir}: run  carla-cosmos preview --clip {clip.path} --grid",
                  file=sys.stderr)
            return 1
        preview_dir = None
    if args.export:
        return export(clip, result_dir, preview_dir, args)
    view_result(clip, result_dir, args.height, args.grid, preview_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
