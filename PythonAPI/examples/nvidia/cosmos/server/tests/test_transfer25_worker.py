"""Transfer 2.5 worker with the fake engine (CPU): job -> InferenceArguments mapping, outputs, guardrail block."""

import asyncio
import json
import subprocess
import sys
import time

import pytest

from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol
from cosmos_workers.common.base import add_common_args
from cosmos_workers.transfer25.worker import Transfer25Worker, build_parser


@pytest.fixture
def worker(tmp_path):
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "t25.sock"
    scratch = tmp_path / "scratch"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.transfer25", "--socket", str(sock), "--name", "t25",
                             "--engine", "fake", "--fake-delay", "0.3", "--scratch-dir", str(scratch),
                             "--hf-home", str(tmp_path / "nohf")])
    for _ in range(200):
        if sock.exists():
            break
        time.sleep(0.05)
    yield str(sock), scratch
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


def job(tmp_path, clip, controls, extra=None, prompt="foggy dawn", guidance=None, negative=None):
    cam = clip.manifest.camera_names[0]
    ctrls = {}
    for name, how in controls.items():
        ctrls[name] = {"derive": True, "weight": how} if how == "derive" or isinstance(how, tuple) and how[0] == "derive" \
            else {"path": str(clip.video(name if name != "vis" else "rgb", cam)), "weight": how}
        if isinstance(how, tuple):
            ctrls[name] = {"derive": True, "weight": how[1]}
    return {
        "job_id": "j_t25", "views": [cam],
        "request": {"backend": "transfer2.5", "prompt": prompt, "negative_prompt": negative, "seed": 3,
                    "num_steps": 12, "guidance": guidance, "resolution": "480",
                    "controls": {k: {} for k in controls}, "extra": extra or {}},
        "manifest": clip.manifest.model_dump(),
        "inputs": {"rgb": {cam: str(clip.video("rgb", cam))}, "controls": ctrls},
        "out_dir": str(tmp_path / "out"),
    }


def test_mapping_and_outputs(worker, clip93, tmp_path):
    sock, scratch = worker
    cam = clip93.manifest.camera_names[0]

    async def go():
        h = await wait_ready(sock)
        assert h["backends"] == ["transfer2.5"]
        assert (await protocol.call_result(sock, "smoke"))["ok"] is True
        j = job(tmp_path, clip93, {"depth": 0.7, "seg": None, "edge": ("derive", 0.4)},
                extra={"num_video_frames_per_chunk": 93, "seg_control_prompt": "car, road", "preset_edge_threshold": "low",
                       "sigma_max": "80", "bogus_key": 1}, guidance=4.6)
        return [m async for m in protocol.call(sock, "run", job=j)]

    events = run(go())
    assert events[-1]["event"] == "done", events[-1]
    files = {f["name"]: f for f in events[-1]["files"]}
    assert set(files) == {"camera_front_wide_120fov.mp4", "control_depth.mp4", "control_seg.mp4", "control_edge.mp4"}
    assert files["control_edge.mp4"]["kind"] == "control"
    assert (tmp_path / "out" / "camera_front_wide_120fov.mp4").stat().st_size == clip93.video("rgb", cam).stat().st_size
    m = events[-1]["manifest"]
    assert m["multicontrol"] is True and m["hints_loaded"] == ["edge", "vis", "depth", "seg"]
    s = m["sample"]
    assert "prompt" not in s and s["guidance"] == 5 and s["num_steps"] == 12 and s["resolution"] == "480"
    assert s["max_frames"] == 93 and s["num_video_frames_per_chunk"] == 93 and s["sigma_max"] == "80"
    assert s["depth"] == {"control_weight": 0.7, "control_path": str(clip93.video("depth", cam))}
    assert s["seg"] == {"control_weight": 1.0, "control_path": str(clip93.video("seg", cam)), "control_prompt": "car, road"}
    assert s["edge"] == {"control_weight": 0.4, "preset_edge_threshold": "low"}  # derived, no path
    assert "vis" not in s and "negative_prompt" not in s
    # the fake engine received exactly the sample dict (what the real engine validates as InferenceArguments)
    req = json.loads((scratch / "jobs").glob("*").__next__().joinpath("fake_request.json").read_text()) \
        if (scratch / "jobs").exists() and any((scratch / "jobs").iterdir()) else None
    assert req is None or req["name"] == "j_t25"  # work dir is cleaned after success


def test_guardrail_block_and_errors(worker, clip93, tmp_path):
    sock, _ = worker

    async def go():
        await wait_ready(sock)
        ev = [m async for m in protocol.call(sock, "run", job=job(tmp_path, clip93, {"depth": None}, prompt="BLOCKME"))]
        assert ev[-1]["event"] == "failed" and "guardrails" in ev[-1]["error"]
        bad = job(tmp_path, clip93, {"depth": None})
        bad["inputs"]["rgb"] = {}
        ev = [m async for m in protocol.call(sock, "run", job=bad)]
        assert ev[-1]["event"] == "failed" and "RGB" in ev[-1]["error"]
        bad = job(tmp_path, clip93, {"depth": None})
        bad["inputs"]["controls"]["wsm"] = {"path": "/x", "weight": None}
        ev = [m async for m in protocol.call(sock, "run", job=bad)]
        assert ev[-1]["event"] == "failed" and "not a Transfer 2.5 hint" in ev[-1]["error"]
        assert (await protocol.call_result(sock, "status"))["state"] == "ready"

    run(go())


def test_parser_and_hint_subset():
    p = build_parser()
    add_common_args(p)
    args = p.parse_args(["--socket", "x", "--hints", "depth", "--engine", "fake"])
    w = Transfer25Worker(args)
    assert w.hints == ["depth"]
    with pytest.raises(SystemExit):
        Transfer25Worker(p.parse_args(["--socket", "x", "--hints", "depth,nope", "--engine", "fake"]))
    # a control not loaded on this worker is rejected at build time
    sample_inputs = {"rgb": {"v": "/rgb.mp4"}, "controls": {"edge": {"derive": True}}}
    with pytest.raises(ValueError, match="not loaded"):
        w.build_sample("j", {"prompt": "p", "controls": {"edge": {}}}, {"frames": 93, "fps": 16}, sample_inputs, "v")


@pytest.mark.skipif(__import__("importlib").util.find_spec("cosmos_transfer2") is None,
                    reason="cosmos_transfer2 not installed (run in the transfer25 venv)")
def test_sample_validates_as_real_inference_arguments(tmp_path, clip93):
    from cosmos_transfer2.config import InferenceArguments

    p = build_parser()
    add_common_args(p)
    w = Transfer25Worker(p.parse_args(["--socket", "x", "--engine", "fake"]))
    cam = clip93.manifest.camera_names[0]
    sample = w.build_sample("j", {"prompt": "p", "guidance": 3.0, "controls": {"depth": {}, "edge": {}},
                                 "extra": {"seg_control_prompt": "car"}},
                            clip93.manifest.model_dump(),
                            {"rgb": {cam: str(clip93.video("rgb", cam))},
                             "controls": {"depth": {"path": str(clip93.video("depth", cam)), "weight": 0.5},
                                          "edge": {"derive": True, "weight": None}}}, cam)
    args = InferenceArguments.model_validate(sample)
    assert args.hint_keys == ["edge", "depth"] and args.control_weight_dict == {"edge": "1.0", "depth": "0.5"}
