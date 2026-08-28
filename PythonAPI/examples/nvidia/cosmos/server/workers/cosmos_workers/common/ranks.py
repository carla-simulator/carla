"""Shared ``torchrun`` rank model for the Transfer 2.5 workers (general and auto/multiview).

NVIDIA's ``Control2WorldInference`` / ``MultiviewInference`` run context-parallel: every
rank builds the pipeline under ``torch.distributed`` and every rank takes part in each
``generate`` (collectives).  The socket worker therefore never loads the model itself; it
starts a rank group once and hands jobs over through a spool directory::

    socket worker (RankSupervisor)                 rank processes (RankLoop, one per GPU)
    start()  ── spawns ─────────────────────────►  python -m <module> ...                   (nproc == 1)
                                                   torchrun --nproc_per_node N -m <module>  (nproc  > 1)
    submit() ── <spool>/requests/<job>.json ────►  rank 0 polls, broadcasts to the other ranks
                                                   (torch.distributed.broadcast_object_list),
                                                   every rank runs engine.generate (collective)
             ◄── <spool>/results/<job>.json ──────  rank 0 writes {"ok", "paths", "error"}
    stop()   ── <spool>/requests/__shutdown__.json  every rank exits
    <spool>/READY                                   engine info, written by rank 0 once loaded

``torchrun`` is taken from the worker's own venv (``<venv>/bin/torchrun``, else
``python -m torch.distributed.run``): the vllm base image ships a foreign
``/usr/local/bin/torchrun`` (Python 3.12, no cosmos deps) that a bare ``torchrun`` on
``PATH`` would resolve to.  ``--engine fake`` runs the same loop without a GPU; with more
than one rank the fake ranks rendezvous over torch's ``gloo`` backend (tests), the real
engine relies on the process group NVIDIA's pipeline initialises (NCCL).
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import subprocess
import sys
import time
import traceback
from pathlib import Path
from typing import Any, Callable

log = logging.getLogger("cosmos_worker.ranks")

READY_FILE = "READY"
SHUTDOWN_FILE = "__shutdown__.json"

HEARTBEAT_S = 5.0
"""Longest a rank may sit inside one collective while the worker is idle.

Rank 0 polls the spool and the other ranks wait for it in ``broadcast_object_list``.  On NCCL that
is an *outstanding collective*, and the ProcessGroupNCCL watchdog tears the whole process group down
when one is outstanding longer than ``TORCH_NCCL_TIMEOUT`` (30 min by default) — which is what
killed the AV multiview ranks on the 2026-08-28 node run while two Cosmos 3 jobs ran ahead of them:

    [Rank 3] Watchdog caught collective operation timeout:
    WorkNCCL(SeqNum=850, OpType=BROADCAST, NumelIn=1, Timeout(ms)=1800000)
    ... we are taking the entire process down                       -> SIGABRT, exit 1

So the idle wait is a *loop of short broadcasts* instead of one long one: every ``HEARTBEAT_S`` rank
0 broadcasts either the next job or ``None``, and no collective is ever outstanding for more than a
few seconds.  An idle worker costs one 1-element broadcast every 5 s.

