"""CosmosClient / Job / Result against a real mock server (uvicorn subprocess).

Needs the ``carla-cosmos-server`` package importable (installed from
``../server``); skipped otherwise.
"""

import importlib.util
import json

import pytest

from carla_cosmos import Clip, ControlInput, CosmosClient
from carla_cosmos.client import CosmosError, JobFailed
from carla_cosmos.serve import MockServer
from carla_cosmos.synthetic import av7_clip, make_clip

pytestmark = pytest.mark.skipif(importlib.util.find_spec("carla_cosmos_server") is None,
                                reason="carla-cosmos-server not installed")


@pytest.fixture(scope="module")
def mock(tmp_path_factory):
    srv = MockServer(tmp_path_factory.mktemp("state"), delay=0.4, log_file=tmp_path_factory.mktemp("log") / "s.log")
    client = srv.start()
    yield srv, client
    srv.stop()


@pytest.fixture(scope="module")
def clip(tmp_path_factory):
    return make_clip(tmp_path_factory.mktemp("clips"), frames=16, fps=16, scene=True)


def test_models_and_health(mock):
    srv, client = mock
    assert client.health()["status"] == "ok"
    ok, body = client.ready()
    assert ok and body["profile"] == "mock"
    models = client.models()
    assert set(models) == {"cosmos3-nano", "cosmos3-super", "transfer2.5", "transfer2.5-av"}
    assert all(m.available for m in models.values())
    assert client.contract("transfer2.5").frames[0].step == 93


def test_submit_clip_wait_download_dedup(mock, clip, tmp_path):
    srv, client = mock
    seen = []
    job = client.submit_clip(clip, "cosmos3-nano", "sunset", {"depth": "clip", "seg": "clip", "edge": "derive"})
    assert job.info.status == "queued"
    info = job.wait(poll=0.1, on_progress=seen.append)
    assert info.status == "done" and seen[-1].progress == 1.0 and any(0 < i.progress < 1 for i in seen)
    res = job.result()
    assert res.manifest.request.controls["edge"].derive is True
    assert res.manifest.request.rgb  # RGB uploaded because edge is derived
    paths = res.download(tmp_path / "out")
    assert set(paths) == {"camera_front_wide_120fov.mp4", "mock_debug.json"}
    assert (tmp_path / "out" / "manifest.json").exists()
    dbg = json.loads(paths["mock_debug.json"].read_text())
    assert dbg["controls"] == ["depth", "edge", "seg"]
    # second submission of the same clip: nothing missing on the server
    from carla_cosmos.client import sha256_file

    cam = clip.manifest.camera_names[0]
    assert client.missing_blobs([sha256_file(clip.video("rgb", cam)), sha256_file(clip.video("depth", cam))]) == []
    listed = client.jobs(mine=True)
    assert job.id in {j.id for j in listed}


def test_local_validation_before_upload(mock, clip):
    srv, client = mock
    with pytest.raises(CosmosError) as ei:
        client.submit_clip(clip, "transfer2.5", "x", {"depth": "clip"})  # 16 frames, needs 93k
    assert any("93" in e for e in ei.value.errors)
    with pytest.raises(CosmosError, match="unknown source"):
        client.submit_clip(clip, "cosmos3-nano", "x", {"depth": "magic"})
    with pytest.raises(CosmosError) as ei:
        client.submit_clip(clip, "cosmos3-nano", "x", {"vis": "clip"})
    assert any("not supported" in e for e in ei.value.errors)


def test_av7_scene_and_weights(mock, tmp_path):
    srv, client = mock
    clip = av7_clip(tmp_path / "clips", seconds=1)
    job = client.submit_clip(clip, "transfer2.5-av", "rain", {"hdmap_bbox": ("scene", 0.7)},
                             views=list(clip.manifest.camera_names[:2]))
    info = job.wait(poll=0.1)
    assert info.status == "done" and len(info.views) == 2
    res = job.result()
    assert res.manifest.request.controls["hdmap_bbox"].weight == 0.7
    assert {f.view for f in res.manifest.files if f.kind == "video"} == set(clip.manifest.camera_names[:2])
    # explicit ControlInput passthrough with per-view blobs
    cams = clip.manifest.camera_names[:2]
    blobs = {v: client.upload(clip.video("rgb", v)) for v in cams}
    with pytest.raises(CosmosError) as ei:
        client.submit_clip(clip, "transfer2.5-av", "x", {"hdmap_bbox": ControlInput(blobs={cams[0]: blobs[cams[0]]})},
                           views=cams)
    assert any("no blob for view" in e for e in ei.value.errors)


def test_cancel_and_job_failed(mock, clip):
    srv, client = mock
    a = client.submit_clip(clip, "cosmos3-nano", "a", {"depth": "clip"})
    b = client.submit_clip(clip, "cosmos3-nano", "b", {"depth": "clip"})
    assert b.cancel()["accepted"] is True
    with pytest.raises(JobFailed):
        b.wait(poll=0.05)
    assert b.wait(poll=0.05, raise_on_failure=False).status == "cancelled"
    assert a.wait(poll=0.1).status == "done"


# ----------------------------------------------------------------------------- mask-out classes

def _semantic_clip(tmp_path_factory, frames, name):
    """A synthetic clip whose seg video is a real CityScapes-palette semantic AOV."""
    import numpy as np

    from carla_cosmos import controls

    clip = make_clip(tmp_path_factory.mktemp(name), frames=frames, fps=16, width=96, height=64)
    tags = np.full((64, 96), 1, np.uint8)     # road
    tags[20:44, 30:66] = 14                   # a car
    controls.encode_frames(clip.video("seg", clip.manifest.camera_names[0]),
                           [controls.colourise_semantic(tags)] * frames, 16, "control")
    return clip


