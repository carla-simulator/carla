"""The side-by-side viewer: input RGB | control(s) | Cosmos result, per camera — and its video export.

``demos/viewer.py`` is the command line around this module.  Everything that decides what a frame
*looks like* lives in :class:`Composer`: which tiles a layout has, how they are fitted and labelled,
the footer with the status line and the prompt.  :meth:`Composer.render` returns one finished
``pygame.Surface``; the interactive loop blits it to the window and :func:`export_video` pipes it to
ffmpeg, so an exported video is pixel-identical to what the window shows at the same size — there is
no second layout implementation to drift.

Rendering never touches SDL's video subsystem (only ``pygame.font`` and plain ``Surface`` blits), so
the exporter works with no ``DISPLAY`` — under ``systemd-run``, over ssh, in a container.

Layouts:

``grid``
    Per camera the control the model saw (or the input RGB) over the result, :data:`GRID_COLUMNS`
    cameras per row.  The default for a multi-camera result.
``single``
    One camera as ``input | control | result``.  The default for a single-camera result.
"""

from __future__ import annotations

import logging
import os
import shutil
import subprocess
from dataclasses import dataclass, replace
from pathlib import Path

import numpy as np

from .clip import Clip
from .contracts import ResultManifest, canonical_camera_name

log = logging.getLogger(__name__)

GAP = 4
"""Pixels between panels."""
LINE_H = 18
"""Height of one text line in the footer (status line + wrapped prompt)."""
PROMPT_MAX_LINES = 4
"""The prompt gets up to this many wrapped lines under the panels; longer prompts end with an ellipsis."""
GRID_COLUMNS = 4
"""Cameras per row in grid mode (each camera is a column of two tiles: control/input over result)."""
PREVIEW_TITLE = "scene GT"
"""Tile title of the local ground-truth preview (already drawn over the RGB — never re-overlaid)."""
DEFAULT_HEIGHT = 360
"""Tile height (single view) / maximum tile height (grid) at the natural window size."""
VIEWER_VIDEO_PREFIX = "viewer_"
"""``viewer_grid.mp4`` / ``viewer_single.mp4``: the export written next to a stored result."""
VIEWER_KIND = "viewer"
"""``job.json`` file kind of the exported viewer video (it comes from here, not from the server)."""
ENV_VIEWER_VIDEO = "COSMOS_VIEWER_VIDEO"
"""Set to 0/no/false/off to turn the automatic export off for every download."""
ENV_CLIPS = "COSMOS_CLIPS"
"""Extra directory searched for the clip a stored result was generated from."""


# -- video ----------------------------------------------------------------------------------------

class Video:
    def __init__(self, path: Path) -> None:
        import cv2

        self.path = path
        self.cap = cv2.VideoCapture(str(path))
        self.n = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS) or 10.0
        self._idx = -1
        self._frame = None

    def frame(self, i: int) -> np.ndarray | None:
        import cv2

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


def preview_video(preview_dir: Path, camera: str) -> Path | None:
    """The local GT preview of one camera (``carla-cosmos preview``), if it was written."""
    p = preview_dir / f"{canonical_camera_name(camera)}.mp4"
    return p if p.is_file() else None


def short_name(camera: str) -> str:
    """``camera:front:wide:120fov`` -> ``front_wide_120fov`` (tile label)."""
    return canonical_camera_name(camera).removeprefix("camera_")


