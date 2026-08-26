"""FastAPI application factory.

Routes (all under ``/v1`` need ``Authorization: Bearer <token>`` except health):

=====================================  ==========================================================
``GET  /v1/health/live``               process is up
``GET  /v1/health/ready``              200 when every worker is loaded and smoke-tested, else 503
``GET  /v1/status``                    profile, workers, queues, uptime
``GET  /v1/models``                    backend contracts + availability
``GET  /v1/models/{id}``
``PUT  /v1/blobs/{sha256}``            raw body upload, content-addressed (201 new / 200 existed)
``POST /v1/blobs/check``               ``{"ids": [...]}`` -> ``{"present": [...], "missing": [...]}``
``GET  /v1/blobs/{sha256}``            download
``POST /v1/jobs``                      :class:`JobSubmission` -> :class:`JobInfo` (400 with ``errors``)
``GET  /v1/jobs``                      ``?status=&backend=&mine=1&limit=``
``GET  /v1/jobs/{id}``
``DELETE /v1/jobs/{id}``               cancel (queued/running) or delete (terminal)
``GET  /v1/jobs/{id}/result``          :class:`ResultManifest`
``GET  /v1/jobs/{id}/result/{name}``   one result file
``GET  /v1/metrics``                   Prometheus text
``GET  /ui``                           status page
=====================================  ==========================================================
"""

from __future__ import annotations

import asyncio
import hashlib
import logging
import os
import tempfile
import time
from contextlib import asynccontextmanager
from pathlib import Path
from typing import Any

from fastapi import FastAPI, HTTPException, Query, Request
from fastapi.responses import FileResponse, HTMLResponse, JSONResponse, PlainTextResponse, RedirectResponse
from pydantic import BaseModel

from carla_cosmos.contracts import (
    BUILTIN_CONTRACTS,
    BackendContract,
    JobInfo,
    JobSubmission,
    ModelInfo,
    ResultManifest,
    validate_request,
)

from . import __version__, metrics
from .auth import BearerAuthMiddleware, TokenStore
from .config import Settings
from .gc import gc_loop
from .scheduler import Scheduler, load_result_manifest, result_file
from .store import Store
from .ui import PAGE
from .workers_rpc import WorkerHandle

log = logging.getLogger(__name__)


class AppState:
    def __init__(self, settings: Settings, tokens: TokenStore, workers: list[WorkerHandle],
                 profile_name: str, contracts: dict[str, BackendContract] | None = None) -> None:
        self.settings = settings
        self.tokens = tokens
        self.profile_name = profile_name
        self.contracts = contracts or dict(BUILTIN_CONTRACTS)
        self.store = Store(settings.db_file, settings.blobs_dir, settings.jobs_dir)
        self.scheduler = Scheduler(self.store, workers, self.contracts)
        self.started = time.time()
        self._tasks: list[asyncio.Task] = []

    @property
    def workers(self) -> list[WorkerHandle]:
        return self.scheduler.workers

    async def start(self) -> None:
        await self.scheduler.start()
        self._tasks.append(asyncio.create_task(self._bring_up_workers(), name="workers-bringup"))
        self._tasks.append(asyncio.create_task(gc_loop(self.store, self.settings, self.tokens), name="gc"))

    async def _bring_up_workers(self) -> None:
        results = await asyncio.gather(*(w.wait_ready() for w in self.workers), return_exceptions=True)
        for w, r in zip(self.workers, results):
            if isinstance(r, Exception):
                w.state, w.error = "error", f"{type(r).__name__}: {r}"
            if w.state != "ready":
                log.error("worker %s failed to come up: %s", w.name, w.error)
        if self.scheduler.is_ready():
            log.info("all %d worker(s) ready; serving %s", len(self.workers),
                     sorted(self.scheduler.backends_available()))
        self.scheduler.wake()

    async def stop(self) -> None:
        for t in self._tasks:
            t.cancel()
        await asyncio.gather(*self._tasks, return_exceptions=True)
        await self.scheduler.stop()
        await asyncio.gather(*(w.shutdown() for w in self.workers if w.process is not None), return_exceptions=True)
        self.tokens.flush_usage()
        self.store.close()


