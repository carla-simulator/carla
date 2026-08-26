"""Base class and socket server for workers.

Subclass :class:`Worker`, implement :meth:`Worker.load`, :meth:`Worker.run`
and optionally :meth:`Worker.smoke`, then call :func:`serve` from the
worker's ``__main__``.  ``run`` executes in a thread so a blocking inference
loop does not stall the socket; it must poll ``ctx.cancelled`` between chunks.
"""

from __future__ import annotations

import argparse
import asyncio
import logging
import os
import signal
import sys
import threading
import time
import traceback
from dataclasses import dataclass, field
from typing import Any, Callable

from . import protocol

log = logging.getLogger("cosmos_worker")


class CancelledJob(Exception):
    """Raised by :meth:`RunContext.check_cancelled` when the server cancelled the job."""


@dataclass
class RunContext:
    """What a running job can do: report progress, learn about cancellation."""

    job_id: str
    _report: Callable[[float, str], None]
    _cancel: threading.Event = field(default_factory=threading.Event)

    def progress(self, fraction: float, message: str = "") -> None:
        """Report progress (``fraction`` in 0..1)."""
        self._report(max(0.0, min(1.0, float(fraction))), message)

    @property
    def cancelled(self) -> bool:
        return self._cancel.is_set()

    def check_cancelled(self) -> None:
        """Raise :class:`CancelledJob` if the server asked to stop."""
        if self._cancel.is_set():
            raise CancelledJob(self.job_id)


@dataclass
class RunResult:
    """What :meth:`Worker.run` returns."""

    files: list[dict[str, Any]]
    """``{"name": relative path, "view": camera or None, "kind": "video"|"json"|"other"}``."""
    manifest: dict[str, Any] = field(default_factory=dict)


class Worker:
    """A model backend adapter.  One job at a time."""

    name: str = "worker"
    backends: tuple[str, ...] = ()

    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.state = "loading"
        self.error: str | None = None
        self.current: RunContext | None = None
        self._lock = threading.Lock()

    # -- to implement -------------------------------------------------------
    def load(self) -> None:
        """Load weights / start the model process.  Called once, before ``ready``."""

    def run(self, job: dict[str, Any], ctx: RunContext) -> RunResult:
        """Generate.  Return files written under ``job["out_dir"]``."""
        raise NotImplementedError

    def smoke(self) -> None:
        """Tiny end-to-end sample proving the model works (raise on failure)."""

    def shutdown(self) -> None:
        """Release resources (model subprocesses, GPU memory)."""

    # -- helpers ----------------------------------------------------------------
    def hello(self) -> dict[str, Any]:
        return {
            "name": self.name,
            "backends": list(self.backends),
            "state": self.state,
            "error": self.error,
            "version": protocol.VERSION,
            "pid": os.getpid(),
        }


def add_common_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--socket", required=True, help="unix socket path to bind")
    parser.add_argument("--name", default=None, help="worker name (defaults to the class name)")
    parser.add_argument("--backends", default=None,
                        help="comma-separated backend ids served (defaults to the worker's list)")
    parser.add_argument("--log-level", default=os.environ.get("COSMOS_LOG_LEVEL", "INFO"))


