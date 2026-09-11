"""Result persistence (``carla_cosmos.results``) against a fake HTTP server.

The fake serves the four endpoints a download touches (``/v1/status``,
``/v1/jobs/{id}``, ``/v1/jobs/{id}/result`` and ``/v1/jobs/{id}/result/{name}``)
through an ``httpx.MockTransport``, so no server package, no network, no GPU.
"""

from __future__ import annotations

import hashlib
import json
import time
from datetime import datetime, timedelta, timezone

import httpx
import pytest

from carla_cosmos import CosmosClient, ResultStore
from carla_cosmos.client import CosmosError, JobFailed, sha256_file
from carla_cosmos.contracts import (
    CameraManifest,
    ClipManifest,
    ControlInput,
    JobRequest,
    ResultFile,
    ResultManifest,
    RigManifest,
)
from carla_cosmos.results import IndexEntry, default_results_root

CAM = "camera:front:wide:120fov"
JOB_ID = "job_0001"
CLIP_ID = "clip_alpha"


def clip_manifest(clip_id: str = CLIP_ID) -> ClipManifest:
    return ClipManifest(
        clip_id=clip_id, carla_version="test", map="Town10", weather={"cloudiness": 10.0},
        rig=RigManifest(name="single", mount="exact", cameras=[
            CameraManifest(name=CAM, hfov=120.0, width=64, height=32, t_flu=[1.7, 0.0, 1.5],
                           rpy_flu=[0.0, 0.0, 0.0],
                           attach_ue={"x": 1.7, "y": 0.0, "z": 1.5, "pitch": 0.0, "yaw": 0.0, "roll": 0.0})]),
        fps=16, frames=93, ego_id=1, ego_type_id="vehicle.test", rear_axle_offset_ue=[-1.4, 0.0, 0.0],
        aovs=["rgb", "depth"], videos={"rgb/camera_front_wide_120fov": "rgb_camera_front_wide_120fov.mp4"},
        scene_dir=None, seed=3)


class FakeServer:
    """A carla-cosmos server with one finished job, in ~40 lines."""

    def __init__(self, files: dict[str, bytes] | None = None, status: str = "done",
                 retention: dict | None = None, finished: str | None = None, error: str | None = None) -> None:
        self.files = files if files is not None else {
            "camera_front_wide_120fov.mp4": b"result-video-bytes" * 8,
            "control_depth.mp4": b"depth-control-bytes" * 4,
            "grid.mp4": b"grid" * 16,
            "mock_debug.json": b'{"controls": ["depth"]}',
        }
        self.status = status
        self.error = error
        self.retention = {"job_ttl_hours": 168.0, "blob_ttl_hours": 72.0} if retention is None else retention
        self.finished = finished or _iso(datetime.now(timezone.utc))
        self.request = JobRequest(
            backend="transfer2.5", prompt="wet asphalt at dusk", seed=11, resolution="720", views=[CAM],
            controls={"depth": ControlInput(blob="b" * 64, weight=0.7), "edge": ControlInput(derive=True)},
            rgb={CAM: "a" * 64}, extra={"control_guidance": 1.0})
        self.hits: list[str] = []
        self.corrupt: set[str] = set()

    # -- fake wire ------------------------------------------------------------
    def manifest(self) -> ResultManifest:
        return ResultManifest(
            job_id=JOB_ID, backend="transfer2.5", worker="transfer25-0", request=self.request,
            clip=clip_manifest(), timings={"queued": 1.5, "preparing": 2.0, "running": 30.0},
            worker_manifest={"checkpoint": "sha256:beef"},
            files=[ResultFile(name=n, size=len(b), sha256=hashlib.sha256(b).hexdigest(),
                              view=CAM if n.endswith(".mp4") else None,
                              kind="control" if n.startswith("control_") else
                                   ("json" if n.endswith(".json") else "video"))
                   for n, b in self.files.items()])

    def handler(self, request: httpx.Request) -> httpx.Response:
        path = request.url.path
        self.hits.append(path)
        if path == "/v1/status":
            return httpx.Response(200, json={"version": "test", "retention": self.retention})
        if path == f"/v1/jobs/{JOB_ID}":
            return httpx.Response(200, json={
                "id": JOB_ID, "backend": "transfer2.5", "status": self.status, "priority": "interactive",
                "token_id": "t1", "created": _iso(datetime.now(timezone.utc) - timedelta(minutes=5)),
                "started": None, "finished": self.finished, "progress": 1.0, "message": "",
                "error": self.error, "views": [CAM], "worker": "transfer25-0"})
        if path == f"/v1/jobs/{JOB_ID}/result":
            if self.status != "done":
                return httpx.Response(409, json={"detail": f"job is '{self.status}'"})
            return httpx.Response(200, json=json.loads(self.manifest().model_dump_json()))
        prefix = f"/v1/jobs/{JOB_ID}/result/"
        if path.startswith(prefix):
            name = path[len(prefix):]
            if name not in self.files:
                return httpx.Response(404, json={"detail": "no such file"})
            body = self.files[name] + b"extra" if name in self.corrupt else self.files[name]
            return httpx.Response(200, content=body)
        return httpx.Response(404, json={"detail": path})

    def client(self) -> CosmosClient:
        return CosmosClient("http://fake", token="cc_test", transport=httpx.MockTransport(self.handler))

    def job(self):
        return self.client().job(JOB_ID)

    def downloads(self) -> list[str]:
        return [h for h in self.hits if "/result/" in h]


