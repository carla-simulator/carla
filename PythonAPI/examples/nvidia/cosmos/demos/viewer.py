"""Side-by-side viewer: input RGB | control(s) | Cosmos result, per camera, with scrubbing.

    python viewer.py --clip clips/<id> --result results/<id>/<job>      # or: viewer.py --result DIR (clip from manifest)

Multi-camera results (Transfer 2.5 AV) open as a grid: per camera the control the model saw (or the input
RGB) over the result, GRID_COLUMNS cameras per row.  Single-camera results open as input | control | result.

Keys: space play/pause · ←/→ step · home/end · g grid/single · tab next camera (single) · c cycle
control/input · o control over input RGB on/off · s save frame · q quit.  The window is resizable; tiles re-fit keeping their aspect ratio.
Needs the ``viewer`` extra (pygame) and OpenCV (``capture`` extra).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import cv2
import numpy as np

from carla_cosmos import Clip
from carla_cosmos.contracts import ResultManifest, canonical_camera_name


class Video:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.cap = cv2.VideoCapture(str(path))
        self.n = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS) or 10.0
        self._idx = -1
        self._frame = None

    def frame(self, i: int) -> np.ndarray | None:
        if self.n == 0:
            return None
        i = max(0, min(self.n - 1, i))
        if i != self._idx + 1:
            self.cap.set(cv2.CAP_PROP_POS_FRAMES, i)
        ok, f = self.cap.read()
        if not ok:
            return self._frame
        self._idx = i
        self._frame = cv2.cvtColor(f, cv2.COLOR_BGR2RGB)
        return self._frame


def short_name(camera: str) -> str:
    """``camera:front:wide:120fov`` -> ``front_wide_120fov`` (tile label)."""
    return canonical_camera_name(camera).removeprefix("camera_")


class CameraPanels:
    """The videos available for one camera: input RGB, controls (clip-side and server-rendered), result."""

    def __init__(self, clip: Clip, result_dir: Path, manifest: ResultManifest, camera: str) -> None:
        self.camera = camera
        self.input: Video | None = None
        self.controls: list[tuple[str, Video]] = []
        self.result: Video | None = None
        rgb = clip.manifest.video("rgb", camera)
        if rgb:
            self.input = Video(clip.path / rgb)
        for f in manifest.files:  # server-rendered/derived controls first: they are what the model saw
            if f.kind == "control" and f.view == camera:
                self.controls.append((f.name.removesuffix(".mp4").removesuffix("_" + canonical_camera_name(camera)),
                                      Video(result_dir / f.name)))
        for kind in ("depth", "seg", "edge"):
            name = clip.manifest.video(kind, camera)
            if name:
                self.controls.append((f"clip {kind}", Video(clip.path / name)))
        for f in manifest.files:
            if f.kind == "video" and f.view == camera:
                self.result = Video(result_dir / f.name)

    def top(self, ctrl_i: int) -> tuple[str, Video] | None:
        """What the grid shows above the result: controls cycled with ``c``, then the input RGB."""
        choices = self.controls + ([("input rgb", self.input)] if self.input else [])
        return choices[ctrl_i % len(choices)] if choices else None

    def strip(self, ctrl_i: int) -> list[tuple[str, Video]]:
        """Single-camera row: input | one control | result."""
        row = [("input rgb", self.input)] if self.input else []
        if self.controls:
            row.append(self.controls[ctrl_i % len(self.controls)])
        if self.result:
            row.append(("result", self.result))
        return row


def screen_blend(base: np.ndarray, over: np.ndarray) -> np.ndarray:
    """``over`` (a control render) screen-blended onto ``base`` (the input RGB), resized to ``base``."""
    if over.shape[:2] != base.shape[:2]:
        over = cv2.resize(over, (base.shape[1], base.shape[0]))
    a, b = base.astype(np.uint16), over.astype(np.uint16)
    return (255 - ((255 - a) * (255 - b)) // 255).astype(np.uint8)


GAP = 4
"""Pixels between panels."""
LINE_H = 18
"""Height of one text line in the footer (status line + wrapped prompt)."""
PROMPT_MAX_LINES = 4
"""The prompt gets up to this many wrapped lines under the panels; longer prompts end with an ellipsis."""
GRID_COLUMNS = 4
"""Cameras per row in grid mode (each camera is a column of two tiles: control/input over result)."""


def wrap_text(font, text: str, width: int, max_lines: int) -> list[str]:
    """Word-wrap ``text`` to ``width`` pixels with ``font``; at most ``max_lines`` (last one ellipsised)."""
    lines: list[str] = []
    line = ""
    for word in text.split():
        cand = f"{line} {word}".strip()
        if font.size(cand)[0] <= width or not line:
            line = cand
        else:
            lines.append(line)
            line = word
    if line:
        lines.append(line)
    if len(lines) > max_lines:
        lines = lines[:max_lines]
        while lines[-1] and font.size(lines[-1] + " …")[0] > width:
            lines[-1] = lines[-1].rsplit(" ", 1)[0] if " " in lines[-1] else lines[-1][:-1]
        lines[-1] += " …"
    return lines


def footer_height(prompt_lines: int) -> int:
    """Footer = one status line + the wrapped prompt lines, with padding."""
    return LINE_H * (1 + prompt_lines) + 8


def fit_rows(rows: list[list[np.ndarray]], win_w: int, win_h: int, footer_h: int = LINE_H + 8) -> int:
    """Largest tile height at which every row fits the width and all rows fit above the footer (aspect kept)."""
    by_h = (win_h - footer_h - GAP * (len(rows) - 1)) / max(1, len(rows))
    by_w = [(win_w - GAP * (len(r) - 1)) / sum(f.shape[1] / f.shape[0] for f in r) for r in rows if r]
    return max(16, int(min([by_h] + by_w)))


def fit_panels(frames: list[np.ndarray], win_w: int, win_h: int, footer_h: int = LINE_H + 8) -> int:
    """Single-row form of :func:`fit_rows` (kept for callers/tests)."""
    return fit_rows([frames], win_w, win_h, footer_h)


def view_result(clip: Clip, result_dir: Path, height: int = 360, grid: bool | None = None) -> None:
    import pygame

    manifest = ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
    result_views = [f.view for f in manifest.files if f.kind == "video" and f.view]
    cameras = [c for c in clip.manifest.camera_names if c in result_views] or clip.manifest.camera_names[:1]
    panels = {c: CameraPanels(clip, result_dir, manifest, c) for c in cameras}
    grid = len(cameras) > 1 if grid is None else grid
    cam_i, ctrl_i, frame, playing, overlay = 0, 0, 0, True, False
    pygame.init()
    font = pygame.font.SysFont("dejavusansmono,monospace", 14)
    screen = None
    win_size: tuple[int, int] | None = None  # None until the first layout / user resize
    clock = pygame.time.Clock()
    any_video = next((v for cp in panels.values() for v in [cp.result, cp.input] if v), None)
    fps = any_video.fps if any_video else 10
    while True:
        # rows of (title, Video): grid = per camera a control/input tile over the result tile, GRID_COLUMNS
        # cameras per row; single = input | control | result for the current camera
        if grid:
            rows: list[list[tuple[str, Video]]] = []
            for start in range(0, len(cameras), GRID_COLUMNS):
                chunk = cameras[start:start + GRID_COLUMNS]
                tops, bottoms = [], []
                for c in chunk:
                    t = panels[c].top(ctrl_i)
                    if t:
                        tops.append((f"{short_name(c)} · {t[0]}", t[1], c))
                    if panels[c].result:
                        bottoms.append((f"{short_name(c)} · result", panels[c].result, c))
                rows += [r for r in (tops, bottoms) if r]
        else:
            rows = [[(t, v, cameras[cam_i]) for t, v in panels[cameras[cam_i]].strip(ctrl_i)]]
        def tile(title: str, vid: Video, cam: str) -> tuple[str, np.ndarray] | None:
            f = vid.frame(frame)
            if f is None:
                return None
            # 'o': draw a control over the input RGB (alignment check); only for control tiles with an input
            if overlay and title.split(" · ")[-1] not in ("input rgb", "result") and panels[cam].input:
                rgb = panels[cam].input.frame(frame)
                if rgb is not None:
                    return f"{title} over rgb", screen_blend(rgb, f)
            return title, f

        frame_rows = [[t for title, vid, cam in row if (t := tile(title, vid, cam))] for row in rows]
        frame_rows = [r for r in frame_rows if r]
        if not frame_rows:
            break
        if win_size is None:
            tile_h = height if len(frame_rows) == 1 else max(160, min(height, 900 // len(frame_rows)))
            total_w = max(sum(int(f.shape[1] * tile_h / f.shape[0]) for _, f in r) + GAP * (len(r) - 1) for r in frame_rows)
            prompt_lines = wrap_text(font, "prompt: " + manifest.request.prompt, total_w - 12, PROMPT_MAX_LINES)
            win_size = (total_w, tile_h * len(frame_rows) + GAP * (len(frame_rows) - 1) + footer_height(len(prompt_lines)))
        if screen is None:
            screen = pygame.display.set_mode(win_size, pygame.RESIZABLE)
            pygame.display.set_caption(f"carla-cosmos — {clip.manifest.clip_id} — {manifest.backend}")
        prompt_lines = wrap_text(font, "prompt: " + manifest.request.prompt, win_size[0] - 12, PROMPT_MAX_LINES)
        tile_h = fit_rows([[f for _, f in r] for r in frame_rows], *win_size, footer_height(len(prompt_lines)))
        screen.fill((20, 20, 20))
        y = 0
        for r in frame_rows:
            surfaces = []
            for title, f in r:
                h, w = f.shape[:2]
                f = cv2.resize(f, (max(1, int(w * tile_h / h)), tile_h))
                surf = pygame.surfarray.make_surface(np.transpose(f, (1, 0, 2)))
                surf.blit(font.render(title, True, (255, 255, 80), (0, 0, 0)), (6, 6))
                surfaces.append(surf)
            total_w = sum(s.get_width() for s in surfaces) + GAP * (len(surfaces) - 1)
            x = max(0, (win_size[0] - total_w) // 2)  # centre each row in a wider window
            for s in surfaces:
                screen.blit(s, (x, y))
                x += s.get_width() + GAP
            y += tile_h + GAP
        n = max(vid.n for r in rows for _, vid, _ in r)
        mode = f"grid {len(cameras)} cameras (g: single view)" if grid else f"{cameras[cam_i]} (tab: next camera, g: grid)"
        status = (f"{mode}  frame {frame + 1}/{n}  {'▶' if playing else '❚❚'}  c: cycle control/input  "
                  f"o: overlay {'on' if overlay else 'off'}")
        screen.blit(font.render(status, True, (220, 220, 220)), (6, y))
        for i, line in enumerate(prompt_lines):  # the full prompt, word-wrapped to the window width
            screen.blit(font.render(line, True, (180, 180, 180)), (6, y + LINE_H * (i + 1)))
        pygame.display.flip()

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                pygame.quit()
                return
            if ev.type == pygame.VIDEORESIZE:
                win_size = (max(64, ev.w), max(footer_height(len(prompt_lines)) + 16, ev.h))
                screen = pygame.display.set_mode(win_size, pygame.RESIZABLE)
            if ev.type == pygame.KEYDOWN:
                if ev.key in (pygame.K_q, pygame.K_ESCAPE):
                    pygame.quit()
                    return
                if ev.key == pygame.K_SPACE:
                    playing = not playing
                elif ev.key == pygame.K_RIGHT:
                    frame, playing = min(n - 1, frame + 1), False
                elif ev.key == pygame.K_LEFT:
                    frame, playing = max(0, frame - 1), False
                elif ev.key == pygame.K_HOME:
                    frame = 0
                elif ev.key == pygame.K_END:
                    frame = n - 1
                elif ev.key == pygame.K_TAB:
                    cam_i = (cam_i + 1) % len(cameras)
                elif ev.key == pygame.K_g:
                    grid, win_size = not grid, None  # re-layout at the natural size of the new mode
                elif ev.key == pygame.K_c:
                    ctrl_i += 1
                elif ev.key == pygame.K_o:
                    overlay = not overlay
                elif ev.key == pygame.K_s:
                    tag = "grid" if grid else canonical_camera_name(cameras[cam_i])
                    out = result_dir / f"viewer_{tag}_{frame:04d}.png"
                    pygame.image.save(screen, str(out))
                    print(f"saved {out}")
        if playing:
            frame = (frame + 1) % n
        clock.tick(fps)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--result", required=True, help="downloaded result directory (with manifest.json)")
    ap.add_argument("--clip", default=None, help="clip directory (default: <result>/../../<clip_id> or manifest)")
    ap.add_argument("--height", type=int, default=360, help="tile height (single view) / max tile height (grid)")
    ap.add_argument("--grid", dest="grid", action="store_true", default=None, help="force the per-camera grid")
    ap.add_argument("--single", dest="grid", action="store_false", help="force the single-camera strip")
    args = ap.parse_args()
    result_dir = Path(args.result)
    if args.clip:
        clip = Clip.load(args.clip)
    else:
        m = json.loads((result_dir / "manifest.json").read_text())
        candidates = [result_dir.parent.parent.parent / "clips" / m["clip"]["clip_id"], result_dir.parent]
        found = next((c for c in candidates if (c / "manifest.json").exists()), None)
        if found is None:
            print("pass --clip: could not locate the clip directory", file=sys.stderr)
            return 1
        clip = Clip.load(found)
    view_result(clip, result_dir, args.height, args.grid)
    return 0


if __name__ == "__main__":
    sys.exit(main())
