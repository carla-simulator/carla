import time

from carla_cosmos.contracts import JobRequest
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
