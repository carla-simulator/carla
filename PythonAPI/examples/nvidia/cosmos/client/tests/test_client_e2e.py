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