def _iso(dt: datetime) -> str:
    return dt.strftime("%Y-%m-%dT%H:%M:%S") + ".000Z"


# -- root resolution ---------------------------------------------------------------------------

def test_results_root_precedence(tmp_path, monkeypatch):
    monkeypatch.delenv("COSMOS_RESULTS", raising=False)
    monkeypatch.chdir(tmp_path)
    assert default_results_root() == pytest.importorskip("pathlib").Path("cosmos-results")
    monkeypatch.setenv("COSMOS_RESULTS", str(tmp_path / "env"))
    assert default_results_root() == tmp_path / "env"
    assert default_results_root(tmp_path / "explicit") == tmp_path / "explicit"


# -- downloading -------------------------------------------------------------------------------

def test_download_stores_every_file_and_job_json(tmp_path):
    srv = FakeServer()
    stored = srv.job().download(tmp_path / "results")

    d = tmp_path / "results" / CLIP_ID / JOB_ID
    assert stored.path == d.resolve()
    assert {p.name for p in d.iterdir()} == set(srv.files) | {"manifest.json", "job.json"}
    for name, body in srv.files.items():
        assert (d / name).read_bytes() == body
    assert sorted(stored.videos) == ["camera_front_wide_120fov.mp4", "control_depth.mp4", "grid.mp4"]
    assert stored.bytes == sum(len(b) for b in srv.files.values())

    rec = json.loads((d / "job.json").read_text())
    assert rec["job_id"] == JOB_ID and rec["clip_id"] == CLIP_ID and rec["backend"] == "transfer2.5"
    assert rec["status"] == "done" and rec["error"] is None and rec["worker"] == "transfer25-0"
    assert rec["prompt"] == "wet asphalt at dusk" and rec["seed"] == 11 and rec["resolution"] == "720"
    assert rec["views"] == [CAM] and rec["complete"] is True
    assert rec["controls"]["depth"]["weight"] == 0.7 and rec["controls"]["edge"]["derive"] is True
    assert rec["request"]["extra"] == {"control_guidance": 1.0}          # backend pass-through kept
    assert rec["request"]["rgb"] == {CAM: "a" * 64}
    assert rec["timings"] == {"queued": 1.5, "preparing": 2.0, "running": 30.0}
    assert rec["server"] == "http://fake"
    by_name = {f["name"]: f for f in rec["files"]}
    for name, body in srv.files.items():
        assert by_name[name]["size"] == len(body)
        assert by_name[name]["sha256"] == hashlib.sha256(body).hexdigest() == sha256_file(d / name)
    assert by_name["control_depth.mp4"]["kind"] == "control"

    # the server's own listing is kept verbatim next to the files
    man = json.loads((d / "manifest.json").read_text())
    assert man["worker_manifest"] == {"checkpoint": "sha256:beef"} and man["job_id"] == JOB_ID


def test_download_is_idempotent_and_repairs(tmp_path):
    srv = FakeServer()
    job = srv.job()
    job.download(tmp_path)
    first = len(srv.downloads())
    assert first == len(srv.files)

    job.download(tmp_path)                                  # nothing to fetch the second time
    assert len(srv.downloads()) == first

    d = tmp_path / CLIP_ID / JOB_ID
    (d / "grid.mp4").write_bytes(b"truncated")              # damaged on disk -> re-fetched
    stored = job.download(tmp_path)
    assert srv.downloads()[first:] == [f"/v1/jobs/{JOB_ID}/result/grid.mp4"]
    assert (d / "grid.mp4").read_bytes() == srv.files["grid.mp4"]
    assert stored.complete is True


def test_size_mismatch_is_an_error_and_leaves_no_file(tmp_path):
    srv = FakeServer()
    srv.corrupt.add("grid.mp4")
    with pytest.raises(CosmosError, match="listing says"):
        srv.job().download(tmp_path)
    d = tmp_path / CLIP_ID / JOB_ID
    assert not (d / "grid.mp4").exists() and not (d / "grid.mp4.part").exists()


