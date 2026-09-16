import json

import pytest

from conftest import submission, upload


def _submit_ok(server, clip, backend="cosmos3-nano", controls=None, priority="interactive"):
    cam = clip.manifest.camera_names[0]
    rgb = upload(server, clip.video("rgb", cam))
    ctrls = controls or {"depth": {"blob": upload(server, clip.video("depth", cam))},
                         "seg": {"blob": upload(server, clip.video("seg", cam))}}
    body = submission(clip, backend, ctrls, rgb={cam: rgb}, priority=priority)
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    return r.json()


def test_job_lifecycle_and_result(server, clip16):
    info = _submit_ok(server, clip16)
    assert info["status"] == "queued" and info["queue_position"] == 0
    final = server.wait_job(info["id"])
    assert final["status"] == "done", final
    assert final["progress"] == 1.0 and final["worker"] == "mock" and final["started"] and final["finished"]
    res = server.client.get(f"/v1/jobs/{info['id']}/result")
    assert res.status_code == 200
    m = res.json()
    assert m["backend"] == "cosmos3-nano" and m["worker_manifest"]["mock"] is True
    names = {f["name"] for f in m["files"]}
    assert names == {"camera_front_wide_120fov.mp4", "mock_debug.json"}
    assert set(m["timings"]) == {"queued", "preparing", "running"}
    video = next(f for f in m["files"] if f["kind"] == "video")
    assert video["view"] == "camera:front:wide:120fov" and video["size"] > 0
    r = server.client.get(f"/v1/jobs/{info['id']}/result/{video['name']}")
    assert r.status_code == 200 and len(r.content) == video["size"]
    dbg = json.loads(server.client.get(f"/v1/jobs/{info['id']}/result/mock_debug.json").content)
    assert dbg["controls"] == ["depth", "seg"]
    assert server.client.get(f"/v1/jobs/{info['id']}/result/../../request.json").status_code == 404
    # delete a finished job removes its files
    r = server.client.delete(f"/v1/jobs/{info['id']}")
    assert r.json()["action"] == "delete"
    assert not server.store.job_dir(info["id"]).exists()
    assert server.client.get(f"/v1/jobs/{info['id']}").status_code == 404


def test_result_only_when_done(server, clip16):
    info = _submit_ok(server, clip16)
    r = server.client.get(f"/v1/jobs/{info['id']}/result")
    assert r.status_code == 409
    server.wait_job(info["id"])


def test_validation_errors(server, clip16, clip93):
    cam = clip16.manifest.camera_names[0]
    # transfer2.5 needs 93*k frames: 16 frames -> 400 with actionable errors
    body = submission(clip16, "transfer2.5", {"depth": {"blob": "a" * 64}}, rgb={cam: "b" * 64})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 400
    errs = r.json()["detail"]["errors"]
    assert any("93" in e and "frames" in e for e in errs)
    # unknown backend
    body = submission(clip16, "cosmos9", {"depth": {"blob": "a" * 64}})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 400 and "unknown backend" in r.json()["detail"]["errors"][0]
    # unsupported control + missing required control
    body = submission(clip93, "transfer2.5-av", {"depth": {"blob": "a" * 64}}, rgb={cam: "b" * 64})
    r = server.client.post("/v1/jobs", json=body)
    errs = r.json()["detail"]["errors"]
    assert any("not supported" in e for e in errs) and any("required" in e for e in errs)
    # valid shape but blob never uploaded
    body = submission(clip93, "transfer2.5", {"depth": {"blob": "a" * 64}}, rgb={cam: "b" * 64})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 400 and "not uploaded" in r.json()["detail"]["errors"][0]
    # empty prompt rejected by the model itself
    body = submission(clip16, "cosmos3-nano", {"depth": {"blob": "a" * 64}})
    body["request"]["prompt"] = "   "
    assert server.client.post("/v1/jobs", json=body).status_code == 422
    assert server.store.status_counts() == {}


