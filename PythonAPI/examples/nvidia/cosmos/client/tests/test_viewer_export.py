"""The viewer's video export (``carla_cosmos.viewer``) — headless, on a tiny synthetic result.

The point of the export is that it is *the viewer*, serialised: same compositor, same tiles, same
labels.  So the tests decode the produced mp4 with OpenCV and compare it against what
:meth:`Composer.render` draws for the same frame, and they run with ``DISPLAY`` removed from the
environment so a regression that needs a window fails here instead of in a systemd unit.
"""

from __future__ import annotations

import hashlib
import shutil

import numpy as np
import pytest

from carla_cosmos.contracts import JobRequest, ResultFile, ResultManifest
from carla_cosmos.synthetic import make_clip

pygame = pytest.importorskip("pygame")
cv2 = pytest.importorskip("cv2")

from carla_cosmos.viewer import (  # noqa: E402
    Composer,
    ViewState,
    even,
    export_video,
    find_clip_dir,
    viewer_video_enabled,
    write_viewer_video,
)

FRAMES = 6
CAM = "camera:front:wide:120fov"
AV2 = ["camera:front:wide:120fov", "camera:cross:left:120fov"]


@pytest.fixture(autouse=True)
def no_display(monkeypatch):
    """Every test here runs as the exporter does under systemd-run: no display of any kind."""
    for var in ("DISPLAY", "WAYLAND_DISPLAY"):
        monkeypatch.delenv(var, raising=False)
    monkeypatch.delenv("COSMOS_VIEWER_VIDEO", raising=False)
    monkeypatch.delenv("COSMOS_CLIPS", raising=False)


def build_result(root, cameras=(CAM,), frames=FRAMES, fps=8):
    """A results-store-shaped job directory: a clip in ``_clips/`` and a result next to it.

    The 'generated' videos and the rendered control are copies of the clip's own AOVs — the export
    does not care what is in them, only that every panel decodes.
    """
    clip = make_clip(root / "_clips", frames=frames, fps=fps, cameras=list(cameras),
                     width=64, height=48, kinds=("rgb", "depth", "seg"))
    dest = root / clip.manifest.clip_id / "j_test0001"
    dest.mkdir(parents=True)
    files = []
    for cam in cameras:
        src = clip.path / clip.manifest.video("rgb", cam)
        for name, kind in ((clip.manifest.video("rgb", cam).removeprefix("rgb_"), "video"),
                           ("control_depth_" + clip.manifest.video("rgb", cam).removeprefix("rgb_"), "control")):
            shutil.copyfile(src, dest / name)
            body = (dest / name).read_bytes()
            files.append(ResultFile(name=name, size=len(body), sha256=hashlib.sha256(body).hexdigest(),
                                    view=cam, kind=kind))
    manifest = ResultManifest(
        job_id="j_test0001", backend="transfer2.5", worker="w0", clip=clip.manifest, files=files,
        request=JobRequest(backend="transfer2.5", prompt="a synthetic street, " + "long prompt " * 12,
                           seed=1, views=list(cameras)))
    (dest / "manifest.json").write_text(manifest.model_dump_json(indent=2))
    return clip, dest, manifest


def decode(path):
    """Every frame of ``path`` as RGB arrays."""
    cap = cv2.VideoCapture(str(path))
    out = []
    while True:
        ok, f = cap.read()
        if not ok:
            break
        out.append(cv2.cvtColor(f, cv2.COLOR_BGR2RGB))
    cap.release()
    return out


