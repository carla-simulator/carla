"""Job scheduler: per-backend FIFO with priority, one job per worker.

The loop wakes on every submission/cancellation/worker-state change (or once a
second), and for every idle worker claims the best queued job among the
backends that worker serves: ``interactive`` before ``batch``, then oldest
first.  Dispatch is ``prepare`` (resolve blobs into the job's ``inputs``
directory, unpack scene packages) -> ``run`` on the worker with progress
updates -> result manifest.
"""

from __future__ import annotations

import asyncio
import hashlib
import json
import logging
import os
import shutil
import time
import zipfile
from pathlib import Path
from typing import Any

from carla_cosmos.contracts import (
    BackendContract,
    ClipManifest,
    JobRequest,
    ResultFile,
    ResultManifest,
    canonical_camera_name,
)

from .store import JobRecord, Store
from .workers_rpc import WorkerHandle

log = logging.getLogger(__name__)


class PrepareError(Exception):
    """Inputs could not be materialised (missing blob, corrupt scene package)."""


class Scheduler:
    def __init__(self, store: Store, workers: list[WorkerHandle], contracts: dict[str, BackendContract]) -> None:
        self.store = store
        self.workers = workers
        self.contracts = contracts
        self._wake = asyncio.Event()
        self._renderer_lock = asyncio.Lock()
        self._task: asyncio.Task | None = None
        self._running: dict[str, tuple[WorkerHandle, asyncio.Task]] = {}
        self._stopping = False

    # -- lifecycle ------------------------------------------------------------------------
    async def start(self) -> None:
        n = self.store.requeue_interrupted()
        if n:
            log.warning("requeued %d job(s) interrupted by a restart", n)
        self._task = asyncio.create_task(self._loop(), name="scheduler")

    async def stop(self) -> None:
        self._stopping = True
        if self._task:
            self._task.cancel()
            try:
                await self._task
            except asyncio.CancelledError:
                pass
        for _, (_, task) in list(self._running.items()):
            task.cancel()
        await asyncio.gather(*(t for _, t in self._running.values()), return_exceptions=True)

    def wake(self) -> None:
        self._wake.set()

    # -- queries ----------------------------------------------------------------------------
    def backends_available(self) -> dict[str, list[str]]:
        """backend id -> names of workers that are loaded (ready or busy) for it."""
        out: dict[str, list[str]] = {}
        for w in self.workers:
            if w.state in ("ready", "busy") and w.smoke_ok:
                for b in w.backends:
                    if b != self.RENDERER_BACKEND:
                        out.setdefault(b, []).append(w.name)
        return out

    def is_ready(self) -> bool:
        return bool(self.workers) and all(w.state in ("ready", "busy") and w.smoke_ok for w in self.workers)

    RENDERER_BACKEND = "wsm-renderer"

    def renderers(self) -> list[WorkerHandle]:
        return [w for w in self.workers if self.RENDERER_BACKEND in w.backends and w.state in ("ready", "busy")
                and w.smoke_ok]

    def scene_rendering_available(self) -> bool:
        return bool(self.renderers())

    # -- cancel -------------------------------------------------------------------------------
    async def cancel(self, job_id: str) -> bool:
        if self.store.cancel_if_queued(job_id):
            self.wake()
            return True
        entry = self._running.get(job_id)
        if entry is None:
            return False
        worker, _ = entry
        job = self.store.get_job(job_id)
        if job is not None and job.status == "preparing":
            # not on the worker yet: mark and let _execute notice
            self.store.set_status(job_id, "cancelled", error="cancelled during preparation")
            return True
        return await worker.cancel(job_id)

    # -- main loop ------------------------------------------------------------------------------
    async def _loop(self) -> None:
        while not self._stopping:
            for w in self.workers:
                if not w.serving:
                    continue
                job = self.store.claim_next(w.backends, w.name)
                if job is None:
                    continue
                task = asyncio.create_task(self._execute(w, job), name=f"job-{job.id}")
                self._running[job.id] = (w, task)
            try:
                await asyncio.wait_for(self._wake.wait(), timeout=1.0)
            except asyncio.TimeoutError:
                pass
            self._wake.clear()

    async def _execute(self, worker: WorkerHandle, job: JobRecord) -> None:
        t_claim = time.time()
        timings = {"queued": t_claim - job.created}
        worker.current_job = job.id
        try:
            try:
                payload = await asyncio.to_thread(self.prepare, job)
            except PrepareError as exc:
                self.store.set_status(job.id, "failed", error=f"prepare: {exc}", timings=timings)
                return
            fresh = self.store.get_job(job.id)
            if fresh is None or fresh.status == "cancelled":
                return
            scene_controls = {n: c for n, c in payload["inputs"]["controls"].items() if "scene_dir" in c}
            if scene_controls:
                t_render = time.time()
                try:
                    await self._render_scenes(job, payload, scene_controls)
                except PrepareError as exc:
                    self.store.set_status(job.id, "failed", error=f"render: {exc}", timings=timings)
                    return
                timings["rendering"] = time.time() - t_render
                fresh = self.store.get_job(job.id)
                if fresh is None or fresh.status == "cancelled":
                    return
            timings["preparing"] = time.time() - t_claim
            self.store.set_status(job.id, "running", message="dispatched")
            t_run = time.time()

            def progress(fraction: float, message: str) -> None:
                self.store.set_progress(job.id, fraction, message)

            outcome = await worker.run(payload, progress)
            timings["running"] = time.time() - t_run
            if outcome.ok:
                manifest = await asyncio.to_thread(self._write_result, job, worker, payload, outcome.files,
                                                   outcome.manifest, timings)
                self.store.set_status(job.id, "done", timings=timings, message=f"{len(manifest.files)} file(s)")
                log.info("job %s done on %s in %.1fs", job.id, worker.name, timings["running"])
            elif outcome.cancelled:
                self.store.set_status(job.id, "cancelled", error="cancelled", timings=timings)
            else:
                self.store.set_status(job.id, "failed", error=outcome.error, timings=timings)
                log.warning("job %s failed on %s: %s", job.id, worker.name, outcome.error)
        except asyncio.CancelledError:
            self.store.set_status(job.id, "failed", error="server shutting down", timings=timings)
            raise
        except Exception as exc:  # noqa: BLE001 - never let a job kill the loop
            log.exception("job %s crashed the dispatcher", job.id)
            self.store.set_status(job.id, "failed", error=f"internal: {type(exc).__name__}: {exc}", timings=timings)
        finally:
            worker.current_job = None
            self._running.pop(job.id, None)
            self.wake()

    # -- server-side world-scenario rendering ---------------------------------------------------------
    async def _render_scenes(self, job: JobRecord, payload: dict[str, Any],
                             scene_controls: dict[str, dict[str, Any]]) -> None:
        """Turn ``scene_dir`` controls into per-view control videos with a renderer worker."""
        renderers = self.renderers()
        if not renderers:
            raise PrepareError("this server has no world-scenario renderer; upload pre-rendered control videos")
        views = payload["views"]
        inputs_dir = self.store.job_dir(job.id) / "inputs"
        async with self._renderer_lock:  # renderers are shared by all model workers; one render at a time
            renderer = renderers[0]
            for name, spec in scene_controls.items():
                self.store.set_progress(job.id, 0.0, f"rendering '{name}' control for {len(views)} view(s)")
                render_job = {
                    "job_id": f"{job.id}:{name}", "scene_dir": spec["scene_dir"], "cameras": views,
                    "fps": payload["manifest"]["fps"], "frames": payload["manifest"]["frames"],
                    "out_dir": str(inputs_dir / f"rendered_{name}"),
                }

                def progress(fraction: float, message: str) -> None:
                    self.store.set_progress(job.id, 0.0, f"render '{name}': {message}")

                outcome = await renderer.run(render_job, progress)
                if not outcome.ok:
                    raise PrepareError(f"'{name}' control: {outcome.error}")
                by_view = {f["view"]: str(Path(render_job["out_dir"]) / f["name"]) for f in outcome.files if f.get("view")}
                missing = [v for v in views if v not in by_view]
                if missing:
                    raise PrepareError(f"renderer returned no '{name}' video for view(s) {missing}")
                spec.pop("scene_dir")
                if len(views) == 1:
                    spec["path"] = by_view[views[0]]
                else:
                    spec["paths"] = by_view
                spec["rendered"] = outcome.manifest
                log.info("job %s: rendered '%s' for %d view(s) on %s", job.id, name, len(views), renderer.name)

    # -- prepare ----------------------------------------------------------------------------------
    def prepare(self, job: JobRecord) -> dict[str, Any]:
        """Materialise ``jobs/<id>/inputs`` and build the worker payload."""
        request = JobRequest.model_validate(job.request)
        manifest = ClipManifest.model_validate(job.manifest)
        jdir = self.store.job_dir(job.id)
        inputs = jdir / "inputs"
        out = jdir / "result"
        if inputs.exists():
            shutil.rmtree(inputs)
        inputs.mkdir(parents=True)
        out.mkdir(parents=True, exist_ok=True)
        (jdir / "request.json").write_text(request.model_dump_json(indent=2))
        (jdir / "clip_manifest.json").write_text(manifest.model_dump_json(indent=2))

        views = job.views
        rgb: dict[str, str] = {}
        for view, blob_id in request.rgb.items():
            rgb[view] = str(self._link_blob(blob_id, inputs / f"rgb_{canonical_camera_name(view)}.mp4"))
        controls: dict[str, dict[str, Any]] = {}
        for name, inp in request.controls.items():
            entry: dict[str, Any] = {"weight": inp.weight}
            if inp.blobs:
                entry["paths"] = {
                    v: str(self._link_blob(b, inputs / f"{name}_{canonical_camera_name(v)}.mp4"))
                    for v, b in inp.blobs.items()}
            elif inp.blob is not None:
                view = views[0] if views else manifest.camera_names[0]
                entry["path"] = str(self._link_blob(inp.blob, inputs / f"{name}_{canonical_camera_name(view)}.mp4"))
            elif inp.scene is not None:
                entry["scene_dir"] = str(self._unpack_scene(inp.scene, inputs / f"scene_{name}"))
            elif inp.derive:
                entry["derive"] = True
            controls[name] = entry
        return {
            "job_id": job.id,
            "request": request.model_dump(),
            "manifest": manifest.model_dump(),
            "views": views,
            "inputs": {"rgb": rgb, "controls": controls},
            "out_dir": str(out),
        }

    def _link_blob(self, blob_id: str, dest: Path) -> Path:
        src = self.store.blob_path(blob_id)
        if self.store.get_blob(blob_id) is None or not src.exists():
            raise PrepareError(f"blob {blob_id} is missing (expired or never uploaded)")
        try:
            os.symlink(src.resolve(), dest)
        except OSError:
            shutil.copyfile(src, dest)
        return dest

    def _unpack_scene(self, blob_id: str, dest: Path) -> Path:
        src = self.store.blob_path(blob_id)
        if self.store.get_blob(blob_id) is None or not src.exists():
            raise PrepareError(f"scene blob {blob_id} is missing")
        try:
            with zipfile.ZipFile(src) as zf:
                dest.mkdir(parents=True, exist_ok=True)
                root = dest.resolve()
                for info in zf.infolist():
                    target = (dest / info.filename).resolve()
                    if root not in target.parents and target != root:
                        raise PrepareError(f"scene package entry escapes the archive: {info.filename}")
                zf.extractall(dest)
        except zipfile.BadZipFile as exc:
            raise PrepareError(f"scene blob {blob_id} is not a zip archive: {exc}") from exc
        # tolerate one top-level folder inside the zip
        entries = [p for p in dest.iterdir() if not p.name.startswith(".")]
        if len(entries) == 1 and entries[0].is_dir():
            return entries[0]
        return dest

    # -- result -----------------------------------------------------------------------------------
    def _write_result(self, job: JobRecord, worker: WorkerHandle, payload: dict[str, Any],
                      files: list[dict[str, Any]], worker_manifest: dict[str, Any],
                      timings: dict[str, float]) -> ResultManifest:
        out = Path(payload["out_dir"])
        result_files: list[ResultFile] = []
        for f in files:
            name = f["name"]
            p = out / name
            if not p.is_file():
                raise RuntimeError(f"worker reported missing result file {name}")
            result_files.append(ResultFile(name=name, size=p.stat().st_size, sha256=_sha256(p),
                                           view=f.get("view"), kind=f.get("kind", "other")))
        manifest = ResultManifest(
            job_id=job.id, backend=job.backend, worker=worker.name,
            request=JobRequest.model_validate(job.request), clip=ClipManifest.model_validate(job.manifest),
            files=result_files, worker_manifest=worker_manifest, timings=dict(timings),
        )
        (out / "manifest.json").write_text(manifest.model_dump_json(indent=2))
        return manifest


def _sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def load_result_manifest(store: Store, job_id: str) -> ResultManifest | None:
    p = store.job_dir(job_id) / "result" / "manifest.json"
    if not p.is_file():
        return None
    return ResultManifest.model_validate_json(p.read_text())


def result_file(store: Store, job_id: str, name: str) -> Path | None:
    """Safe path of a result file (no traversal)."""
    base = (store.job_dir(job_id) / "result").resolve()
    p = (base / name).resolve()
    if base != p and base not in p.parents:
        return None
    return p if p.is_file() else None


def job_payload_debug(store: Store, job_id: str) -> dict[str, Any]:
    d = store.job_dir(job_id)
    return {"dir": str(d), "exists": d.exists(), "files": sorted(str(p.relative_to(d)) for p in d.rglob("*"))
            if d.exists() else []}


__all__ = ["Scheduler", "PrepareError", "load_result_manifest", "result_file", "json"]