def test_scene_control_and_multiview(render_server, tmp_path):
    server = render_server
    from carla_cosmos.client import sha256_file, zip_dir
    from carla_cosmos.synthetic import av7_clip

    clip = av7_clip(tmp_path / "clips", seconds=1)
    z = zip_dir(clip.scene_dir, tmp_path / "scene.zip")
    bid = sha256_file(z)
    r = server.client.put(f"/v1/blobs/{bid}", content=z.read_bytes(), headers={"Content-Type": "application/zip"})
    assert r.status_code == 201
    body = submission(clip, "transfer2.5-av", {"hdmap_bbox": {"scene": bid, "weight": 0.8}},
                      views=list(clip.manifest.camera_names[:3]))
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    info = server.wait_job(r.json()["id"])
    assert info["status"] == "done" and len(info["views"]) == 3
    m = server.client.get(f"/v1/jobs/{info['id']}/result").json()
    assert sorted(f["view"] for f in m["files"] if f["kind"] == "video") == sorted(clip.manifest.camera_names[:3])
    scene_dir = server.store.job_dir(info["id"]) / "inputs" / "scene_hdmap_bbox"
    assert any(scene_dir.glob("*.obstacle.parquet"))
    assert len(list((server.store.job_dir(info["id"]) / "inputs" / "rendered_hdmap_bbox").glob("*.mp4"))) == 3


def test_bad_scene_zip_fails_in_prepare(render_server, clip16):
    server = render_server
    import hashlib

    data = b"definitely not a zip"
    bid = hashlib.sha256(data).hexdigest()
    server.client.put(f"/v1/blobs/{bid}", content=data)
    body = submission(clip16, "cosmos3-nano", {"wsm": {"scene": bid}})
    body["manifest"]["frames"] = 101  # wsm needs 101*k (manifest is client-declared)
    body["manifest"]["fps"] = 10  # and a scene package needs a renderer-compatible rate (10/15/30, not 16)
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    info = server.wait_job(r.json()["id"])
    assert info["status"] == "failed" and "not a zip" in info["error"]


def test_failed_render_reports_worker_error(render_server, tmp_path):
    """A renderer failure lands in the job message (phase + worker error) and in ``error``."""
    server = render_server
    from carla_cosmos.client import sha256_file, zip_dir
    from carla_cosmos.synthetic import av7_clip

    clip = av7_clip(tmp_path / "clips", seconds=1)
    z = zip_dir(clip.scene_dir, tmp_path / "scene.zip")
    bid = sha256_file(z)
    server.client.put(f"/v1/blobs/{bid}", content=z.read_bytes(), headers={"Content-Type": "application/zip"})
    body = submission(clip, "transfer2.5-av", {"hdmap_bbox": {"scene": bid}}, views=list(clip.manifest.camera_names[:1]))
    body["manifest"]["frames"] = 3 * (29 + 28 * 19)  # valid for the contract, far longer than the scene package
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    info = server.wait_job(r.json()["id"])
    assert info["status"] == "failed", info
    assert info["message"].startswith("render 'hdmap_bbox' failed: RuntimeError: rendered camera:front:wide:120fov")
    assert "too short" in info["message"] and "\n" not in info["message"]
    assert info["error"].startswith(info["message"])


def test_failed_run_reports_worker_error(tmp_path, clip16):
    from conftest import make_server

    ctx = make_server(tmp_path, ["--delay", "0.1", "--fail"])
    with ctx.client:
        ctx.client.headers.update({"Authorization": f"Bearer {ctx.token}"})
        ctx.wait_ready()
        info = ctx.wait_job(_submit_ok(ctx, clip16)["id"])
        assert info["status"] == "failed"
        assert info["message"] == "run on mock failed: RuntimeError: mock worker configured with --fail"
        assert info["error"].startswith(info["message"])
        assert ctx.client.get("/v1/jobs?status=failed").json()[0]["message"] == info["message"]