def test_submit_masks_the_uploaded_videos(mock, tmp_path_factory):
    """The blobs the server receives are the masked ones, not the clip's own files."""
    from carla_cosmos.client import sha256_file

    srv, client = mock
    clip = _semantic_clip(tmp_path_factory, 93, "maskclip")
    cam = clip.manifest.camera_names[0]
    job = client.submit_clip(clip, "transfer2.5", "empty street",
                             {"depth": "clip", "seg": "clip", "edge": "derive"},
                             mask_classes=["vehicle"])
    req = job.result().manifest.request if job.wait(poll=0.1).status == "done" else None
    assert req is not None
    assert req.mask_classes == ["car", "truck", "bus", "train", "motorcycle", "bicycle"]
    assert req.mask_dilate == 3
    assert req.controls["depth"].blob != sha256_file(clip.video("depth", cam)), "depth must be re-encoded masked"
    assert req.rgb[cam] != sha256_file(clip.video("rgb", cam)), "RGB is masked too (edge is derived from it)"


def test_mask_classes_are_refused_where_the_backend_has_no_mask_input(mock, tmp_path_factory):
    """Cosmos 3 takes no mask video, and a pixels-only mask does not remove anything.

    Measured on the node 2026-08-28: ``c3-blur-mask-vehicles`` (blur derived from a masked RGB)
    reproduced the black silhouettes in the output frame for frame, so the request is refused
    rather than silently producing them — before the masking pass re-encodes anything.
    """
    srv, client = mock
    clip = _semantic_clip(tmp_path_factory, 16, "maskclip_c3")
    for backend in ("cosmos3-nano", "cosmos3-super"):
        with pytest.raises(CosmosError, match="has no mask input") as err:
            client.submit_clip(clip, backend, "empty street", {"depth": "clip"}, mask_classes=["vehicle"])
        assert "ClipGT scene package" in str(err.value.errors[0])


def test_submit_uploads_the_mask_video_for_transfer25(mock, tmp_path_factory):
    srv, client = mock
    clip = _semantic_clip(tmp_path_factory, 93, "maskclip93")
    cam = clip.manifest.camera_names[0]
    job = client.submit_clip(clip, "transfer2.5", "empty street", {"depth": "clip", "seg": "clip"},
                             weights={"depth": 1.0, "seg": 0.5}, mask_classes=["car"], mask_dilate=1)
    assert job.wait(poll=0.1).status == "done"
    req = job.result().manifest.request
    assert set(req.masks) == {cam} and len(req.masks[cam]) == 64
    assert req.mask_classes == ["car"] and req.mask_dilate == 1
    assert (req.controls["depth"].weight, req.controls["seg"].weight) == (1.0, 0.5)


def test_mask_needs_a_semantic_source(mock, clip):
    srv, client = mock
    with pytest.raises(CosmosError, match="no semantic class information"):
        client.submit_clip(clip, "transfer2.5", "x", {"depth": "clip"}, mask_classes=["car"])


def test_mask_rejects_unknown_class_before_any_upload(mock, clip):
    srv, client = mock
    with pytest.raises(CosmosError, match="unknown semantic class"):
        client.submit_clip(clip, "cosmos3-nano", "x", {"depth": "clip"}, mask_classes=["kangaroo"])


def test_mask_without_any_pixel_input_is_an_error(mock, tmp_path_factory):
    """AV with only a scene-rendered control uploads no pixels; masking would silently do nothing."""
    srv, client = mock
    av = av7_clip(tmp_path_factory.mktemp("av"), seconds=1)
    with pytest.raises(CosmosError, match="has no effect on this request"):
        client.submit_clip(av, "transfer2.5-av", "x", {"hdmap_bbox": "scene"}, mask_classes=["car"])


def test_weight_for_an_absent_control_is_rejected(mock, clip):
    srv, client = mock
    with pytest.raises(CosmosError, match="weight given for control"):
        client.submit_clip(clip, "cosmos3-nano", "x", {"depth": "clip"}, weights={"seg": 0.5})


def test_a_derived_blur_or_vis_hint_gets_our_stronger_preset(mock, clip, tmp_path_factory):
    """Both pipelines default ``preset_blur_strength`` to "medium", which pins the palette.

    See ``carla_cosmos.client.DEFAULT_BLUR_PRESET``: the control is a bilateral-filtered copy of
    the capture, and at "medium" it is 20.5/255 away from the RGB, so the prompt cannot restyle.
    """
    srv, client = mock

    def extra_of(the_clip=clip, **kwargs):
        job = client.submit_clip(the_clip, **kwargs)
        job.wait(poll=0.1)
        return job.result().manifest.request.extra

    assert extra_of(backend="cosmos3-nano", prompt="night",
                    controls={"blur": "derive"})["preset_blur_strength"] == "very_high"
    clip93 = make_clip(tmp_path_factory.mktemp("clip93"), frames=93, fps=16)
    assert extra_of(clip93, backend="transfer2.5", prompt="night",
                    controls={"vis": "derive"})["preset_blur_strength"] == "very_high"
    # the caller's own value wins
    assert extra_of(backend="cosmos3-nano", prompt="night", controls={"blur": "derive"},
                    extra={"preset_blur_strength": "medium"})["preset_blur_strength"] == "medium"
    # and a control that is not a derived blur/vis is left alone
    assert "preset_blur_strength" not in extra_of(backend="cosmos3-nano", prompt="night",
                                                  controls={"edge": "derive", "depth": "clip"})
