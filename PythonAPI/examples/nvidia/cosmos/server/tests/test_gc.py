import time

from carla_cosmos_server.gc import collect

from conftest import submission, upload


def test_ttl_collect(server, clip16):
    cam = clip16.manifest.camera_names[0]
    rgb = upload(server, clip16.video("rgb", cam))
    depth = upload(server, clip16.video("depth", cam))
    body = submission(clip16, "cosmos3-nano", {"depth": {"blob": depth}}, rgb={cam: rgb})
    jid = server.client.post("/v1/jobs", json=body).json()["id"]
    server.wait_job(jid)
    st = server.store
    # nothing is old yet
    assert collect(st, server.settings) == (0, 0)
    old = time.time() - 10 * 24 * 3600
    with st._lock:
        st._conn.execute("UPDATE blobs SET last_used=?", (old,))
        st._conn.execute("UPDATE jobs SET finished=?", (old,))
    assert collect(st, server.settings) == (2, 1)
    assert st.blob_stats() == (0, 0) and st.get_job(jid) is None
    assert not st.job_dir(jid).exists() and not st.blob_path(rgb).exists()


def test_live_job_keeps_blobs(tmp_path):
    from carla_cosmos.contracts import JobRequest
    from carla_cosmos_server.config import Settings
    from carla_cosmos_server.store import Store

    st = Store(tmp_path / "db", tmp_path / "blobs", tmp_path / "jobs")
    st.add_blob("a" * 64, 1, None, None)
    st.create_job("t", JobRequest(backend="cosmos3-nano", prompt="p", controls={"depth": {"blob": "a" * 64}}),
                  {}, ["v"], ["a" * 64])
    with st._lock:
        st._conn.execute("UPDATE blobs SET last_used=0")
    settings = Settings(state_dir=tmp_path, blob_ttl_hours=1)
    assert collect(st, settings) == (0, 0)  # referenced by a queued job