def test_partial_download_by_name(tmp_path):
    srv = FakeServer()
    stored = srv.job().download(tmp_path, names=["grid.mp4"])
    assert [f.name for f in stored.files] == ["grid.mp4"] and stored.complete is False
    assert ResultStore(tmp_path).get(JOB_ID).stored is False


def test_download_of_a_failed_job_raises(tmp_path):
    srv = FakeServer(status="failed", error="worker OOM")
    with pytest.raises(JobFailed, match="worker OOM"):
        srv.job().download(tmp_path)
    assert not (tmp_path / CLIP_ID).exists()


def test_progress_callback(tmp_path):
    srv = FakeServer()
    seen = []
    srv.job().download(tmp_path, progress=lambda name, i, n: seen.append((name, i, n)))
    assert [s[1] for s in seen] == [1, 2, 3, 4] and {s[2] for s in seen} == {4}


# -- retention ---------------------------------------------------------------------------------

def test_expiry_is_recorded_from_the_server_ttl(tmp_path):
    srv = FakeServer()
    stored = srv.job().download(tmp_path)
    assert stored.retention_hours == 168.0
    left = stored.expires_in_hours()
    assert 167.0 < left <= 168.0


def test_expiry_warning_when_the_ttl_is_nearly_up(tmp_path, caplog):
    srv = FakeServer(retention={"job_ttl_hours": 2.0},
                     finished=_iso(datetime.now(timezone.utc) - timedelta(hours=1.5)))
    with caplog.at_level("WARNING"):
        stored = srv.job().download(tmp_path)
    assert stored.expires_in_hours() < 1.0
    assert any("expires in 0.5 h" in r.getMessage() for r in caplog.records)


def test_no_retention_published_means_no_expiry_claim(tmp_path):
    srv = FakeServer(retention={})
    stored = srv.job().download(tmp_path)
    assert stored.retention_hours is None and stored.server_expires is None
    assert stored.expires_in_hours() is None


def test_client_retention_is_cached_and_tolerates_a_broken_status():
    srv = FakeServer()
    c = srv.client()
    assert c.retention()["job_ttl_hours"] == 168.0
    assert c.retention() == {"job_ttl_hours": 168.0, "blob_ttl_hours": 72.0}
    assert srv.hits.count("/v1/status") == 1

    def boom(request):
        return httpx.Response(500, text="nope")

    broken = CosmosClient("http://fake", transport=httpx.MockTransport(boom))
    assert broken.retention() is None


# -- the index ---------------------------------------------------------------------------------

def test_index_records_where_results_live(tmp_path):
    srv = FakeServer()
    stored = srv.job().download(tmp_path)
    store = ResultStore(tmp_path)
    (entry,) = store.index()
    assert entry.job_id == JOB_ID and entry.clip_id == CLIP_ID and entry.stored is True
    assert entry.directory == stored.directory and entry.files == 4 and entry.bytes == stored.bytes
    assert entry.server_expires == stored.server_expires
    assert store.get(JOB_ID) == entry and store.get("nope") is None
    assert store.load(JOB_ID).prompt == "wet asphalt at dusk"
    body = json.loads((tmp_path / "index.json").read_text())
    assert body["format_version"] == 1 and len(body["jobs"]) == 1


def test_note_submitted_then_download_upgrades_the_entry(tmp_path):
    srv = FakeServer()
    store = ResultStore(tmp_path)
    job = srv.job()
    store.note_submitted(job, clip_id=CLIP_ID)
    entry = store.get(JOB_ID)
    assert entry.stored is False and entry.directory is None and entry.backend == "transfer2.5"
    job.download(tmp_path)
    entry = store.get(JOB_ID)
    assert entry.stored is True and entry.directory.endswith(f"{CLIP_ID}/{JOB_ID}")
    assert len(store.index()) == 1


def test_index_survives_unreadable_rows_and_can_be_rebuilt(tmp_path):
    srv = FakeServer()
    srv.job().download(tmp_path)
    store = ResultStore(tmp_path)
    (tmp_path / "index.json").write_text(json.dumps({"format_version": 1, "jobs": [{"nonsense": 1}]}))
    assert store.index() == []
    entries = store.rebuild_index()
    assert [e.job_id for e in entries] == [JOB_ID] and store.get(JOB_ID).stored is True

    (tmp_path / "index.json").write_text("{not json")
    assert ResultStore(tmp_path).index() == []


def test_store_root_from_env(tmp_path, monkeypatch):
    monkeypatch.setenv("COSMOS_RESULTS", str(tmp_path / "env-root"))
    srv = FakeServer()
    stored = srv.job().download()
    assert stored.path == (tmp_path / "env-root" / CLIP_ID / JOB_ID).resolve()
    assert (tmp_path / "env-root" / "index.json").exists()


