"""API + scheduler: a scene-package control is rendered server-side before dispatch."""

from carla_cosmos.client import sha256_file, zip_dir
from carla_cosmos.synthetic import av7_clip, make_clip

from conftest import make_server, submission


def _upload_scene(ctx, clip, tmp_path):
    z = zip_dir(clip.scene_dir, tmp_path / "scene.zip")
    bid = sha256_file(z)
    r = ctx.client.put(f"/v1/blobs/{bid}", content=z.read_bytes(), headers={"Content-Type": "application/zip"})
    assert r.status_code in (200, 201)
    return bid


def test_av_scene_rendered_then_generated(render_server, tmp_path):
    ctx = render_server
    clip = av7_clip(tmp_path / "clips", seconds=1)
    models = ctx.client.get("/v1/models").json()
    assert models["transfer2.5-av"]["scene_rendering"] is True and models["transfer2.5"]["scene_rendering"] is False
    assert "wsm-renderer" not in ctx.client.get("/v1/health/ready").json()["backends"]
    bid = _upload_scene(ctx, clip, tmp_path)
    body = submission(clip, "transfer2.5-av", {"hdmap_bbox": {"scene": bid, "weight": 0.9}},
                      views=list(clip.manifest.camera_names))
    r = ctx.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    info = ctx.wait_job(r.json()["id"], timeout=60)
    assert info["status"] == "done", info
    m = ctx.client.get(f"/v1/jobs/{info['id']}/result").json()
    assert sorted(f["view"] for f in m["files"] if f["kind"] == "video") == sorted(clip.manifest.camera_names)
    assert "rendering" in m["timings"]
    rendered = ctx.store.job_dir(info["id"]) / "inputs" / "rendered_hdmap_bbox"
    assert len(list(rendered.glob("*.mp4"))) == 7
    # the mock worker echoed a control (no RGB) -> result equals a rendered control
    dbg = ctx.client.get(f"/v1/jobs/{info['id']}/result/mock_debug.json").json()
    assert dbg["inputs"]["controls"]["hdmap_bbox"]["paths"][clip.manifest.camera_names[0]].startswith(str(rendered))
    assert "scene_dir" not in dbg["inputs"]["controls"]["hdmap_bbox"]


def test_cosmos3_wsm_scene_single_view(render_server, tmp_path):
    ctx = render_server
    clip = make_clip(tmp_path / "clips", frames=101, fps=10, scene=True, cameras=["camera:front:wide:120fov"])
    bid = _upload_scene(ctx, clip, tmp_path)
    body = submission(clip, "cosmos3-nano", {"wsm": {"scene": bid}})
    r = ctx.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    info = ctx.wait_job(r.json()["id"], timeout=60)
    assert info["status"] == "done", info
    dbg = ctx.client.get(f"/v1/jobs/{info['id']}/result/mock_debug.json").json()
    assert dbg["inputs"]["controls"]["wsm"]["path"].endswith("rendered_wsm/camera_front_wide_120fov.mp4")


def test_scene_without_renderer_is_409(tmp_path):
    ctx = make_server(tmp_path, ["--delay", "0.1"])
    with ctx.client:
        ctx.client.headers.update({"Authorization": f"Bearer {ctx.token}"})
        ctx.wait_ready()
        clip = av7_clip(tmp_path / "clips", seconds=1)
        assert ctx.client.get("/v1/models").json()["transfer2.5-av"]["scene_rendering"] is False
        bid = _upload_scene(ctx, clip, tmp_path)
        body = submission(clip, "transfer2.5-av", {"hdmap_bbox": {"scene": bid}}, views=list(clip.manifest.camera_names))
        r = ctx.client.post("/v1/jobs", json=body)
        assert r.status_code == 409 and "renderer" in r.text
