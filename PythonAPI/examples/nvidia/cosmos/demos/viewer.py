"""Side-by-side viewer: input RGB | control(s) | Cosmos result, per camera, with scrubbing.

    python viewer.py --clip clips/<id> --result results/<id>/<job>      # or: viewer.py --result DIR (clip from manifest)

Keys: space play/pause · ←/→ step · home/end · tab next camera · c cycle control · s save frame · q quit.
The window is resizable; the panels re-fit side by side keeping their aspect ratio.
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


def load_panels(clip: Clip, result_dir: Path, camera: str) -> list[tuple[str, Video]]:
    panels: list[tuple[str, Video]] = []
    rgb = clip.manifest.video("rgb", camera)
    if rgb:
        panels.append(("input rgb", Video(clip.path / rgb)))
    for kind in ("depth", "seg", "edge"):
        name = clip.manifest.video(kind, camera)
        if name:
            panels.append((f"control {kind}", Video(clip.path / name)))
    manifest = ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
    for f in manifest.files:
        if f.kind == "video" and (f.view == camera or f.view is None):
            panels.append((f"{manifest.backend} result", Video(result_dir / f.name)))
        elif f.kind == "control" and f.view == camera:
            panels.append((f"server {f.name}", Video(result_dir / f.name)))
    return panels


GAP = 4
"""Pixels between panels."""
STATUS_H = 24
"""Height of the status line under the panels."""


def fit_panels(frames: list[np.ndarray], win_w: int, win_h: int) -> int:
    """Largest panel height at which all ``frames`` fit side by side (aspect kept) in the window."""
    aspects = sum(f.shape[1] / f.shape[0] for f in frames)
    by_width = (win_w - GAP * (len(frames) - 1)) / aspects if aspects else win_h
    return max(16, int(min(win_h - STATUS_H, by_width)))


def view_result(clip: Clip, result_dir: Path, height: int = 360) -> None:
    import pygame

    cameras = clip.manifest.camera_names
    cam_i, ctrl_i, frame, playing = 0, 0, 0, True
    panels = load_panels(clip, result_dir, cameras[cam_i])
    manifest = ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
    pygame.init()
    font = pygame.font.SysFont("dejavusansmono,monospace", 14)
    screen = None
    win_size: tuple[int, int] | None = None  # None until the first layout / user resize
    clock = pygame.time.Clock()
    fps = panels[0][1].fps if panels else 10
    while True:
        # visible panels: input, one control (cycled), result(s)
        inputs = [p for p in panels if p[0].startswith("input")]
        controls = [p for p in panels if p[0].startswith("control")]
        results = [p for p in panels if not p[0].startswith(("input", "control"))]
        shown = inputs + ([controls[ctrl_i % len(controls)]] if controls else []) + results
        frames = [(title, f) for title, vid in shown if (f := vid.frame(frame)) is not None]
        if not frames:
            break
        if win_size is None:
            # natural size: --height tall, panels side by side
            total_w = sum(int(f.shape[1] * height / f.shape[0]) for _, f in frames) + GAP * (len(frames) - 1)
            win_size = (total_w, height + STATUS_H)
        if screen is None:
            screen = pygame.display.set_mode(win_size, pygame.RESIZABLE)
            pygame.display.set_caption(f"carla-cosmos — {clip.manifest.clip_id} — {manifest.backend}")
        panel_h = fit_panels([f for _, f in frames], *win_size)
        surfaces = []
        for title, f in frames:
            h, w = f.shape[:2]
            f = cv2.resize(f, (max(1, int(w * panel_h / h)), panel_h))
            surf = pygame.surfarray.make_surface(np.transpose(f, (1, 0, 2)))
            surf.blit(font.render(title, True, (255, 255, 255), (0, 0, 0)), (6, 6))
            surfaces.append(surf)
        screen.fill((20, 20, 20))
        total_w = sum(s.get_width() for s in surfaces) + GAP * (len(surfaces) - 1)
        x = max(0, (win_size[0] - total_w) // 2)  # centre the strip in a wider window
        for s in surfaces:
            screen.blit(s, (x, 0))
            x += s.get_width() + GAP
        n = max(v.n for _, v in shown)
        status = (f"{cameras[cam_i]}  frame {frame + 1}/{n}  {'▶' if playing else '❚❚'}  "
                  f"prompt: {manifest.request.prompt[:90]}")
        screen.blit(font.render(status, True, (220, 220, 220)), (6, panel_h + 4))
        pygame.display.flip()

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                pygame.quit()
                return
            if ev.type == pygame.VIDEORESIZE:
                win_size = (max(64, ev.w), max(STATUS_H + 16, ev.h))
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
                    panels = load_panels(clip, result_dir, cameras[cam_i])
                elif ev.key == pygame.K_c:
                    ctrl_i += 1
                elif ev.key == pygame.K_s:
                    out = result_dir / f"viewer_{canonical_camera_name(cameras[cam_i])}_{frame:04d}.png"
                    pygame.image.save(screen, str(out))
                    print(f"saved {out}")
        if playing:
            frame = (frame + 1) % n
        clock.tick(fps)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--result", required=True, help="downloaded result directory (with manifest.json)")
    ap.add_argument("--clip", default=None, help="clip directory (default: <result>/../../<clip_id> or manifest)")
    ap.add_argument("--height", type=int, default=360)
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
    view_result(clip, result_dir, args.height)
    return 0


if __name__ == "__main__":
    sys.exit(main())