Those idle broadcasts must NOT run on NCCL.  A rank waiting inside a NCCL collective keeps a kernel
resident that busy-polls the GPU: on cosmos-dev4 (2026-08-28) the three parked Transfer 2.5 ranks
showed 99.9 % CPU and 37-62 % SM on GPUs 1-3 while a Cosmos 3 Nano job ran on the same GPUs, and the
Nano job slowed from ~14 s to ~51 s per diffusion step.  The idle loop therefore broadcasts over a
separate ``gloo`` group (:func:`idle_group`) -- a socket wait on the CPU, nothing on the GPU -- and the
NCCL group is only ever used inside ``generate``.
"""


def visible_gpus() -> list[int]:
    """GPU indices this worker was given (``CUDA_VISIBLE_DEVICES`` as set by the launcher)."""
    raw = os.environ.get("CUDA_VISIBLE_DEVICES", "")
    out = []
    for tok in raw.split(","):
        tok = tok.strip()
        if tok.isdigit():
            out.append(int(tok))
    return out


def torchrun_command(nproc: int, master_port: int, module: str, args: list[str]) -> list[str]:
    """``torchrun`` from this venv (never the base image's), or plain python for a single rank."""
    if nproc <= 1:
        return [sys.executable, "-m", module, *args]
    torchrun = os.path.join(os.path.dirname(sys.executable), "torchrun")
    launcher = [torchrun] if os.path.exists(torchrun) else [sys.executable, "-m", "torch.distributed.run"]
    return [*launcher, "--nproc_per_node", str(nproc), "--master_port", str(master_port), "-m", module, *args]


# ----------------------------------------------------------------------------- worker side

class RankSupervisor:
    """Owns the rank process group of one worker and the spool it is served through."""

    def __init__(self, module: str, spool: Path, nproc: int, master_port: int, rank_args: list[str],
                 startup_timeout: float = 3600.0, request_timeout: float = 7200.0, poll: float = 1.0,
                 env: dict[str, str] | None = None) -> None:
        self.module = module
        self.spool = Path(spool)
        self.nproc = max(1, int(nproc))
        self.master_port = master_port
        self.rank_args = list(rank_args)
        self.startup_timeout = startup_timeout
        self.request_timeout = request_timeout
        self.poll = poll
        self.env = dict(env or {})
        self.proc: subprocess.Popen | None = None
        self.info: dict[str, Any] = {}

    @property
    def command(self) -> list[str]:
        return torchrun_command(self.nproc, self.master_port, self.module, ["--spool", str(self.spool), *self.rank_args])

    @property
    def alive(self) -> bool:
        return self.proc is not None and self.proc.poll() is None

    def start(self) -> dict[str, Any]:
        """Spawn the ranks and block until rank 0 wrote ``READY``; returns the engine info."""
        for d in ("requests", "results", "work"):
            (self.spool / d).mkdir(parents=True, exist_ok=True)
        ready = self.spool / READY_FILE
        if ready.exists():
            ready.unlink()
        for stale in (self.spool / "requests").glob("*.json"):
            stale.unlink()
        cmd = self.command
        env = {**os.environ, **self.env}
        log.info("starting %d rank(s): %s", self.nproc, " ".join(cmd))
        self.proc = subprocess.Popen(cmd, env=env, stdout=sys.stderr, stderr=subprocess.STDOUT)
        t0 = time.monotonic()
        while not ready.exists():
            if self.proc.poll() is not None:
                raise RuntimeError(f"ranks ({self.module}) exited with code {self.proc.returncode} during start-up")
            if time.monotonic() - t0 > self.startup_timeout:
                self.proc.terminate()
                raise RuntimeError(f"ranks ({self.module}) not ready after {self.startup_timeout:.0f}s")
            time.sleep(min(1.0, self.poll))
        self.info = json.loads(ready.read_text())
        log.info("ranks ready after %.0fs: %s", time.monotonic() - t0, self.info)
        return self.info

    def submit(self, job_id: str, payload: dict[str, Any],
               on_wait: Callable[[float], None] | None = None) -> dict[str, Any]:
        """Queue ``payload`` for the ranks and block for the result (``on_wait(elapsed)`` every poll)."""
        if self.proc is not None and self.proc.poll() is not None:
            # The ranks died while nothing was running (see HEARTBEAT_S).  Bring them back rather
            # than failing a job that has nothing to do with whatever killed them.
            log.warning("ranks (%s) were dead (exit %s) before job %s; restarting them",
                        self.module, self.proc.returncode, job_id)
            self.start()
        req = self.spool / "requests" / f"{job_id}.json"
        res = self.spool / "results" / f"{job_id}.json"
        if res.exists():
            res.unlink()
        tmp = req.with_suffix(".tmp")
        tmp.write_text(json.dumps(payload))
        os.replace(tmp, req)
        t0 = time.monotonic()
        while not res.exists():
            if self.proc is not None and self.proc.poll() is not None:
                raise RuntimeError(f"ranks died (exit {self.proc.returncode}) during job {job_id}; "
                                   f"the rank traceback is in this worker's log")
            elapsed = time.monotonic() - t0
            if elapsed > self.request_timeout:
                raise TimeoutError(f"generation still running after {elapsed:.0f}s")
            if on_wait is not None:
                on_wait(elapsed)
            time.sleep(self.poll)
        result = json.loads(res.read_text())
        res.unlink()
        result["wall_s"] = time.monotonic() - t0
        return result

    def stop(self, grace: float = 30.0) -> None:
        if self.proc is None or self.proc.poll() is not None:
            return
        try:
            (self.spool / "requests" / SHUTDOWN_FILE).write_text("{}")
            self.proc.wait(grace)
        except (OSError, subprocess.TimeoutExpired):
            self.proc.terminate()
            try:
                self.proc.wait(grace)
            except subprocess.TimeoutExpired:
                self.proc.kill()


# ----------------------------------------------------------------------------- rank side

class RankEngine:
    """What a rank loop needs from a model: ``info`` for READY and a collective ``generate``."""

    info: dict[str, Any] = {}

    def generate(self, sample: dict[str, Any], out_dir: Path) -> list[str] | str | None:
        """Return the output path(s); ``None`` / ``[]`` when the guardrails blocked the sample."""
        raise NotImplementedError


def add_rank_args(p: argparse.ArgumentParser) -> argparse.ArgumentParser:
    p.add_argument("--spool", required=True)
    p.add_argument("--engine", choices=["real", "fake"], default="real")
    p.add_argument("--context-parallel-size", type=int, default=int(os.environ.get("WORLD_SIZE", "1")))
    p.add_argument("--guardrails", action="store_true", default=True)
    p.add_argument("--no-guardrails", dest="guardrails", action="store_false")
    p.add_argument("--offload-guardrails", action="store_true", default=True)
    p.add_argument("--fake-delay", type=float, default=0.3)
    p.add_argument("--poll", type=float, default=0.5)
    return p


def distributed_state(engine: str) -> tuple[int, int, Any]:
    """``(rank, world_size, torch.distributed | None)`` for this process.

    Called *after* the engine is built: with the real engine NVIDIA's pipeline has already
    initialised the (NCCL) process group for context parallelism; the fake engine rendezvous
    over ``gloo`` so multi-rank tests run on CPU.  A single rank never imports torch.
    """
    world = int(os.environ.get("WORLD_SIZE", "1"))
    if world <= 1:
        return 0, 1, None
    import torch.distributed as dist  # noqa: PLC0415 - only the multi-rank path needs torch

    if not dist.is_initialized():
        dist.init_process_group(backend="gloo" if engine == "fake" else "nccl")
    return dist.get_rank(), dist.get_world_size(), dist


def idle_group(dist: Any):
    """The process group the idle heartbeat broadcasts on: ``gloo`` when the main group is NCCL.

    Every rank must call this once, before the loop (``new_group`` is itself collective).  Returns
    ``None`` (= the default group) when there is no distributed state or the main group already is
    gloo (the fake engine in tests), so single-rank and CPU runs are unchanged.
    """
    if dist is None or dist.get_backend() != "nccl":
        return None
    return dist.new_group(backend="gloo")


def _normalise(paths: list[str] | str | None) -> list[str]:
    if not paths:
        return []
    return [str(paths)] if isinstance(paths, (str, Path)) else [str(p) for p in paths]


def run_rank_loop(args: argparse.Namespace, engine: RankEngine, extra_info: dict[str, Any] | None = None) -> int:
    """Serve spool requests until ``__shutdown__`` (see the module docstring)."""
    spool = Path(args.spool)
    req_dir, res_dir = spool / "requests", spool / "results"
    rank, world, dist = distributed_state(args.engine)
    idle = idle_group(dist)
    if rank == 0:
        req_dir.mkdir(parents=True, exist_ok=True)
        res_dir.mkdir(parents=True, exist_ok=True)
        info = {**engine.info, "world_size": world, "gpus": visible_gpus(), **(extra_info or {})}
        tmp = spool / (READY_FILE + ".tmp")
        tmp.write_text(json.dumps(info))
        os.replace(tmp, spool / READY_FILE)
        log.info("ranks ready (world %d)", world)

    while True:
        payload: dict[str, Any] | None = None
        while payload is None:
            if rank == 0:
                payload = _poll_request(req_dir, args.poll, HEARTBEAT_S)
            if dist is None:
                continue
            box = [payload]
            # completes within HEARTBEAT_S, idle or not -- and on gloo, so no GPU kernel is
            # left spinning while the worker waits (see HEARTBEAT_S)
            dist.broadcast_object_list(box, src=0, group=idle)
            payload = box[0]
        if payload["__file"] == SHUTDOWN_FILE:
            log.info("shutdown requested")
            if dist is not None:
                dist.barrier()
            return 0
        result: dict[str, Any]
        try:
            paths = _normalise(engine.generate(payload["sample"], Path(payload["out_dir"])))
            result = {"ok": bool(paths), "paths": paths,
                      "error": None if paths else "blocked by Cosmos guardrails (prompt or generated video)"}
        except Exception as exc:  # noqa: BLE001
            log.error("generation failed:\n%s", traceback.format_exc())
            result = {"ok": False, "paths": [], "error": _describe_failure(exc, rank, world, payload["sample"])}
        if rank == 0:
            tmp = res_dir / (payload["__file"] + ".tmp")
            tmp.write_text(json.dumps(result))
            os.replace(tmp, res_dir / payload["__file"])
        if dist is not None:
            dist.barrier()


def _poll_request(req_dir: Path, poll: float, budget: float) -> dict[str, Any] | None:
    """The oldest queued request, or ``None`` after ``budget`` seconds of nothing."""
    deadline = time.monotonic() + budget
    while True:
        files = sorted(req_dir.glob("*.json"), key=lambda f: f.stat().st_mtime)
        if files:
            f = files[0]
            try:
                payload = json.loads(f.read_text())
            except json.JSONDecodeError:
                time.sleep(0.1)  # half-written; retry
                continue
            payload["__file"] = f.name
            f.unlink()
            return payload
        if time.monotonic() >= deadline:
            return None
        time.sleep(min(poll, max(0.0, deadline - time.monotonic())))


def _describe_failure(exc: BaseException, rank: int, world: int, sample: dict[str, Any]) -> str:
    """``ExcType: message``; a CUDA OOM names the memory (the real limit) rather than leaving a bare torch error."""
    text = f"{type(exc).__name__}: {exc}"
    if "out of memory" in text.lower() or type(exc).__name__ == "OutOfMemoryError":
        views = [k for k in ("front_wide", "cross_left", "cross_right", "rear_left", "rear_right", "rear", "front_tele")
                 if isinstance(sample.get(k), dict)]
        what = f"{len(views)} view(s)" if views else f"resolution {sample.get('resolution', '?')}"
        return (f"OutOfMemoryError: GPU memory exhausted on rank {rank}/{world} generating {what} x "
                f"{sample.get('num_steps', '?')} steps; give this worker more GPUs (context parallel) or fewer "
                f"views/frames per job — {text.splitlines()[0][:300]}")
    return text


def rank_main(argv: list[str] | None, build_parser: Callable[[], argparse.ArgumentParser],
              make_engine: Callable[[argparse.Namespace], RankEngine]) -> int:
    """Entry point for a ``*.ranks`` module: parse, build the engine, serve."""
    args = add_rank_args(build_parser()).parse_args(argv)
    logging.basicConfig(level="INFO", format="%(asctime)s %(levelname)s rank%(process)d %(name)s: %(message)s",
                        stream=sys.stderr)
    return run_rank_loop(args, make_engine(args))


def is_rank_zero() -> bool:
    return os.environ.get("RANK", "0") == "0"
