#! /usr/bin/env python3
"""Build the comparison videos for a :mod:`showcase` run.

Reads ``<results>/showcase.json`` (written by ``demos/showcase.py``) and, for
every row that finished, produces

* ``sheet_<row>.mp4`` — ``input | control(s) | output`` side by side, one tile
  per stream, each tile labelled;
* ``grid_<row>.mp4`` — for a multi-view row, the generated cameras in a grid
  laid out the way the rig sits on the car;
* ``modes_reel.mp4`` — every labelled output back to back, one clip that walks
  through the whole matrix.

Labels are burned in with PIL and overlaid as PNGs: ffmpeg's ``drawtext``
segfaults together with ``xstack`` on this host, and every filter graph here
maps its output explicitly (``-map "[v]"``) because ffmpeg otherwise feeds the
first input to every output.

    python demos/showcase_sheets.py            # $COSMOS_RESULTS -> $COSMOS_RESULTS/_showcase
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw, ImageFont

FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
LABEL_H = 34
TILE_W = 640
# where each AV camera sits in the 3-column grid
AV_GRID = [
    ["camera_cross_left_120fov", "camera_front_wide_120fov", "camera_cross_right_120fov"],
    ["camera_rear_left_70fov", "camera_front_tele_30fov", "camera_rear_right_70fov"],
    ["", "camera_rear_tele_30fov", ""],
]


def run(cmd: list[str]) -> None:
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        raise SystemExit(f"ffmpeg failed ({proc.returncode}):\n  {' '.join(cmd[:12])} ...\n{proc.stderr[-2000:]}")


def probe(path: Path) -> dict[str, Any]:
    out = subprocess.run(["ffprobe", "-v", "error", "-select_streams", "v:0", "-show_entries",
                          "stream=width,height,nb_frames,r_frame_rate", "-of", "json", str(path)],
                         capture_output=True, text=True)
    if out.returncode != 0:
        raise SystemExit(f"ffprobe failed on {path}: {out.stderr[-500:]}")
    s = json.loads(out.stdout)["streams"][0]
    num, _, den = s["r_frame_rate"].partition("/")
    return {"w": int(s["width"]), "h": int(s["height"]),
            "fps": float(num) / float(den or 1), "frames": int(s.get("nb_frames") or 0)}


def label_png(text: str, sub: str, width: int, dest: Path) -> Path:
    """Dark caption strip with ``text`` in bold and ``sub`` in grey, ``width`` x ``LABEL_H``."""
    img = Image.new("RGBA", (width, LABEL_H), (16, 18, 22, 235))
    draw = ImageDraw.Draw(img)
    bold = ImageFont.truetype(FONT_BOLD, 19)
    small = ImageFont.truetype(FONT, 16)
    draw.text((10, 7), text, font=bold, fill=(255, 255, 255, 255))
    if sub:
        x = 14 + draw.textlength(text, font=bold)
        room = width - x - 10
        while sub and draw.textlength(sub, font=small) > room:
            sub = sub[:-2]
        draw.text((x, 9), sub, font=small, fill=(165, 172, 185, 255))
    dest.parent.mkdir(parents=True, exist_ok=True)
    img.save(dest)
    return dest


def tiled(tiles: list[tuple[Path, str, str]], dest: Path, work: Path, cols: int | None = None,
          tile_w: int = TILE_W, fps: float = 16.0) -> Path:
    """``xstack`` the given ``(video, label, sublabel)`` tiles into ``dest``.

    Every tile is scaled to ``tile_w`` at the aspect of the first one and gets
    its caption strip on top; the stack stops with the shortest input.
    """
    ref = probe(tiles[0][0])
    tile_h = int(round(tile_w * ref["h"] / ref["w"])) // 2 * 2  # yuv420p needs even sizes
    cols = cols or len(tiles)
    cell_h = tile_h + LABEL_H

    inputs: list[str] = []
    labels: list[str] = []
    graph: list[str] = []
    for i, (video, text, sub) in enumerate(tiles):
        png = label_png(text, sub, tile_w, work / f"{dest.stem}_lab{i}.png")
        inputs += ["-i", str(video)]
        labels += ["-i", str(png)]
        graph.append(f"[{i}:v]fps={fps},scale={tile_w}:{tile_h},setsar=1,"
                     f"pad={tile_w}:{cell_h}:0:{LABEL_H}:color=0x101216[p{i}]")
        graph.append(f"[p{i}][{len(tiles) + i}:v]overlay=0:0[t{i}]")
    inputs += labels

    layout = "|".join(f"{(i % cols) * tile_w}_{(i // cols) * cell_h}" for i in range(len(tiles)))
    stack = "".join(f"[t{i}]" for i in range(len(tiles)))
    if len(tiles) == 1:
        graph.append("[t0]copy[v]")
    else:
        graph.append(f"{stack}xstack=inputs={len(tiles)}:layout={layout}:fill=0x101216:shortest=1[v]")
    dest.parent.mkdir(parents=True, exist_ok=True)
    run(["ffmpeg", "-v", "error", "-y", *inputs, "-filter_complex", ";".join(graph),
         "-map", "[v]", "-c:v", "libx264", "-preset", "medium", "-crf", "20", "-pix_fmt", "yuv420p",
         "-r", str(fps), str(dest)])
    return dest


def clip_video(clip_dir: Path, kind: str, view: str) -> Path | None:
    p = clip_dir / f"{kind}_{view}.mp4"
    return p if p.exists() else None


def sheet_for(rec: dict[str, Any], out_dir: Path, work: Path) -> Path | None:
    """``input | control(s) | output`` for one row (the first view of a multi-view row)."""
    result = Path(rec["directory"])
    clip_dir = Path(rec["clip_dir"])
    videos = sorted(v for v in rec.get("videos", []) if not v.startswith(("control_", "grid")))
    if not videos:
        return None
    hero = "camera_front_wide_120fov.mp4"          # the sheet shows the forward view of a rig
    if hero in videos:
        videos = [hero] + [v for v in videos if v != hero]
    view = videos[0][:-4]
    tiles: list[tuple[Path, str, str]] = []
    rgb = clip_video(clip_dir, "rgb", view)
    if rgb:
        tiles.append((rgb, "CARLA input", f"{rec['clip_id']} · {view}"))
    for hint in rec.get("controls", {}):
        served = result / f"control_{hint}_{view}.mp4"
        if not served.exists():
            served = result / f"control_{hint}.mp4"
        if served.exists():
            tiles.append((served, f"control {hint}", "rendered by the server"))
            continue
        local = clip_video(clip_dir, hint, view)
        weight = (rec.get("weights") or {}).get(hint)
        if local:
            # the server did not return this control; show the clip's own as a stand-in and say so
            how = rec.get("controls", {}).get(hint)
            source = "derived server-side · clip's own shown" if how == "derive" else "from the clip"
            tiles.append((local, f"control {hint}", f"{source}{f' · w={weight:g}' if weight else ''}"))
    tiles.append((result / videos[0], "Cosmos output", rec["id"]))
    fps = probe(result / videos[0])["fps"]
    return tiled(tiles, out_dir / f"sheet_{rec['id']}.mp4", work, cols=len(tiles), fps=fps)


def grid_for(rec: dict[str, Any], out_dir: Path, work: Path) -> Path | None:
    """The generated cameras of a multi-view row, laid out like the rig."""
    result = Path(rec["directory"])
    have = {v[:-4] for v in rec.get("videos", []) if not v.startswith(("control_", "grid"))}
    if len(have) < 2:
        return None
    tiles: list[tuple[Path, str, str]] = []
    for row in AV_GRID:
        for cam in row:
            if cam and cam in have:
                tiles.append((result / f"{cam}.mp4", cam.replace("camera_", ""), ""))
            else:
                tiles.append((work / "blank.mp4", "", ""))
    ref = probe(result / f"{sorted(have)[0]}.mp4")
    make_blank(work / "blank.mp4", ref)
    return tiled(tiles, out_dir / f"grid_{rec['id']}.mp4", work, cols=3, tile_w=480, fps=ref["fps"])


def make_blank(dest: Path, ref: dict[str, Any]) -> Path:
    if dest.exists():
        return dest
    dest.parent.mkdir(parents=True, exist_ok=True)
    seconds = max(1.0, (ref["frames"] or 60) / max(ref["fps"], 1.0))
    run(["ffmpeg", "-v", "error", "-y", "-f", "lavfi", "-i",
         f"color=c=0x101216:s={ref['w']}x{ref['h']}:r={ref['fps']:.4f}:d={seconds:.2f}",
         "-c:v", "libx264", "-pix_fmt", "yuv420p", str(dest)])
    return dest


def reel(records: list[dict[str, Any]], out_dir: Path, work: Path, fps: float = 16.0) -> Path | None:
    """Every output back to back, each labelled with its row and what it shows."""
    parts: list[Path] = []
    for rec in records:
        videos = sorted(v for v in rec.get("videos", []) if not v.startswith(("control_", "grid")))
        if not videos:
            continue
        src = Path(rec["directory"]) / videos[0]
        part = work / f"reel_{rec['id']}.mp4"
        tiled([(src, rec["id"], rec.get("shows", ""))], part, work, cols=1, tile_w=960, fps=fps)
        parts.append(part)
    if not parts:
        return None
    listing = work / "reel.txt"
    listing.write_text("".join(f"file '{p}'\n" for p in parts))
    dest = out_dir / "modes_reel.mp4"
    run(["ffmpeg", "-v", "error", "-y", "-f", "concat", "-safe", "0", "-i", str(listing),
         "-c:v", "libx264", "-preset", "medium", "-crf", "20", "-pix_fmt", "yuv420p", str(dest)])
    return dest


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", default=None, help="results root (default $COSMOS_RESULTS or ./cosmos-results)")
    ap.add_argument("--ledger", default=None, help="showcase.json (default <results>/showcase.json)")
    ap.add_argument("--out", default=None,
                    help="where the sheets are written (default <results root>/_showcase)")
    ap.add_argument("--only", default=None, help="comma-separated row ids")
    ap.add_argument("--no-reel", action="store_true")
    args = ap.parse_args(argv)

    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "client"))
    from carla_cosmos.results import default_results_root  # noqa: E402

    root = default_results_root(args.results)
    ledger = Path(args.ledger) if args.ledger else root / "showcase.json"
    if not ledger.exists():
        raise SystemExit(f"{ledger} does not exist — run demos/showcase.py first")
    records = [r for r in json.loads(ledger.read_text()).values() if r.get("status") == "done"]
    if args.only:
        wanted = {w.strip() for w in args.only.split(",")}
        records = [r for r in records if r["id"] in wanted]
    if not records:
        raise SystemExit(f"no finished row in {ledger}")

    out_dir = Path(args.out) if args.out else root / "_showcase"
    work = out_dir / "work"
    work.mkdir(parents=True, exist_ok=True)
    made: list[Path] = []
    for rec in records:
        sheet = sheet_for(rec, out_dir, work)
        if sheet:
            made.append(sheet)
            print(f"sheet {rec['id']:<22} {sheet}")
        grid = grid_for(rec, out_dir, work)
        if grid:
            made.append(grid)
            print(f"grid  {rec['id']:<22} {grid}")
    if not args.no_reel:
        r = reel(records, out_dir, work)
        if r:
            made.append(r)
            print(f"reel  {'all rows':<22} {r}")
    print(f"\n{len(made)} file(s) in {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