def create_app(settings: Settings, tokens: TokenStore, workers: list[WorkerHandle],
               profile_name: str = "custom", contracts: dict[str, BackendContract] | None = None) -> FastAPI:
    settings.ensure_dirs()
    state = AppState(settings, tokens, workers, profile_name, contracts)

    @asynccontextmanager
    async def lifespan(app: FastAPI):
        await state.start()
        try:
            yield
        finally:
            await state.stop()

    app = FastAPI(title="carla-cosmos", version=__version__, lifespan=lifespan, docs_url="/v1/docs",
                  openapi_url="/v1/openapi.json")
    app.state.cosmos = state
    _routes(app, state)
    # Outermost: rejects before any body is read.
    return _wrap(app, BearerAuthMiddleware(app, tokens))


class _AsgiWrapper:
    """Expose the FastAPI object (for tests) while serving through the auth middleware."""

    def __init__(self, app: FastAPI, asgi) -> None:
        self.app = app
        self.asgi = asgi
        self.state = app.state
        self.router = app.router

    async def __call__(self, scope, receive, send):
        await self.asgi(scope, receive, send)


def _wrap(app: FastAPI, asgi) -> Any:
    return _AsgiWrapper(app, asgi)


class BlobCheck(BaseModel):
    ids: list[str]


