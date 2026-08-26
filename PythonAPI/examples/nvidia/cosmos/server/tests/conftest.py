"""Fixtures: an API app with a real mock-worker subprocess behind Starlette's TestClient."""

from __future__ import annotations

import sys
import time
from dataclasses import dataclass
from pathlib import Path

import pytest
from starlette.testclient import TestClient

from carla_cosmos.synthetic import make_clip
from carla_cosmos_server.app import create_app
from carla_cosmos_server.auth import TokenStore
from carla_cosmos_server.config import Settings, default_run_dir
from carla_cosmos_server.workers_rpc import spawn_worker


@dataclass
class Ctx:
    client: TestClient
    token: str
    token_id: str
    settings: Settings
    tokens: TokenStore
    app: object

    @property
    def store(self):
        return self.app.state.cosmos.store

    @property
    def scheduler(self):
        return self.app.state.cosmos.scheduler

    def wait_ready(self, timeout: float = 30.0) -> None:
        t0 = time.monotonic()
        while time.monotonic() - t0 < timeout:
            if self.client.get("/v1/health/ready").status_code == 200:
                return
            time.sleep(0.1)
        raise RuntimeError(self.client.get("/v1/health/ready").json())

    def wait_job(self, job_id: str, timeout: float = 30.0) -> dict:
        t0 = time.monotonic()
        while time.monotonic() - t0 < timeout:
            info = self.client.get(f"/v1/jobs/{job_id}").json()
            if info["status"] in ("done", "failed", "cancelled"):
                return info
            time.sleep(0.05)
        raise TimeoutError(info)


def make_server(tmp_path: Path, worker_args: list[str], with_worker: bool = True) -> Ctx:
    settings = Settings(state_dir=tmp_path / "state", run_dir=default_run_dir(tmp_path / "state"),
                        gc_interval_s=3600)
    settings.ensure_dirs()
    tokens = TokenStore(settings.tokens_file)
    token = tokens.create("test")
    token_id = token.split("_")[1]
    workers = []
    if with_worker:
        workers.append(spawn_worker(
            name="mock", type_="mock", backends=["cosmos3-nano", "cosmos3-super", "transfer2.5", "transfer2.5-av"],
            gpus=[], python=sys.executable, module="cosmos_workers.mock", socket=settings.run_dir / "mock.sock",
            extra_args=worker_args, env={}, log_dir=settings.state_dir / "logs"))
    app = create_app(settings, tokens, workers, profile_name="test")
    return Ctx(client=TestClient(app), token=token, token_id=token_id, settings=settings, tokens=tokens, app=app)


@pytest.fixture
def server(tmp_path):
    ctx = make_server(tmp_path, ["--delay", "0.3", "--steps", "3"])
    with ctx.client:
        ctx.client.headers.update({"Authorization": f"Bearer {ctx.token}"})
        ctx.wait_ready()
        yield ctx


@pytest.fixture
def slow_server(tmp_path):
    ctx = make_server(tmp_path, ["--delay", "1.2", "--steps", "4"])
    with ctx.client:
        ctx.client.headers.update({"Authorization": f"Bearer {ctx.token}"})
        ctx.wait_ready()
        yield ctx


@pytest.fixture(scope="session")
def clip16(tmp_path_factory):
    """16 frames @16 fps, rgb/depth/seg/edge, with a (placeholder) scene package."""
    return make_clip(tmp_path_factory.mktemp("clips"), frames=16, fps=16, scene=True, clip_id="clip16")


@pytest.fixture(scope="session")
def clip93(tmp_path_factory):
    return make_clip(tmp_path_factory.mktemp("clips"), frames=93, fps=16, clip_id="clip93")


def upload(ctx: Ctx, path: Path) -> str:
    from carla_cosmos.client import sha256_file

    bid = sha256_file(path)
    r = ctx.client.put(f"/v1/blobs/{bid}", content=path.read_bytes(), headers={"Content-Type": "video/mp4"})
    assert r.status_code in (200, 201), r.text
    return bid


def submission(clip, backend: str, controls: dict, rgb: dict | None = None, **kw) -> dict:
    from carla_cosmos.contracts import JobRequest, JobSubmission

    req = JobRequest(backend=backend, prompt=kw.pop("prompt", "test prompt"), controls=controls, rgb=rgb or {}, **kw)
    return JobSubmission(request=req, manifest=clip.manifest).model_dump(mode="json")
