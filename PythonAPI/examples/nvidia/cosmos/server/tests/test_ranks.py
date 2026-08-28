"""Shared torchrun rank supervisor (``cosmos_workers.common.ranks``) with the fake engines on CPU.

The multi-rank cases launch two real ``torchrun`` ranks that rendezvous over gloo; they need torch in the
test interpreter (the model venvs have it; a bare API venv does not -> skipped with the reason).
"""

from __future__ import annotations

import asyncio
import importlib.util
import json
import os
import subprocess
import sys
import time
from pathlib import Path

import pytest

from carla_cosmos.synthetic import av7_clip
from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol
from cosmos_workers.common.ranks import (RankSupervisor, _describe_failure, _poll_request,
                                         torchrun_command, visible_gpus)
from cosmos_workers.transfer25_av.common import CAMERA_KEYS

HAS_TORCH = importlib.util.find_spec("torch") is not None
needs_torch = pytest.mark.skipif(not HAS_TORCH, reason="torch not importable here: torchrun multi-rank test needs it")


def test_torchrun_command_prefers_venv_binary(monkeypatch, tmp_path):
    assert torchrun_command(1, 12345, "m.ranks", ["--x"]) == [sys.executable, "-m", "m.ranks", "--x"]
    cmd = torchrun_command(2, 12345, "m.ranks", ["--x"])
    venv_torchrun = os.path.join(os.path.dirname(sys.executable), "torchrun")
    if os.path.exists(venv_torchrun):
        assert cmd[0] == venv_torchrun
    else:  # never a bare "torchrun" from PATH (the base image ships a foreign one)
        assert cmd[:3] == [sys.executable, "-m", "torch.distributed.run"]
    assert cmd[-5:] == ["--master_port", "12345", "-m", "m.ranks", "--x"]
    assert cmd[cmd.index("--nproc_per_node") + 1] == "2"


def test_visible_gpus(monkeypatch):
    monkeypatch.setenv("CUDA_VISIBLE_DEVICES", "2, 3,x")
    assert visible_gpus() == [2, 3]
    monkeypatch.setenv("CUDA_VISIBLE_DEVICES", "")
    assert visible_gpus() == []


def test_oom_message_names_memory_not_views():
    msg = _describe_failure(RuntimeError("CUDA out of memory. Tried to allocate 2 GiB"), 1, 4,
                            {"front_wide": {}, "rear": {}, "num_steps": 35})
    assert msg.startswith("OutOfMemoryError: GPU memory exhausted on rank 1/4 generating 2 view(s) x 35 steps")
    assert "more GPUs" in msg and "CUDA out of memory" in msg
    assert _describe_failure(ValueError("bad"), 0, 1, {}) == "ValueError: bad"


@needs_torch
def test_transfer25_supervisor_two_ranks_cpu(tmp_path, clip93):
    """Two fake Transfer 2.5 ranks under torchrun: rank 0 serves the spool, both see every request."""
    spool = tmp_path / "spool"
    sup = RankSupervisor("cosmos_workers.transfer25.ranks", spool, nproc=2, master_port=29517,
                         rank_args=["--engine", "fake", "--fake-delay", "0.05", "--hints", "depth,edge",
                                    "--context-parallel-size", "2"], startup_timeout=120, request_timeout=60, poll=0.1)
    try:
        info = sup.start()
        assert info["world_size"] == 2 and info["hints_loaded"] == ["depth", "edge"]
        cam = clip93.manifest.camera_names[0]
        sample = {"name": "j1", "prompt": "p", "video_path": str(clip93.video("rgb", cam)), "num_steps": 1,
                  "depth": {"control_weight": 1.0, "control_path": str(clip93.video("depth", cam))}}
        ticks = []
        res = sup.submit("j1", {"sample": sample, "out_dir": str(spool / "work" / "j1")}, ticks.append)
        assert res["ok"] and Path(res["paths"][0]).exists() and (spool / "work" / "j1" / "j1_control_depth.mp4").exists()
        seen = json.loads((spool / "work" / "j1" / "fake_request.json").read_text())
        assert seen["world_size"] == "2" and seen["rank"] == "0"  # written by rank 0 of a 2-rank group
        blocked = sup.submit("j2", {"sample": {**sample, "name": "j2", "prompt": "BLOCKME"},
                                    "out_dir": str(spool / "work" / "j2")})
        assert blocked["ok"] is False and "guardrails" in blocked["error"]
        assert sup.alive
    finally:
        sup.stop()
    assert not sup.alive


