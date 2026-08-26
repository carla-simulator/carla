"""Server-side handle of one worker process (see ``cosmos_workers.common.protocol``)."""

from __future__ import annotations

import asyncio
import logging
import os
import subprocess
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Awaitable, Callable

from cosmos_workers.common import protocol

log = logging.getLogger(__name__)

ProgressCb = Callable[[float, str], Awaitable[None] | None]


@dataclass
class RunOutcome:
    ok: bool
    files: list[dict[str, Any]] = field(default_factory=list)
    manifest: dict[str, Any] = field(default_factory=dict)
    error: str | None = None
    cancelled: bool = False


@dataclass
class WorkerHandle:
    """A worker as seen by the scheduler."""

    name: str
    backends: list[str]
    socket: Path
    type: str = "external"
    gpus: list[int] = field(default_factory=list)
    process: subprocess.Popen | None = None
    state: str = "starting"
    """starting -> loading -> ready <-> busy; error / dead are terminal."""
    error: str | None = None
    smoke_ok: bool | None = None
    smoke_seconds: float | None = None
    current_job: str | None = None
    started_at: float = field(default_factory=time.time)
    ready_at: float | None = None

    # -- liveness --------------------------------------------------------------------
    @property
    def alive(self) -> bool:
        if self.process is None:
            return self.state not in ("dead",)
        return self.process.poll() is None

    @property
    def serving(self) -> bool:
        """Ready for jobs: loaded, smoke passed, idle."""
        return self.state == "ready" and self.smoke_ok is True and self.current_job is None

    def snapshot(self) -> dict[str, Any]:
        return {
            "name": self.name, "type": self.type, "backends": self.backends, "gpus": self.gpus,
            "state": self.state, "error": self.error, "smoke_ok": self.smoke_ok,
            "smoke_seconds": self.smoke_seconds, "current_job": self.current_job,
            "pid": self.process.pid if self.process else None,
            "ready_after_s": (self.ready_at - self.started_at) if self.ready_at else None,
        }

    # -- protocol calls ------------------------------------------------------------------
    async def hello(self) -> dict[str, Any] | None:
        try:
            return await protocol.call_result(str(self.socket), "hello")
        except (OSError, protocol.ProtocolError) as exc:
            log.debug("hello %s: %s", self.name, exc)
            return None

    async def wait_ready(self, timeout: float | None = None, poll: float = 1.0) -> bool:
        """Poll ``hello`` until the worker reports ``ready`` (then run the smoke sample)."""
        t0 = time.monotonic()
        while True:
            if not self.alive:
                self.state = "dead"
                self.error = self.error or f"worker process exited with code {self.process.returncode if self.process else '?'}"
                return False
            info = await self.hello() if self.socket.exists() else None
            if info is not None:
                self.state = info.get("state", "loading")
                if info.get("backends"):
                    self.backends = list(info["backends"])
                if self.state == "error":
                    self.error = info.get("error") or "worker reported error state"
                    return False
                if self.state == "ready":
                    self.ready_at = time.time()
                    await self.smoke()
                    return self.smoke_ok is True
            if timeout is not None and time.monotonic() - t0 > timeout:
                self.error = f"worker not ready after {timeout:.0f}s (state {self.state})"
                return False
            await asyncio.sleep(poll)

    async def smoke(self) -> None:
        try:
            res = await protocol.call_result(str(self.socket), "smoke")
        except (OSError, protocol.ProtocolError) as exc:
            self.smoke_ok, self.error = False, f"smoke call failed: {exc}"
            return
        self.smoke_ok = bool(res.get("ok"))
        self.smoke_seconds = res.get("seconds")
        if not self.smoke_ok:
            self.error = f"smoke sample failed: {res.get('error')}"
            log.error("worker %s: %s", self.name, self.error)
        else:
            log.info("worker %s: smoke sample ok in %.1fs", self.name, self.smoke_seconds or 0)

    async def run(self, job: dict[str, Any], on_progress: ProgressCb) -> RunOutcome:
        self.current_job = job["job_id"]
        self.state = "busy"
        stream = protocol.call(str(self.socket), "run", job=job)
        try:
            async for msg in stream:
                ev = msg.get("event")
                if ev == "progress":
                    r = on_progress(float(msg.get("fraction", 0.0)), str(msg.get("message", "")))
                    if asyncio.iscoroutine(r):
                        await r
                elif ev == "done":
                    return RunOutcome(ok=True, files=msg.get("files", []), manifest=msg.get("manifest", {}))
                elif ev == "failed":
                    return RunOutcome(ok=False, error=msg.get("error", "worker failed"),
                                      cancelled=bool(msg.get("cancelled")))
            return RunOutcome(ok=False, error="worker ended the run without a terminal event")
        except (OSError, protocol.ProtocolError) as exc:
            if not self.alive:
                self.state, self.error = "dead", f"worker died during job: {exc}"
                return RunOutcome(ok=False, error=self.error)
            return RunOutcome(ok=False, error=f"worker connection error: {exc}")
        finally:
            await stream.aclose()
            self.current_job = None
            if self.state == "busy":
                self.state = "ready"

    async def cancel(self, job_id: str) -> bool:
        try:
            res = await protocol.call_result(str(self.socket), "cancel", job_id=job_id)
            return bool(res.get("cancelled"))
        except (OSError, protocol.ProtocolError) as exc:
            log.warning("cancel on %s failed: %s", self.name, exc)
            return False

    async def shutdown(self, grace: float = 10.0) -> None:
        if self.socket.exists():
            try:
                await asyncio.wait_for(protocol.call_result(str(self.socket), "shutdown"), timeout=3.0)
            except (OSError, protocol.ProtocolError, asyncio.TimeoutError):
                pass
        if self.process is not None and self.process.poll() is None:
            try:
                self.process.terminate()
                await asyncio.to_thread(self.process.wait, grace)
            except subprocess.TimeoutExpired:
                self.process.kill()
        self.state = "dead"


def spawn_worker(name: str, type_: str, backends: list[str], gpus: list[int], python: str, module: str,
                 socket: Path, extra_args: list[str], env: dict[str, str], log_dir: Path | None = None) -> WorkerHandle:
    """Start a worker subprocess in its venv."""
    if socket.exists():
        socket.unlink()
    cmd = [python, "-m", module, "--socket", str(socket), "--name", name, "--backends", ",".join(backends), *extra_args]
    penv = {**os.environ, **env}
    penv["CUDA_VISIBLE_DEVICES"] = ",".join(str(g) for g in gpus) if gpus else ""
    stdout = None
    if log_dir is not None:
        log_dir.mkdir(parents=True, exist_ok=True)
        stdout = open(log_dir / f"worker-{name}.log", "ab")  # noqa: SIM115 - lives with the process
    log.info("starting worker %s: %s (GPUs %s)", name, " ".join(cmd), gpus or "-")
    proc = subprocess.Popen(cmd, env=penv, stdout=stdout, stderr=subprocess.STDOUT if stdout else None)
    return WorkerHandle(name=name, backends=backends, socket=socket, type=type_, gpus=gpus, process=proc)