def test_export_is_what_the_compositor_draws(tmp_path):
    """Frame count, size and pixels: the file is the viewer's own frames, not a re-implementation."""
    clip, dest, manifest = build_result(tmp_path)
    composer = Composer(clip, dest, manifest)
    state = ViewState(grid=False)
    size = even(composer.render(state).get_size())

    out = export_video(composer, tmp_path / "strip.mp4", state, crf=0)  # lossless: compare pixel-wise
    frames = decode(out)

    assert len(frames) == FRAMES == composer.n_frames(state)
    assert frames[0].shape[:2] == (size[1], size[0])
    assert size[0] % 2 == 0 and size[1] % 2 == 0, "yuv420p needs even dimensions"
    for i in (0, FRAMES // 2, FRAMES - 1):
        drawn = np.transpose(pygame.surfarray.array3d(
            composer.render(ViewState(grid=False, frame=i), size)), (1, 0, 2))
        diff = np.abs(drawn.astype(np.int16) - frames[i].astype(np.int16))
        assert diff.mean() < 2.0, f"frame {i} differs from the compositor by {diff.mean():.2f} on average"
    assert not pygame.display.get_init(), "the export must not touch the display subsystem"


def test_export_fps_and_frame_rate(tmp_path):
    """``--export-fps`` overrides; the default is the result's own frame rate."""
    clip, dest, manifest = build_result(tmp_path, fps=8)
    composer = Composer(clip, dest, manifest)
    assert composer.fps == pytest.approx(8.0)

    out = export_video(composer, tmp_path / "fast.mp4", ViewState(grid=False), fps=24)
    cap = cv2.VideoCapture(str(out))
    assert cap.get(cv2.CAP_PROP_FPS) == pytest.approx(24.0, abs=0.1)
    cap.release()


def test_write_viewer_video_picks_the_layout(tmp_path):
    """``auto``: the strip for one camera, the grid for several — named after the layout."""
    clip, dest, manifest = build_result(tmp_path)
    single = write_viewer_video(dest, clip=clip, manifest=manifest)
    assert single.name == "viewer_single.mp4" and single.stat().st_size > 0
    assert write_viewer_video(dest, clip=clip, manifest=manifest) == single  # idempotent, no re-encode

    clip2, dest2, manifest2 = build_result(tmp_path / "multi", cameras=AV2)
    grid = write_viewer_video(dest2, clip=clip2, manifest=manifest2)
    assert grid.name == "viewer_grid.mp4"
    frames = decode(grid)
    assert len(frames) == FRAMES
    # the grid stacks a control row over a result row, so it is taller than the two-tile strip
    assert frames[0].shape[0] > decode(single)[0].shape[0]


def test_find_clip_dir_next_to_the_results_root(tmp_path):
    """A stored result finds its clip in ``<results root>/_clips/<clip_id>`` without being told."""
    clip, dest, manifest = build_result(tmp_path)
    assert find_clip_dir(clip.manifest.clip_id, dest, tmp_path) == clip.path.resolve()
    assert find_clip_dir("no_such_clip", dest, tmp_path) is None


def test_store_records_the_viewer_video(tmp_path):
    """``ResultStore`` writes it and hands back a ``StoredFile`` of kind 'viewer' for job.json."""
    from carla_cosmos.results import ResultStore

    clip, dest, manifest = build_result(tmp_path)
    rec = ResultStore(tmp_path).write_viewer_video(dest, manifest)
    assert rec is not None and rec.kind == "viewer" and rec.name == "viewer_single.mp4"
    assert rec.size == (dest / rec.name).stat().st_size and len(rec.sha256) == 64


def test_viewer_video_can_be_turned_off(tmp_path, monkeypatch):
    """``viewer_video=False`` wins; otherwise ``COSMOS_VIEWER_VIDEO`` decides, on by default."""
    from carla_cosmos.results import ResultStore

    assert viewer_video_enabled(None) is True
    monkeypatch.setenv("COSMOS_VIEWER_VIDEO", "0")
    assert viewer_video_enabled(None) is False
    assert viewer_video_enabled(True) is True

    clip, dest, manifest = build_result(tmp_path)
    assert ResultStore(tmp_path).write_viewer_video(dest, manifest, enabled=False) is None
    assert not (dest / "viewer_single.mp4").exists()


def test_a_missing_clip_never_fails_the_download(tmp_path):
    """No clip on disk: the videos are still stored, there is simply no viewer video."""
    from carla_cosmos.results import ResultStore

    clip, dest, manifest = build_result(tmp_path)
    shutil.rmtree(clip.path)
    assert ResultStore(tmp_path).write_viewer_video(dest, manifest) is None
