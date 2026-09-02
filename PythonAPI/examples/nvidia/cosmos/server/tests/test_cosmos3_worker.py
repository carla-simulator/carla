"""Cosmos 3 worker against the fake vLLM-Omni server (CPU only)."""

import asyncio
import json
import subprocess
import sys
import time
from pathlib import Path

import pytest

from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol
from cosmos_workers.common.base import add_common_args
from cosmos_workers.cosmos3.worker import Cosmos3Worker, _resolve_model, build_parser

FAKE = Path(__file__).with_name("fake_vllm_omni.py")


@pytest.fixture
def worker(tmp_path):
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "c3.sock"
    storage = tmp_path / "vllm-out"
    cmd = f"{sys.executable} {FAKE} --port {{port}} --storage {storage} --delay 0.8"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.cosmos3", "--socket", str(sock), "--name", "c3",
                             "--backends", "cosmos3-nano", "--vllm-cmd", cmd, "--model", "nvidia/Cosmos3-Nano",
                             "--hf-home", str(tmp_path / "nohf"), "--poll", "0.1", "--storage-dir", str(storage),
                             "--startup-timeout", "30", "--no-guardrails"])
    for _ in range(200):
        if sock.exists():
            break
        time.sleep(0.05)
    yield str(sock), storage
    proc.terminate()
    proc.wait(15)


def run(coro):
    return asyncio.new_event_loop().run_until_complete(coro)


async def wait_ready(sock):
    for _ in range(300):
        h = await protocol.call_result(sock, "hello")
        if h["state"] == "ready":
            return h
        if h["state"] == "error":
            raise RuntimeError(h["error"])
        await asyncio.sleep(0.1)
    raise TimeoutError


def requests_log(storage: Path) -> list[dict]:
    return [json.loads(l) for l in (storage / "requests.jsonl").read_text().splitlines()]


def job(tmp_path, clip, controls, extra=None, prompt="a rainy evening", views=None, resolution="480"):
    cam = clip.manifest.camera_names[0]
    inputs_ctrl = {}
    for name, how in controls.items():
        if how == "derive":
            inputs_ctrl[name] = {"derive": True, "weight": None}
        else:
            inputs_ctrl[name] = {"path": str(clip.video(name, cam)), "weight": how}
    return {
        "job_id": "j_test", "views": views or [cam],
        "request": {"backend": "cosmos3-nano", "prompt": prompt, "seed": 7, "num_steps": 20, "guidance": 3.0,
                    "resolution": resolution, "controls": {k: {} for k in controls}, "extra": extra or {}},
        "manifest": clip.manifest.model_dump(),
        "inputs": {"rgb": {cam: str(clip.video("rgb", cam))}, "controls": inputs_ctrl},
        "out_dir": str(tmp_path / "out"),
    }


def test_smoke_and_transfer_request_mapping(worker, clip16, tmp_path):
    sock, storage = worker

    async def go():
        h = await wait_ready(sock)
        assert h["backends"] == ["cosmos3-nano"]
        s = await protocol.call_result(sock, "smoke")
        assert s["ok"] is True, s
        events = [m async for m in protocol.call(sock, "run", job=job(
            tmp_path, clip16, {"depth": None, "seg": 0.5, "edge": "derive"},
            extra={"control_guidance": 2.0, "num_video_frames_per_chunk": 16, "preset_edge_threshold": "high"}))]
        return events

    events = run(go())
    assert events[-1]["event"] == "done", events[-1]
    assert any(e["event"] == "progress" and "vLLM-Omni" in e["message"] for e in events)
    done = events[-1]
    assert done["files"][0]["name"] == "camera_front_wide_120fov.mp4"
    out = tmp_path / "out" / done["files"][0]["name"]
    assert out.stat().st_size == clip16.video("rgb", clip16.manifest.camera_names[0]).stat().st_size  # echoed RGB
    m = done["manifest"]
    assert m["backend_impl"] == "vllm-omni" and m["input_reference"] is True and m["inference_time_s"] == 0.8

    reqs = [r for r in requests_log(storage) if r["event"] == "submit"]
    smoke, real = reqs[0], reqs[1]
    assert smoke["extra"]["depth"]["control_path"].endswith("depth.png") and smoke["extra"]["resolution"] == "256"
    assert smoke["fields"]["num_frames"] == "5" and smoke["fields"]["num_inference_steps"] == "2"
    f, x = real["fields"], real["extra"]
    assert f["model"] == "nvidia/Cosmos3-Nano" and f["prompt"] == "a rainy evening"
    assert f["num_frames"] == "16" and f["fps"] == "16" and f["seed"] == "7"
    assert f["num_inference_steps"] == "20" and f["guidance_scale"] == "3.0"
    assert real["files"] == ["input_reference"]
    assert x["depth"] == {"control_path": str(clip16.video("depth", clip16.manifest.camera_names[0])), "control_weight": 1.0}
    assert x["seg"]["control_weight"] == 0.5
    assert x["edge"] == {"control_weight": 1.0, "preset_edge_threshold": "high"}  # derived: no control_path
    assert x["resolution"] == "480" and x["max_frames"] == 16
    assert x["control_guidance"] == 2.0 and x["num_video_frames_per_chunk"] == 16
    assert "guardrails" not in x  # server started with --no-guardrails
    # outputs are deleted from the vLLM store after download
    assert any(r["event"] == "delete" and r["status_at_delete"] == "completed" for r in requests_log(storage))