class CameraPanels:
    """The videos available for one camera: input RGB, controls (clip-side and server-rendered), result."""

    def __init__(self, clip: Clip, result_dir: Path | None, manifest: ResultManifest | None, camera: str,
                 preview_dir: Path | None = None) -> None:
        self.camera = camera
        self.input: Video | None = None
        self.controls: list[tuple[str, Video]] = []
        self.result: Video | None = None
        rgb = clip.manifest.video("rgb", camera)
        if rgb:
            self.input = Video(clip.path / rgb)
        gt = preview_video(preview_dir, camera) if preview_dir else None
        if gt and manifest is None:  # no result: the GT preview is what there is to look at
            self.controls.append((PREVIEW_TITLE, Video(gt)))
        for f in (manifest.files if manifest else ()):  # server-rendered/derived controls first: what the model saw
            if f.kind == "control" and f.view == camera:
                self.controls.append((f.name.removesuffix(".mp4").removesuffix("_" + canonical_camera_name(camera)),
                                      Video(result_dir / f.name)))
        for kind in ("depth", "seg", "edge"):
            name = clip.manifest.video(kind, camera)
            if name:
                self.controls.append((f"clip {kind}", Video(clip.path / name)))
        if gt and manifest is not None:
            self.controls.append((PREVIEW_TITLE, Video(gt)))
        for f in (manifest.files if manifest else ()):
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


def control_overlay(base: np.ndarray, over: np.ndarray, alpha: float = 0.85, dim: float = 0.6) -> np.ndarray:
    """Draw a control render onto the input RGB: where the control drew something (non-black) its own colours
    show at ``alpha``; elsewhere the RGB is kept, dimmed by ``dim`` so the drawing stands out.  (A plain
    screen/additive blend tints the whole frame — the control's black background is not 'nothing' to it.)"""
    import cv2

    if over.shape[:2] != base.shape[:2]:
        over = cv2.resize(over, (base.shape[1], base.shape[0]), interpolation=cv2.INTER_NEAREST)
    drawn = (over.max(axis=2) > 24)[..., None]
    out = (base.astype(np.float32) * dim)
    out = np.where(drawn, out * (1 - alpha) + over.astype(np.float32) * alpha, out)
    return out.clip(0, 255).astype(np.uint8)


# -- layout ---------------------------------------------------------------------------------------

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


# -- the compositor -------------------------------------------------------------------------------

@dataclass
class ViewState:
    """Everything the keys change — one of these plus a :class:`Composer` determines a frame exactly."""

    frame: int = 0
    cam_i: int = 0
    ctrl_i: int = 0
    grid: bool = True
    overlay: bool = False
    playing: bool = True

    @property
    def layout(self) -> str:
        return "grid" if self.grid else "single"