def _routes(app: FastAPI, st: AppState) -> None:
    store, sched = st.store, st.scheduler

    # -- health / status ---------------------------------------------------------------------
    @app.get("/")
    async def root():
        return RedirectResponse("/ui")

    @app.get("/ui", response_class=HTMLResponse)
    async def ui():
        return PAGE

    @app.get("/v1/health/live")
    async def live():
        return {"status": "ok", "version": __version__}

    @app.get("/v1/health/ready")
    async def ready():
        body = {"ready": sched.is_ready(), "profile": st.profile_name,
                "backends": sorted(sched.backends_available()),
                "workers": [w.snapshot() for w in st.workers]}
        return JSONResponse(body, status_code=200 if body["ready"] else 503)

    @app.get("/v1/status")
    async def status():
        return {
            "version": __version__, "profile": st.profile_name, "uptime_s": round(time.time() - st.started, 1),
            "ready": sched.is_ready(), "workers": [w.snapshot() for w in st.workers],
            "queued": store.queued_counts(), "jobs": store.status_counts(),
            "blobs": dict(zip(("count", "bytes"), store.blob_stats())),
            "guardrails": st.settings.guardrails,
        }

    @app.get("/v1/metrics", response_class=PlainTextResponse)
    async def metrics_():
        return metrics.render(store, sched)

    # -- models --------------------------------------------------------------------------------
    def _model_info(c: BackendContract) -> ModelInfo:
        avail = sched.backends_available()
        return ModelInfo(contract=c, available=c.id in avail, workers=avail.get(c.id, []),
                         queued=store.queued_counts().get(c.id, 0))

    @app.get("/v1/models")
    async def models() -> dict[str, ModelInfo]:
        return {cid: _model_info(c) for cid, c in st.contracts.items()}

    @app.get("/v1/models/{model_id}")
    async def model(model_id: str) -> ModelInfo:
        c = st.contracts.get(model_id)
        if c is None:
            raise HTTPException(404, f"unknown backend '{model_id}'")
        return _model_info(c)

    # -- blobs ---------------------------------------------------------------------------------------
    @app.put("/v1/blobs/{blob_id}")
    async def put_blob(blob_id: str, request: Request):
        blob_id = blob_id.lower()
        if len(blob_id) != 64 or any(c not in "0123456789abcdef" for c in blob_id):
            raise HTTPException(400, "blob id must be the hex sha256 of the content")
        existing = store.get_blob(blob_id)
        if existing is not None and store.blob_path(blob_id).exists():
            store.touch_blobs([blob_id])
            return JSONResponse({"id": blob_id, "size": existing.size, "existed": True}, status_code=200)
        dest = store.blob_path(blob_id)
        dest.parent.mkdir(parents=True, exist_ok=True)
        h = hashlib.sha256()
        size = 0
        fd, tmp = tempfile.mkstemp(prefix=".upload-", dir=dest.parent)
        try:
            with os.fdopen(fd, "wb") as f:
                async for chunk in request.stream():
                    f.write(chunk)
                    h.update(chunk)
                    size += len(chunk)
            if h.hexdigest() != blob_id:
                raise HTTPException(400, f"content sha256 {h.hexdigest()} does not match blob id {blob_id}")
            os.replace(tmp, dest)
        finally:
            if os.path.exists(tmp):
                os.unlink(tmp)
        rec = store.add_blob(blob_id, size, request.headers.get("content-type"), request.headers.get("x-filename"))
        return JSONResponse({"id": blob_id, "size": rec.size, "existed": False}, status_code=201)

    @app.post("/v1/blobs/check")
    async def check_blobs(body: BlobCheck):
        missing = set(store.missing_blobs(body.ids))
        present = [i for i in body.ids if i not in missing]
        store.touch_blobs(present)
        return {"present": present, "missing": sorted(missing)}

    @app.get("/v1/blobs/{blob_id}")
    async def get_blob(blob_id: str):
        rec = store.get_blob(blob_id)
        p = store.blob_path(blob_id)
        if rec is None or not p.exists():
            raise HTTPException(404, "no such blob")
        return FileResponse(p, media_type=rec.content_type or "application/octet-stream", filename=rec.filename)

    # -- jobs -----------------------------------------------------------------------------------------
    def _info(job) -> JobInfo:
        return job.info(store.queue_position(job))

    @app.post("/v1/jobs", status_code=202)
    async def submit(sub: JobSubmission, request: Request) -> JobInfo:
        contract = st.contracts.get(sub.request.backend)
        if contract is None:
            raise HTTPException(400, {"errors": [f"unknown backend '{sub.request.backend}' "
                                                 f"(known: {sorted(st.contracts)})"]})
        errors = validate_request(contract, sub.request, sub.manifest)
        if errors:
            raise HTTPException(400, {"errors": errors})
        if sub.request.backend not in sched.backends_available():
            raise HTTPException(409, {"errors": [f"backend '{sub.request.backend}' has no loaded worker on this "
                                                 f"server (profile '{st.profile_name}')"]})
        blobs = _referenced_blobs(sub)
        missing = store.missing_blobs(blobs)
        if missing:
            raise HTTPException(400, {"errors": [f"blob(s) not uploaded: {missing}"]})
        views = sub.request.views or sub.manifest.camera_names
        token_id = request.scope.get("state", {}).get("token_id", "anonymous")
        job = store.create_job(token_id, sub.request, sub.manifest.model_dump(), views, blobs)
        sched.wake()
        log.info("job %s queued: %s by token %s (%s)", job.id, job.backend, token_id, job.priority)
        return _info(job)

    @app.get("/v1/jobs")
    async def list_jobs(request: Request, status: str | None = None, backend: str | None = None,
                        mine: bool = False, limit: int = Query(100, ge=1, le=1000)) -> list[JobInfo]:
        token_id = request.scope.get("state", {}).get("token_id") if mine else None
        return [_info(j) for j in store.list_jobs(status, backend, token_id, limit)]

    @app.get("/v1/jobs/{job_id}")
    async def get_job(job_id: str) -> JobInfo:
        job = store.get_job(job_id)
        if job is None:
            raise HTTPException(404, "no such job")
        return _info(job)

    @app.delete("/v1/jobs/{job_id}")
    async def cancel_or_delete(job_id: str):
        job = store.get_job(job_id)
        if job is None:
            raise HTTPException(404, "no such job")
        if job.status in ("queued", "preparing", "running"):
            ok = await sched.cancel(job_id)
            return {"id": job_id, "action": "cancel", "accepted": ok}
        d = store.job_dir(job_id)
        if d.exists():
            await asyncio.to_thread(_rmtree, d)
        store.delete_job(job_id)
        return {"id": job_id, "action": "delete", "accepted": True}

    @app.get("/v1/jobs/{job_id}/result")
    async def job_result(job_id: str) -> ResultManifest:
        job = store.get_job(job_id)
        if job is None:
            raise HTTPException(404, "no such job")
        if job.status != "done":
            raise HTTPException(409, f"job is '{job.status}', result available only when done")
        m = load_result_manifest(store, job_id)
        if m is None:
            raise HTTPException(410, "result files were removed (TTL)")
        return m

    @app.get("/v1/jobs/{job_id}/result/{name:path}")
    async def job_result_file(job_id: str, name: str):
        job = store.get_job(job_id)
        if job is None:
            raise HTTPException(404, "no such job")
        p = result_file(store, job_id, name)
        if p is None:
            raise HTTPException(404, "no such result file")
        return FileResponse(p, filename=Path(name).name)


def _referenced_blobs(sub: JobSubmission) -> list[str]:
    ids = list(sub.request.rgb.values())
    for inp in sub.request.controls.values():
        if inp.blob:
            ids.append(inp.blob)
        ids.extend(inp.blobs.values())
        if inp.scene:
            ids.append(inp.scene)
    return sorted(set(ids))


def _rmtree(p: Path) -> None:
    import shutil

    shutil.rmtree(p, ignore_errors=True)