def test_cancel_queued_and_running(slow_server, clip16):
    s = slow_server
    a = _submit_ok(s, clip16)
    b = _submit_ok(s, clip16)
    # b is queued behind a (single worker)
    assert s.client.get(f"/v1/jobs/{b['id']}").json()["queue_position"] == 0
    r = s.client.delete(f"/v1/jobs/{b['id']}")
    assert r.json() == {"id": b["id"], "action": "cancel", "accepted": True}
    assert s.client.get(f"/v1/jobs/{b['id']}").json()["status"] == "cancelled"
    # a is running: cancel reaches the worker
    import time

    for _ in range(100):
        if s.client.get(f"/v1/jobs/{a['id']}").json()["status"] == "running":
            break
        time.sleep(0.05)
    r = s.client.delete(f"/v1/jobs/{a['id']}")
    assert r.json()["accepted"] is True
    info = s.wait_job(a["id"])
    assert info["status"] == "cancelled"
    # worker is reusable afterwards
    c = _submit_ok(s, clip16)
    assert s.wait_job(c["id"])["status"] == "done"


def test_list_filters_and_status_endpoints(server, clip16):
    a = _submit_ok(server, clip16, priority="batch")
    server.wait_job(a["id"])
    jobs = server.client.get("/v1/jobs?status=done").json()
    assert [j["id"] for j in jobs] == [a["id"]]
    assert server.client.get("/v1/jobs?backend=transfer2.5").json() == []
    st = server.client.get("/v1/status").json()
    assert st["jobs"] == {"done": 1} and st["workers"][0]["state"] == "ready"
    models = server.client.get("/v1/models").json()
    assert models["cosmos3-nano"]["available"] is True and models["cosmos3-nano"]["workers"] == ["mock"]
    assert server.client.get("/v1/models/nope").status_code == 404
    metrics = server.client.get("/v1/metrics").text
    assert 'cosmos_jobs{status="done"} 1' in metrics and "cosmos_ready 1" in metrics


def test_backend_without_worker_is_409(tmp_path, clip16):
    from conftest import make_server

    ctx = make_server(tmp_path, [], with_worker=False)
    with ctx.client:
        ctx.client.headers.update({"Authorization": f"Bearer {ctx.token}"})
        assert ctx.client.get("/v1/health/ready").status_code == 503
        body = submission(clip16, "cosmos3-nano", {"depth": {"blob": "a" * 64}})
        r = ctx.client.post("/v1/jobs", json=body)
        assert r.status_code == 409 and "no loaded worker" in r.text


def test_mask_video_is_materialised_for_the_worker(server, clip93):
    """--mask-classes uploads one mask video per view; prepare links it into inputs/ so a
    backend that supports it (Transfer 2.5 mask_path) can hand it to the model."""
    cam = clip93.manifest.camera_names[0]
    rgb = upload(server, clip93.video("rgb", cam))
    body = submission(clip93, "transfer2.5", {"depth": {"blob": upload(server, clip93.video("depth", cam))}},
                      rgb={cam: rgb}, masks={cam: upload(server, clip93.video("seg", cam))},
                      mask_classes=["car", "truck"], mask_dilate=3)
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    final = server.wait_job(r.json()["id"])
    assert final["status"] == "done", final
    inputs = server.store.job_dir(final["id"]) / "inputs"
    assert (inputs / "mask_camera_front_wide_120fov.mp4").exists()
    request = json.loads((server.store.job_dir(final["id"]) / "request.json").read_text())
    assert request["mask_classes"] == ["car", "truck"] and request["mask_dilate"] == 3
    # ... and the result manifest carries it, so the job is reproducible from the download
    m = server.client.get(f"/v1/jobs/{final['id']}/result").json()
    assert m["request"]["mask_classes"] == ["car", "truck"]


def test_mask_blob_must_be_uploaded(server, clip93):
    cam = clip93.manifest.camera_names[0]
    body = submission(clip93, "transfer2.5", {"depth": {"blob": upload(server, clip93.video("depth", cam))}},
                      rgb={cam: upload(server, clip93.video("rgb", cam))}, masks={cam: "d" * 64})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 400 and "not uploaded" in str(r.json()["detail"]["errors"])


def test_mask_video_rejected_for_a_backend_without_maskable_controls(server, clip16):
    cam = clip16.manifest.camera_names[0]
    body = submission(clip16, "cosmos3-nano", {"depth": {"blob": upload(server, clip16.video("depth", cam))}},
                      rgb={cam: upload(server, clip16.video("rgb", cam))},
                      masks={cam: upload(server, clip16.video("seg", cam))})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 400
    assert any("takes no mask video" in e for e in r.json()["detail"]["errors"])
