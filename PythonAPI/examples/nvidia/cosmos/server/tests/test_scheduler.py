import time

from carla_cosmos.contracts import JobRequest
from carla_cosmos_server.scheduler import ERROR_TEXT_LIMIT, failure_text
from carla_cosmos_server.store import Store

from conftest import submission, upload


def _mk_store(tmp_path):
    return Store(tmp_path / "db.sqlite", tmp_path / "blobs", tmp_path / "jobs")


def _req(backend="cosmos3-nano", priority="interactive"):
    return JobRequest(backend=backend, prompt="p", priority=priority, controls={"depth": {"blob": "a" * 64}})


def test_claim_order_priority_then_fifo(tmp_path):
    st = _mk_store(tmp_path)
    b1 = st.create_job("t", _req(priority="batch"), {}, ["v"], [])
    time.sleep(0.01)
    b2 = st.create_job("t", _req(priority="batch"), {}, ["v"], [])
    time.sleep(0.01)
    i1 = st.create_job("t", _req(priority="interactive"), {}, ["v"], [])
    other = st.create_job("t", _req(backend="transfer2.5"), {}, ["v"], [])
    assert st.queue_position(b1) == 1 and st.queue_position(b2) == 2 and st.queue_position(i1) == 0
    assert st.claim_next(["cosmos3-nano"], "w").id == i1.id
    assert st.claim_next(["cosmos3-nano"], "w").id == b1.id
    assert st.claim_next(["cosmos3-nano"], "w").id == b2.id
    assert st.claim_next(["cosmos3-nano"], "w") is None
    assert st.claim_next(["transfer2.5", "cosmos3-nano"], "w").id == other.id
    assert st.get_job(i1.id).status == "preparing"


def test_requeue_interrupted(tmp_path):
    st = _mk_store(tmp_path)
    j = st.create_job("t", _req(), {}, ["v"], [])
    st.claim_next(["cosmos3-nano"], "w")
    st.set_status(j.id, "running")
    assert st.requeue_interrupted() == 1
    assert st.get_job(j.id).status == "queued" and st.get_job(j.id).worker is None


def test_interactive_jumps_batch_queue(slow_server, clip16):
    s = slow_server
    cam = clip16.manifest.camera_names[0]
    rgb = upload(s, clip16.video("rgb", cam))
    depth = upload(s, clip16.video("depth", cam))

    def submit(priority):
        body = submission(clip16, "cosmos3-nano", {"depth": {"blob": depth}}, rgb={cam: rgb}, priority=priority)
        r = s.client.post("/v1/jobs", json=body)
        assert r.status_code == 202
        return r.json()["id"]

    a = submit("batch")       # claimed immediately (idle worker)
    b = submit("batch")
    c = submit("interactive")  # must run before b
    assert s.client.get(f"/v1/jobs/{c}").json()["queue_position"] == 0
    assert s.client.get(f"/v1/jobs/{b}").json()["queue_position"] == 1
    for jid in (a, b, c):
        assert s.wait_job(jid, timeout=60)["status"] == "done"
    started = {jid: s.client.get(f"/v1/jobs/{jid}").json()["started"] for jid in (a, b, c)}
    assert started[a] < started[c] < started[b]


def test_failure_text_message_and_capped_error():
    msg, text = failure_text("render 'wsm'", "ValueError: clip fps 16 must divide the renderer's 30 fps")
    assert msg == text == "render 'wsm' failed: ValueError: clip fps 16 must divide the renderer's 30 fps"
    # multi-line error: first line in the message, the rest kept in the error text
    msg, text = failure_text("run on w", "KeyError: 'view'\n  File x, line 1\n", "Traceback (most recent call last):")
    assert msg == "run on w failed: KeyError: 'view'"
    assert text == "run on w failed: KeyError: 'view'\n  File x, line 1\nTraceback (most recent call last):"
    assert failure_text("prepare", None) == ("prepare failed: unknown error",) * 2
    # long tracebacks are capped keeping the first line and the tail
    msg, text = failure_text("run on w", "RuntimeError: boom", "x" * 10000 + "END")
    assert len(text) <= ERROR_TEXT_LIMIT and text.startswith(msg + "\n...\n") and text.endswith("END")


def test_gpu_sharing_workers_never_run_concurrently(tmp_path, clip16, clip93):
    """Latency mode: two model workers on the same GPU take turns; the API shows mode + placement."""
    import sys

    from starlette.testclient import TestClient

    from carla_cosmos_server.app import create_app
    from carla_cosmos_server.auth import TokenStore
    from carla_cosmos_server.config import Settings, default_run_dir
    from carla_cosmos_server.workers_rpc import spawn_worker

    from conftest import Ctx

    settings = Settings(state_dir=tmp_path / "state", run_dir=default_run_dir(tmp_path / "state"), gc_interval_s=3600)
    settings.ensure_dirs()
    tokens = TokenStore(settings.tokens_file)
    token = tokens.create("test")
    workers = []
    for name, backend, gpus, parallel in (("nano", "cosmos3-nano", [0, 1], {"cfg": 2}),
                                          ("super", "cosmos3-super", [0, 1], {"tp": 2}),
                                          ("t25", "transfer2.5", [2], {})):
        workers.append(spawn_worker(
            name=name, type_="mock", backends=[backend], gpus=gpus, python=sys.executable, module="cosmos_workers.mock",
            socket=settings.run_dir / f"{name}.sock", extra_args=["--delay", "1.0", "--steps", "2"], env={},
            log_dir=settings.state_dir / "logs", parallel=parallel))
    app = create_app(settings, tokens, workers, profile_name="test-latency", mode="latency")
    s = Ctx(client=TestClient(app), token=token, token_id=token.split("_")[1], settings=settings, tokens=tokens, app=app)
    with s.client:
        s.client.headers.update({"Authorization": f"Bearer {token}"})
        s.wait_ready()
        assert workers[0].shares_gpu_with(workers[1]) and not workers[0].shares_gpu_with(workers[2])
        ready = s.client.get("/v1/health/ready").json()
        assert ready["mode"] == "latency" and ready["workers"][0]["parallel"] == {"cfg": 2}
        models = s.client.get("/v1/models").json()
        assert models["cosmos3-nano"]["placement"] == {"mode": "latency", "gpus": [0, 1], "parallel": {"cfg": 2}}
        assert models["transfer2.5"]["placement"] == {"mode": "latency", "gpus": [2], "parallel": {}}
        assert models["transfer2.5-av"]["placement"] is None

        ids = {}
        for backend, clip in (("cosmos3-nano", clip16), ("cosmos3-super", clip16), ("transfer2.5", clip93)):
            cam = clip.manifest.camera_names[0]
            rgb, depth = upload(s, clip.video("rgb", cam)), upload(s, clip.video("depth", cam))
            r = s.client.post("/v1/jobs", json=submission(clip, backend, {"depth": {"blob": depth}}, rgb={cam: rgb}))
            assert r.status_code == 202, r.text
            ids[backend] = r.json()["id"]
        infos = {b: s.wait_job(j, timeout=60) for b, j in ids.items()}
        assert all(i["status"] == "done" for i in infos.values()), infos
        a, b, c = (infos[k] for k in ("cosmos3-nano", "cosmos3-super", "transfer2.5"))
        # nano and super share GPUs 0-1: the second only started after the first finished
        assert b["started"] >= a["finished"], (a, b)
        # the worker on its own GPU ran alongside
        assert c["started"] < a["finished"], (a, c)
        man = s.client.get(f"/v1/jobs/{ids['cosmos3-nano']}/result").json()
        assert man["worker"] == "nano"