@needs_torch
def test_av_worker_seven_views_on_two_torchrun_ranks(tmp_path):
    """7 views on nproc 2 (views are independent of the rank count) through the socket worker + torchrun."""
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "av.sock"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.transfer25_av", "--socket", str(sock), "--name", "av",
                             "--engine", "fake", "--fake-torchrun", "--fake-delay", "0.05", "--nproc", "2",
                             "--master-port", "29518", "--poll", "0.1", "--skip-smoke",
                             "--spool-dir", str(tmp_path / "spool"), "--hf-home", str(tmp_path / "nohf")])
    try:
        clip = av7_clip(tmp_path / "clips", seconds=1)
        views = clip.manifest.camera_names
        assert len(views) == 7
        job = {
            "job_id": "j7", "views": views,
            "request": {"backend": "transfer2.5-av", "prompt": "night", "seed": 1, "num_steps": 2,
                        "controls": {"hdmap_bbox": {}}, "extra": {}},
            "manifest": clip.manifest.model_dump(),
            "inputs": {"rgb": {}, "controls": {"hdmap_bbox": {"paths": {v: str(clip.video("depth", v)) for v in views},
                                                              "weight": None}}},
            "out_dir": str(tmp_path / "out"),
        }

        async def go():
            for _ in range(1200):
                if sock.exists():
                    try:
                        h = await protocol.call_result(str(sock), "hello")
                    except (OSError, protocol.ProtocolError):
                        h = {"state": "starting"}
                    if h["state"] == "ready":
                        break
                    if h["state"] == "error":
                        raise RuntimeError(h["error"])
                await asyncio.sleep(0.1)
            else:
                raise TimeoutError("worker never became ready")
            return [m async for m in protocol.call(str(sock), "run", job=job)]

        ev = asyncio.new_event_loop().run_until_complete(go())
        assert ev[-1]["event"] == "done", ev[-1]
        m = ev[-1]["manifest"]
        assert m["world_size"] == 2 and m["nproc"] == 2 and m["parallel"] == {"nproc": 2, "context": 2}
        assert sorted(f["view"] for f in ev[-1]["files"] if f["view"]) == sorted(views)
        assert sorted(m["sample"][CAMERA_KEYS[v]]["control_path"] for v in views) == \
            sorted(str(clip.video("depth", v)) for v in views)
    finally:
        proc.terminate()
        proc.wait(30)


def test_poll_request_returns_none_after_its_budget(tmp_path):
    """The idle wait is bounded, so the ranks keep completing collectives (see ``HEARTBEAT_S``)."""
    req = tmp_path / "requests"
    req.mkdir()
    t0 = time.monotonic()
    assert _poll_request(req, poll=0.02, budget=0.15) is None
    assert 0.1 <= time.monotonic() - t0 < 2.0
    (req / "j1.json").write_text(json.dumps({"sample": {}, "out_dir": str(tmp_path)}))
    got = _poll_request(req, poll=0.02, budget=5.0)
    assert got is not None and got["__file"] == "j1.json"
    assert not (req / "j1.json").exists()          # claimed, so a second rank loop cannot take it


def test_supervisor_restarts_ranks_that_died_while_idle(tmp_path, clip93):
    """A job must not fail because the ranks died before it was submitted (NCCL watchdog, 2026-08-28)."""
    spool = tmp_path / "spool"
    sup = RankSupervisor("cosmos_workers.transfer25.ranks", spool, nproc=1, master_port=29519,
                         rank_args=["--engine", "fake", "--fake-delay", "0.05", "--hints", "depth"],
                         startup_timeout=120, request_timeout=60, poll=0.1)
    try:
        sup.start()
        first = sup.proc
        sup.proc.kill()
        sup.proc.wait(30)
        assert not sup.alive
        cam = clip93.manifest.camera_names[0]
        sample = {"name": "j1", "prompt": "p", "video_path": str(clip93.video("rgb", cam)), "num_steps": 1,
                  "depth": {"control_weight": 1.0, "control_path": str(clip93.video("depth", cam))}}
        res = sup.submit("j1", {"sample": sample, "out_dir": str(spool / "work" / "j1")})
        assert res["ok"] and sup.alive and sup.proc is not first
    finally:
        sup.stop()