def test_two_clips_two_directories(tmp_path):
    a = FakeServer()
    a.job().download(tmp_path)
    store = ResultStore(tmp_path)
    other = IndexEntry(job_id="job_0002", clip_id="clip_beta", backend="cosmos3-nano", stored=False)
    store.upsert(other)
    assert {e.job_id for e in store.index()} == {JOB_ID, "job_0002"}
    assert store.job_dir("clip_beta", "job_0002") == tmp_path / "clip_beta" / "job_0002"


def test_result_names_cannot_escape_the_job_directory(tmp_path):
    srv = FakeServer(files={"../../escape.mp4": b"nope"})
    with pytest.raises(ValueError, match="refusing result file name"):
        srv.job().download(tmp_path)
    assert not (tmp_path.parent / "escape.mp4").exists()


# -- CLI ---------------------------------------------------------------------------------------

def test_cli_result_stores_into_the_default_root(tmp_path, monkeypatch, capsys):
    from carla_cosmos import cli

    srv = FakeServer()
    monkeypatch.setenv("COSMOS_RESULTS", str(tmp_path / "root"))
    monkeypatch.setattr(cli, "CosmosClient", lambda *a, **kw: srv.client())
    assert cli.main(["result", JOB_ID]) == 0
    out = capsys.readouterr().out
    assert str(tmp_path / "root" / CLIP_ID / JOB_ID) in out and "grid.mp4" in out
    assert (tmp_path / "root" / CLIP_ID / JOB_ID / "job.json").exists()


def test_cli_jobs_shows_where_results_are(tmp_path, monkeypatch, capsys):
    from carla_cosmos import cli

    srv = FakeServer()

    def handler(request: httpx.Request) -> httpx.Response:
        if request.url.path == "/v1/jobs":
            return httpx.Response(200, json=[json.loads(srv.job().info.model_dump_json())])
        return srv.handler(request)

    client = CosmosClient("http://fake", transport=httpx.MockTransport(handler))
    monkeypatch.setattr(cli, "CosmosClient", lambda *a, **kw: client)

    assert cli.main(["jobs", "--out", str(tmp_path)]) == 0
    first = capsys.readouterr()
    assert "not stored locally" in first.out and JOB_ID in first.out

    srv.job().download(tmp_path)
    assert cli.main(["jobs", "--out", str(tmp_path)]) == 0
    second = capsys.readouterr().out
    assert "not stored locally" not in second
    assert f"stored: {(tmp_path / CLIP_ID / JOB_ID).resolve()}" in second

    assert cli.main(["jobs", "--out", str(tmp_path), "--json"]) == 0
    rows = json.loads(capsys.readouterr().out)
    assert rows[0]["stored"] is True and rows[0]["directory"].endswith(JOB_ID)


def test_cli_jobs_lists_locally_stored_jobs_the_server_forgot(tmp_path, monkeypatch, capsys):
    from carla_cosmos import cli

    srv = FakeServer()
    srv.job().download(tmp_path)

    def handler(request: httpx.Request) -> httpx.Response:
        if request.url.path == "/v1/jobs":
            return httpx.Response(200, json=[])          # collected by the server's gc
        return srv.handler(request)

    monkeypatch.setattr(cli, "CosmosClient",
                        lambda *a, **kw: CosmosClient("http://fake", transport=httpx.MockTransport(handler)))
    assert cli.main(["jobs", "--out", str(tmp_path)]) == 0
    out = capsys.readouterr().out
    assert "known locally but not listed by the server (1)" in out and JOB_ID in out


def test_cli_watch_no_download_warns_about_the_ttl(tmp_path, monkeypatch, capsys):
    from carla_cosmos import cli

    srv = FakeServer(retention={"job_ttl_hours": 168.0})
    monkeypatch.setattr(cli, "CosmosClient", lambda *a, **kw: srv.client())
    assert cli.main(["watch", JOB_ID, "--out", str(tmp_path), "--no-download"]) == 0
    cap = capsys.readouterr()
    assert "not downloaded" in cap.out and "carla-cosmos result" in cap.out
    assert "keeps it for 168 h" in cap.err
    assert not (tmp_path / CLIP_ID).exists()


def test_cli_watch_downloads_by_default(tmp_path, monkeypatch, capsys):
    from carla_cosmos import cli

    srv = FakeServer()
    monkeypatch.setattr(cli, "CosmosClient", lambda *a, **kw: srv.client())
    t0 = time.monotonic()
    assert cli.main(["watch", JOB_ID, "--out", str(tmp_path)]) == 0
    assert time.monotonic() - t0 < 10.0
    assert (tmp_path / CLIP_ID / JOB_ID / "job.json").exists()
    assert "stored 4 file(s)" in capsys.readouterr().out