class _Server:
    def __init__(self, worker: Worker, path: str) -> None:
        self.worker = worker
        self.path = path
        self.loop = asyncio.get_event_loop()
        self._stop = asyncio.Event()

    async def handle(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        async def send(msg: dict[str, Any]) -> None:
            writer.write(protocol.encode(msg))
            await writer.drain()

        try:
            req = await protocol.read_message(reader)
            if req is None:
                return
            op, rid = req.get("op"), req.get("id", 0)
            if op == "hello":
                await send({"id": rid, "event": "result", **self.worker.hello()})
            elif op == "status":
                cur = self.worker.current
                await send({"id": rid, "event": "result", "state": self.worker.state,
                            "job_id": cur.job_id if cur else None})
            elif op == "smoke":
                await self._smoke(rid, send)
            elif op == "run":
                await self._run(rid, req.get("job") or {}, send)
            elif op == "cancel":
                cur = self.worker.current
                ok = cur is not None and cur.job_id == req.get("job_id")
                if ok:
                    cur._cancel.set()
                await send({"id": rid, "event": "result", "cancelled": ok})
            elif op == "shutdown":
                await send({"id": rid, "event": "result"})
                self._stop.set()
            else:
                await send({"id": rid, "event": "failed", "error": f"unknown op '{op}'"})
        except (protocol.ProtocolError, ConnectionError) as exc:
            log.warning("connection error: %s", exc)
        finally:
            writer.close()

    async def _smoke(self, rid: int, send) -> None:
        if self.worker.state != "ready":
            await send({"id": rid, "event": "result", "ok": False,
                        "error": f"worker state is '{self.worker.state}'"})
            return
        t0 = time.monotonic()
        try:
            await asyncio.to_thread(self.worker.smoke)
            await send({"id": rid, "event": "result", "ok": True, "seconds": time.monotonic() - t0})
        except Exception as exc:  # noqa: BLE001 - reported to the server
            log.exception("smoke failed")
            await send({"id": rid, "event": "result", "ok": False, "seconds": time.monotonic() - t0,
                        "error": f"{type(exc).__name__}: {exc}"})

    async def _run(self, rid: int, job: dict[str, Any], send) -> None:
        job_id = job.get("job_id", "?")
        with self.worker._lock:
            if self.worker.state != "ready":
                await send({"id": rid, "event": "failed",
                            "error": f"worker is '{self.worker.state}', cannot run {job_id}"})
                return
            self.worker.state = "busy"
        queue: asyncio.Queue[tuple[float, str] | None] = asyncio.Queue()
        loop = asyncio.get_running_loop()

        def report(fraction: float, message: str) -> None:
            loop.call_soon_threadsafe(queue.put_nowait, (fraction, message))

        ctx = RunContext(job_id=job_id, _report=report)
        self.worker.current = ctx
        task = asyncio.create_task(asyncio.to_thread(self.worker.run, job, ctx))
        try:
            while True:
                getter = asyncio.create_task(queue.get())
                done, _ = await asyncio.wait({task, getter}, return_when=asyncio.FIRST_COMPLETED)
                if getter in done:
                    fraction, message = getter.result()
                    await send({"id": rid, "event": "progress", "fraction": fraction, "message": message})
                    if task not in done:
                        continue
                else:
                    getter.cancel()
                # drain remaining progress, then finish
                while not queue.empty():
                    fraction, message = queue.get_nowait()
                    await send({"id": rid, "event": "progress", "fraction": fraction, "message": message})
                try:
                    result = task.result()
                except CancelledJob:
                    await send({"id": rid, "event": "failed", "error": "cancelled", "cancelled": True})
                except Exception as exc:  # noqa: BLE001
                    log.error("job %s failed:\n%s", job_id, traceback.format_exc())
                    await send({"id": rid, "event": "failed", "error": f"{type(exc).__name__}: {exc}"})
                else:
                    await send({"id": rid, "event": "done", "files": result.files, "manifest": result.manifest})
                return
        finally:
            self.worker.current = None
            with self.worker._lock:
                if self.worker.state == "busy":
                    self.worker.state = "ready"

    async def serve_forever(self) -> None:
        if os.path.exists(self.path):
            os.unlink(self.path)
        os.makedirs(os.path.dirname(self.path) or ".", exist_ok=True)
        server = await asyncio.start_unix_server(self.handle, path=self.path, limit=protocol.MAX_LINE)
        os.chmod(self.path, 0o660)
        log.info("worker '%s' listening on %s (state=%s)", self.worker.name, self.path, self.worker.state)

        async def load() -> None:
            try:
                await asyncio.to_thread(self.worker.load)
                self.worker.state = "ready"
                log.info("worker '%s' ready (backends: %s)", self.worker.name, ", ".join(self.worker.backends))
            except Exception as exc:  # noqa: BLE001
                log.error("worker '%s' failed to load:\n%s", self.worker.name, traceback.format_exc())
                self.worker.state = "error"
                self.worker.error = f"{type(exc).__name__}: {exc}"

        loader = asyncio.create_task(load())
        for sig in (signal.SIGTERM, signal.SIGINT):
            self.loop.add_signal_handler(sig, self._stop.set)
        async with server:
            await self._stop.wait()
        loader.cancel()
        await asyncio.to_thread(self.worker.shutdown)
        if os.path.exists(self.path):
            os.unlink(self.path)


def serve(worker_cls: type[Worker], parser: argparse.ArgumentParser | None = None,
          argv: list[str] | None = None) -> int:
    """Entry point for worker ``__main__`` modules."""
    parser = parser or argparse.ArgumentParser(description=worker_cls.__doc__)
    add_common_args(parser)
    args = parser.parse_args(argv)
    logging.basicConfig(level=args.log_level.upper(),
                        format="%(asctime)s %(levelname)s %(name)s: %(message)s", stream=sys.stderr)
    worker = worker_cls(args)
    if args.name:
        worker.name = args.name
    if args.backends:
        worker.backends = tuple(b.strip() for b in args.backends.split(",") if b.strip())
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(_Server(worker, args.socket).serve_forever())
    finally:
        loop.close()
    return 0