class Composer:
    """Draws one viewer frame.  The interactive loop and the exporter both go through :meth:`render`."""

    def __init__(self, clip: Clip, result_dir: Path | None = None, manifest: ResultManifest | None = None,
                 preview_dir: Path | None = None) -> None:
        import pygame

        self.clip = clip
        self.result_dir = result_dir
        self.manifest = manifest
        if manifest is not None:
            result_views = [f.view for f in manifest.files if f.kind == "video" and f.view]
            self.cameras = ([c for c in clip.manifest.camera_names if c in result_views]
                            or clip.manifest.camera_names[:1])
        else:  # preview mode: every camera the preview covers
            self.cameras = ([c for c in clip.manifest.camera_names if preview_dir and preview_video(preview_dir, c)]
                            or clip.manifest.camera_names)
        self.panels = {c: CameraPanels(clip, result_dir, manifest, c, preview_dir) for c in self.cameras}
        self.caption = f"{clip.manifest.clip_id} — {manifest.backend if manifest else 'scene GT preview'}"
        self.subtitle = ("prompt: " + manifest.request.prompt if manifest
                         else f"scene GT preview (no result) — {preview_dir}")
        self.save_dir = result_dir or preview_dir or clip.path
        pygame.font.init()  # fonts only: no SDL video subsystem, so this works with no DISPLAY
        self.font = pygame.font.SysFont("dejavusansmono,monospace", 14)
        any_video = next((v for cp in self.panels.values() for v in [cp.result, cp.input] if v), None)
        self.fps = any_video.fps if any_video else 10.0

    # -- what a layout is made of ---------------------------------------------------------------
    def rows(self, state: ViewState) -> list[list[tuple[str, Video, str]]]:
        """Rows of ``(title, video, camera)`` — grid: control/input over result, single: input|control|result."""
        if state.grid:
            out: list[list[tuple[str, Video, str]]] = []
            for start in range(0, len(self.cameras), GRID_COLUMNS):
                chunk = self.cameras[start:start + GRID_COLUMNS]
                tops, bottoms = [], []
                for c in chunk:
                    t = self.panels[c].top(state.ctrl_i)
                    if t:
                        tops.append((f"{short_name(c)} · {t[0]}", t[1], c))
                    if self.panels[c].result:
                        bottoms.append((f"{short_name(c)} · result", self.panels[c].result, c))
                out += [r for r in (tops, bottoms) if r]
            return out
        cam = self.cameras[state.cam_i % len(self.cameras)]
        return [[(t, v, cam) for t, v in self.panels[cam].strip(state.ctrl_i)]]

    def n_frames(self, state: ViewState) -> int:
        """Longest video of this layout — the length of the scrub bar and of an export."""
        rows = self.rows(state)
        return max((vid.n for r in rows for _, vid, _ in r), default=0)

    def frame_rows(self, state: ViewState) -> list[list[tuple[str, np.ndarray]]]:
        """The decoded tiles of one frame, with the ``o`` overlay already applied."""
        def tile(title: str, vid: Video, cam: str) -> tuple[str, np.ndarray] | None:
            f = vid.frame(state.frame)
            if f is None:
                return None
            # 'o': draw a control over the input RGB (alignment check); only for control tiles with an input
            if (state.overlay and title.split(" · ")[-1] not in ("input rgb", "result", PREVIEW_TITLE)
                    and self.panels[cam].input):
                rgb = self.panels[cam].input.frame(state.frame)
                if rgb is not None:
                    return f"{title} over rgb", control_overlay(rgb, f)
            return title, f

        rows = [[t for title, vid, cam in row if (t := tile(title, vid, cam))] for row in self.rows(state)]
        return [r for r in rows if r]

    def natural_size(self, frame_rows: list[list[tuple[str, np.ndarray]]],
                     height: int = DEFAULT_HEIGHT) -> tuple[int, int]:
        """The window size this layout wants: tiles at their own aspect plus the wrapped-prompt footer."""
        tile_h = height if len(frame_rows) == 1 else max(160, min(height, 900 // len(frame_rows)))
        total_w = max(sum(int(f.shape[1] * tile_h / f.shape[0]) for _, f in r) + GAP * (len(r) - 1)
                      for r in frame_rows)
        prompt_lines = wrap_text(self.font, self.subtitle, total_w - 12, PROMPT_MAX_LINES)
        return (total_w, tile_h * len(frame_rows) + GAP * (len(frame_rows) - 1) + footer_height(len(prompt_lines)))

    # -- the frame ------------------------------------------------------------------------------
    def render(self, state: ViewState, size: tuple[int, int] | None = None, height: int = DEFAULT_HEIGHT):
        """One finished frame as a ``pygame.Surface`` (``None`` when no tile has any pixels left).

        ``size=None`` renders at :meth:`natural_size`; the caller reads the size back off the surface.
        """
        import cv2
        import pygame

        frame_rows = self.frame_rows(state)
        if not frame_rows:
            return None
        if size is None:
            size = self.natural_size(frame_rows, height)
        prompt_lines = wrap_text(self.font, self.subtitle, size[0] - 12, PROMPT_MAX_LINES)
        tile_h = fit_rows([[f for _, f in r] for r in frame_rows], *size, footer_height(len(prompt_lines)))
        surface = pygame.Surface(size)
        surface.fill((20, 20, 20))
        y = 0
        for r in frame_rows:
            surfaces = []
            for title, f in r:
                h, w = f.shape[:2]
                f = cv2.resize(f, (max(1, int(w * tile_h / h)), tile_h))
                surf = pygame.surfarray.make_surface(np.transpose(f, (1, 0, 2)))
                surf.blit(self.font.render(title, True, (255, 255, 80), (0, 0, 0)), (6, 6))
                surfaces.append(surf)
            total_w = sum(s.get_width() for s in surfaces) + GAP * (len(surfaces) - 1)
            x = max(0, (size[0] - total_w) // 2)  # centre each row in a wider window
            for s in surfaces:
                surface.blit(s, (x, y))
                x += s.get_width() + GAP
            y += tile_h + GAP
        n = self.n_frames(state)
        mode = (f"grid {len(self.cameras)} cameras (g: single view)" if state.grid
                else f"{self.cameras[state.cam_i % len(self.cameras)]} (tab: next camera, g: grid)")
        status = (f"{mode}  frame {state.frame + 1}/{n}  {'▶' if state.playing else '❚❚'}  c: cycle control/input  "
                  f"o: overlay {'on' if state.overlay else 'off'}")
        surface.blit(self.font.render(status, True, (220, 220, 220)), (6, y))
        for i, line in enumerate(prompt_lines):  # the full prompt, word-wrapped to the window width
            surface.blit(self.font.render(line, True, (180, 180, 180)), (6, y + LINE_H * (i + 1)))
        return surface


# -- export ---------------------------------------------------------------------------------------

def ffmpeg_exe() -> str:
    """``$COSMOS_FFMPEG``, the system ffmpeg, or the one imageio-ffmpeg ships."""
    explicit = os.environ.get("COSMOS_FFMPEG")
    if explicit:
        return explicit
    found = shutil.which("ffmpeg")
    if found:
        return found
    try:
        import imageio_ffmpeg

        return imageio_ffmpeg.get_ffmpeg_exe()
    except Exception as exc:  # noqa: BLE001 - the message must say what to install
        raise RuntimeError("no ffmpeg: install ffmpeg, or the 'imageio-ffmpeg' package, "
                           "or point $COSMOS_FFMPEG at one") from exc


def even(size: tuple[int, int]) -> tuple[int, int]:
    """yuv420p needs both dimensions even; the extra pixel is background, the layout re-fits into it."""
    return (size[0] + (size[0] & 1), size[1] + (size[1] & 1))


def export_video(composer: Composer, out: str | Path, state: ViewState | None = None, fps: float | None = None,
                 size: tuple[int, int] | None = None, height: int = DEFAULT_HEIGHT,
                 crf: int = 18, progress=None) -> Path:
    """Render every frame of ``state``'s layout through :meth:`Composer.render` and encode it to ``out``.

    Frames are piped to ffmpeg as raw RGB, so the file holds exactly the pixels the viewer draws at
    ``size`` (default: the layout's natural size, rounded up to even).  No display is needed.
    """
    import pygame

    out = Path(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    state = replace(state or ViewState(), frame=0, playing=True)
    n = composer.n_frames(state)
    if n <= 0:
        raise RuntimeError(f"nothing to export: no decodable frames in {composer.save_dir}")
    first = composer.render(state, size, height)
    if first is None:
        raise RuntimeError(f"nothing to export: no tile decoded a frame in {composer.save_dir}")
    size = even(size or first.get_size())
    if first.get_size() != size:
        first = composer.render(state, size, height)
    fps = float(fps or composer.fps or 10.0)
    tmp = out.with_name(out.name + ".part.mp4")  # ffmpeg picks the muxer from the extension
    cmd = [ffmpeg_exe(), "-hide_banner", "-loglevel", "error", "-y",
           "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", f"{size[0]}x{size[1]}", "-r", f"{fps:g}", "-i", "-",
           "-an", "-map", "0:v:0", "-c:v", "libx264", "-preset", "medium", "-crf", str(crf),
           "-pix_fmt", "yuv420p", "-movflags", "+faststart", "-f", "mp4", str(tmp)]
    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        for i in range(n):
            surface = first if i == 0 else composer.render(replace(state, frame=i), size, height)
            if surface is None:
                break
            # array3d is (w, h, 3); ffmpeg wants rows of pixels
            proc.stdin.write(np.ascontiguousarray(
                np.transpose(pygame.surfarray.array3d(surface), (1, 0, 2))).tobytes())
            if progress:
                progress(i + 1, n)
        proc.stdin.close()
    except BrokenPipeError:
        pass
    finally:
        if proc.stdin and not proc.stdin.closed:
            proc.stdin.close()
    err = proc.stderr.read().decode("utf-8", "replace").strip()
    proc.stderr.close()
    if proc.wait() != 0:
        tmp.unlink(missing_ok=True)
        raise RuntimeError(f"ffmpeg failed ({proc.returncode}) writing {out}: {err or 'no message'}")
    os.replace(tmp, out)
    log.info("wrote %s (%d frames, %dx%d @ %g fps, %.1f MB)", out, n, *size, fps, out.stat().st_size / 1e6)
    return out


# -- stored results -------------------------------------------------------------------------------

def find_clip_dir(clip_id: str, result_dir: Path | None = None, results_root: Path | None = None,
                  extra: tuple[Path, ...] = ()) -> Path | None:
    """Locate the clip a stored result was generated from, or ``None``.

    Searched, in order: ``extra``, ``$COSMOS_CLIPS/<clip_id>``, then ``clips/`` and ``_clips/`` under the
    results root, next to it and in the working directory.  A candidate counts only when its
    ``manifest.json`` really is that clip.
    """
    roots: list[Path] = [Path(p) for p in extra]
    env = os.environ.get(ENV_CLIPS)
    if env:
        roots += [Path(env) / clip_id, Path(env)]
    bases = []
    if results_root is not None:
        bases.append(Path(results_root))
    if result_dir is not None:  # <root>/<clip_id>/<job_id>
        bases += [Path(result_dir).parent.parent, Path(result_dir).parent.parent.parent]
    bases.append(Path.cwd())
    for base in bases:
        roots += [base / "_clips" / clip_id, base / "clips" / clip_id,
                  base.parent / "_clips" / clip_id, base.parent / "clips" / clip_id]
    seen: set[Path] = set()
    for cand in roots:
        try:
            cand = cand.resolve()
        except OSError:
            continue
        if cand in seen:
            continue
        seen.add(cand)
        if not (cand / "manifest.json").is_file():
            continue
        try:
            if Clip.load(cand).manifest.clip_id == clip_id:
                return cand
        except (OSError, ValueError):
            continue
    return None


def viewer_video_enabled(explicit: bool | None = None) -> bool:
    """``explicit`` wins; otherwise :data:`ENV_VIEWER_VIDEO` (on unless it says otherwise)."""
    if explicit is not None:
        return explicit
    return os.environ.get(ENV_VIEWER_VIDEO, "1").strip().lower() not in ("0", "no", "false", "off", "")


def viewer_video_path(result_dir: str | Path, layout: str) -> Path:
    return Path(result_dir) / f"{VIEWER_VIDEO_PREFIX}{layout}.mp4"


def write_viewer_video(result_dir: str | Path, clip: Clip | str | Path | None = None,
                       manifest: ResultManifest | None = None, layout: str = "auto",
                       overlay: bool = False, fps: float | None = None, height: int = DEFAULT_HEIGHT,
                       results_root: str | Path | None = None, force: bool = False, progress=None) -> Path | None:
    """Write ``<result_dir>/viewer_<layout>.mp4`` — what the viewer shows, as a file.

    ``layout="auto"`` picks ``grid`` for a multi-camera result and ``single`` for one camera.  Returns
    the path (also when an up-to-date file was already there), or ``None`` when the clip the result was
    generated from cannot be found — the viewer needs its input RGB and clip-side controls.
    """
    result_dir = Path(result_dir)
    if manifest is None:
        manifest = ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
    if not isinstance(clip, Clip):
        clip_dir = Path(clip) if clip else find_clip_dir(manifest.clip.clip_id, result_dir, results_root)
        if clip_dir is None:
            log.info("no viewer video for %s: the clip %s is not next to the results root "
                     "(pass --clip / $%s)", result_dir, manifest.clip.clip_id, ENV_CLIPS)
            return None
        clip = Clip.load(clip_dir)
    composer = Composer(clip, result_dir, manifest)
    if layout == "auto":
        layout = "grid" if len(composer.cameras) > 1 else "single"
    if layout not in ("grid", "single"):
        raise ValueError(f"layout must be grid, single or auto — not {layout!r}")
    out = viewer_video_path(result_dir, layout)
    if out.is_file() and out.stat().st_size > 0 and not force:
        log.debug("keeping %s (already exported)", out)
        return out
    state = ViewState(grid=layout == "grid", overlay=overlay, playing=True)
    return export_video(composer, out, state, fps=fps, height=height, progress=progress)


# -- the interactive window -----------------------------------------------------------------------

def view_result(clip: Clip, result_dir: Path | None = None, height: int = DEFAULT_HEIGHT,
                grid: bool | None = None, preview_dir: Path | None = None) -> None:
    """Show a Cosmos result, or — with ``result_dir=None`` — the clip's local GT preview.

    Keys: space play/pause · ←/→ step · home/end · g grid/single · tab next camera · c cycle
    control/input · o overlay on/off · s save frame · v export this layout as mp4 · q quit.
    """
    import pygame

    manifest = (ResultManifest.model_validate_json((result_dir / "manifest.json").read_text())
                if result_dir else None)
    pygame.init()
    composer = Composer(clip, result_dir, manifest, preview_dir)
    state = ViewState(grid=len(composer.cameras) > 1 if grid is None else grid)
    screen = None
    win_size: tuple[int, int] | None = None  # None until the first layout / user resize
    clock = pygame.time.Clock()
    while True:
        surface = composer.render(state, win_size, height)
        if surface is None:
            break
        win_size = surface.get_size()
        if screen is None or screen.get_size() != win_size:
            screen = pygame.display.set_mode(win_size, pygame.RESIZABLE)
            pygame.display.set_caption(f"carla-cosmos — {composer.caption}")
        screen.blit(surface, (0, 0))
        pygame.display.flip()
        n = composer.n_frames(state)

        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                pygame.quit()
                return
            if ev.type == pygame.VIDEORESIZE:
                footer = footer_height(len(wrap_text(composer.font, composer.subtitle,
                                                     max(64, ev.w) - 12, PROMPT_MAX_LINES)))
                win_size = (max(64, ev.w), max(footer + 16, ev.h))
                screen = pygame.display.set_mode(win_size, pygame.RESIZABLE)
            if ev.type == pygame.KEYDOWN:
                if ev.key in (pygame.K_q, pygame.K_ESCAPE):
                    pygame.quit()
                    return
                if ev.key == pygame.K_SPACE:
                    state.playing = not state.playing
                elif ev.key == pygame.K_RIGHT:
                    state.frame, state.playing = min(n - 1, state.frame + 1), False
                elif ev.key == pygame.K_LEFT:
                    state.frame, state.playing = max(0, state.frame - 1), False
                elif ev.key == pygame.K_HOME:
                    state.frame = 0
                elif ev.key == pygame.K_END:
                    state.frame = n - 1
                elif ev.key == pygame.K_TAB:
                    state.cam_i = (state.cam_i + 1) % len(composer.cameras)
                elif ev.key == pygame.K_g:
                    state.grid, win_size = not state.grid, None  # re-layout at the natural size
                elif ev.key == pygame.K_c:
                    state.ctrl_i += 1
                elif ev.key == pygame.K_o:
                    state.overlay = not state.overlay
                elif ev.key == pygame.K_s:
                    tag = "grid" if state.grid else canonical_camera_name(
                        composer.cameras[state.cam_i % len(composer.cameras)])
                    out = composer.save_dir / f"{VIEWER_VIDEO_PREFIX}{tag}_{state.frame:04d}.png"
                    pygame.image.save(screen, str(out))
                    print(f"saved {out}")
                elif ev.key == pygame.K_v:
                    out = viewer_video_path(composer.save_dir, state.layout)
                    print(f"exporting {composer.n_frames(state)} frames to {out} …", flush=True)
                    try:
                        export_video(composer, out, state, height=height,
                                     progress=lambda i, n: print(f"\r  {i}/{n}", end="", flush=True))
                        print(f"\rwrote {out}                    ")
                    except RuntimeError as exc:
                        print(f"\rexport failed: {exc}")
        if state.playing:
            state.frame = (state.frame + 1) % n
        clock.tick(composer.fps)