def test_guardrail_block_and_cancel(worker, clip16, tmp_path):
    sock, storage = worker

    async def go():
        await wait_ready(sock)
        blocked = [m async for m in protocol.call(sock, "run", job=job(tmp_path, clip16, {"depth": None},
                                                                          prompt="BLOCKME please"))]
        assert blocked[-1]["event"] == "failed" and "guardrails" in blocked[-1]["error"]

        j = job(tmp_path, clip16, {"depth": None})
        j["job_id"] = "j_cancel"

        async def runner():
            return [m async for m in protocol.call(sock, "run", job=j)]

        t = asyncio.create_task(runner())
        await asyncio.sleep(0.3)
        c = await protocol.call_result(sock, "cancel", job_id="j_cancel")
        assert c["cancelled"] is True
        ev = await t
        assert ev[-1]["event"] == "failed" and ev[-1].get("cancelled") is True
        assert any(r["event"] == "delete" and r["status_at_delete"] != "completed" for r in requests_log(storage))
        # error paths in request building
        bad = job(tmp_path, clip16, {"depth": None})
        bad["inputs"]["controls"]["wsm"] = {"scene_dir": "/nope", "weight": None}
        bad["request"]["controls"]["wsm"] = {}
        ev = [m async for m in protocol.call(sock, "run", job=bad)]
        assert ev[-1]["event"] == "failed" and "scene package" in ev[-1]["error"]
        assert (await protocol.call_result(sock, "status"))["state"] == "ready"

    run(go())


def test_resolve_model_and_command(tmp_path):
    hf = tmp_path / "hf"
    snap = hf / "hub" / "models--nvidia--Cosmos3-Nano" / "snapshots" / "abc123"
    snap.mkdir(parents=True)
    (snap.parent.parent / "refs").mkdir()
    (snap.parent.parent / "refs" / "main").write_text("abc123\n")
    assert _resolve_model("nvidia/Cosmos3-Nano", str(hf)) == (str(snap), "abc123", True)
    assert _resolve_model("nvidia/Cosmos3-Super", str(hf)) == ("nvidia/Cosmos3-Super", None, False)
    assert _resolve_model(str(snap), str(hf)) == (str(snap), "abc123", True)

    parser = build_parser()
    add_common_args(parser)
    args = parser.parse_args(["--socket", "x", "--model", "nvidia/Cosmos3-Super", "--tp", "4",
                                      "--hf-home", str(hf), "--vllm-arg", "--quantization fp8", "--no-guardrails"])
    w = Cosmos3Worker(args)
    cmd = w._vllm_command()
    assert cmd[:3] == ["vllm", "serve", "nvidia/Cosmos3-Super"] and "--omni" in cmd
    assert cmd[cmd.index("--tensor-parallel-size") + 1] == "4"
    assert "--no-guardrails" in cmd and "--vae-use-tiling" in cmd and "--quantization" in cmd
    assert cmd[cmd.index("--served-model-name") + 1] == "nvidia/Cosmos3-Super"
    assert w.offline is False


def test_rendered_control_comes_back(worker, clip16, tmp_path):
    """A control the API server rendered (e.g. wsm from a scene package) is returned as control_<hint>.mp4."""
    sock, _ = worker
    j = job(tmp_path, clip16, {"depth": None})
    j["inputs"]["controls"]["depth"]["rendered"] = {"renderer": "fake"}  # stands in for a rendered wsm video

    async def go():
        await wait_ready(sock)
        return [m async for m in protocol.call(sock, "run", job=j)]

    done = run(go())[-1]
    assert done["event"] == "done", done
    assert [(f["name"], f["kind"]) for f in done["files"]] == [
        ("camera_front_wide_120fov.mp4", "video"), ("control_depth.mp4", "control")]
    assert (tmp_path / "out" / "control_depth.mp4").stat().st_size == clip16.video("depth", clip16.manifest.camera_names[0]).stat().st_size
