import asyncio
import json
import subprocess
import sys
import time

import pytest

from carla_cosmos.synthetic import av7_clip
from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol
from cosmos_workers.common.base import add_common_args
from cosmos_workers.transfer25_av.common import CAMERA_KEYS
from cosmos_workers.transfer25_av.worker import Transfer25AVWorker, build_parser


@pytest.fixture
def worker(tmp_path):
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "av.sock"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.transfer25_av", "--socket", str(sock), "--name", "av",
                             "--engine", "fake", "--fake-delay", "0.2", "--nproc", "8", "--poll", "0.1",
                             "--spool-dir", str(tmp_path / "spool"), "--hf-home", str(tmp_path / "nohf")])
    for _ in range(200):
        if sock.exists():
            break
        time.sleep(0.05)
    yield str(sock), tmp_path / "spool"
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


def job(tmp_path, clip, views, rgb=True, extra=None, prompt="dusk, light rain", weight=0.8, job_id="j_av"):
    paths = {v: str(clip.video("depth", v)) for v in views}  # stand-in for rendered hdmap_bbox videos
    return {
        "job_id": job_id, "views": views,
        "request": {"backend": "transfer2.5-av", "prompt": prompt, "seed": 11, "num_steps": 20, "guidance": 2.0,
                    "controls": {"hdmap_bbox": {}}, "extra": extra or {}},
        "manifest": clip.manifest.model_dump(),
        "inputs": {"rgb": {v: str(clip.video("rgb", v)) for v in views} if rgb else {},
                   "controls": {"hdmap_bbox": {"paths": paths, "weight": weight}}},
        "out_dir": str(tmp_path / "out" / job_id),
    }


def test_seven_views_autoregressive(worker, tmp_path):
    sock, spool = worker
    clip = av7_clip(tmp_path / "clips", seconds=3)  # 3*(29+28*2)=255 frames @30 -> 85 @10 -> k=3
    views = clip.manifest.camera_names

    async def go():
        h = await wait_ready(sock)
        assert h["backends"] == ["transfer2.5-av"]
        assert (await protocol.call_result(sock, "smoke"))["ok"] is True
        return [m async for m in protocol.call(sock, "run", job=job(tmp_path, clip, views, rgb=False))]

    ev = run(go())
    assert ev[-1]["event"] == "done", ev[-1]
    files = ev[-1]["files"]
    assert sorted(f["view"] for f in files if f["view"]) == sorted(views)
    assert any(f["name"] == "grid.mp4" for f in files)
    for f in files:
        assert (tmp_path / "out" / "j_av" / f["name"]).stat().st_size > 0
    m = ev[-1]["manifest"]
    s = m["sample"]
    assert m["nproc"] == 8 and m["frames_at_model_fps"] == 85 and m["engine"] == "fake-multiview"
    assert s["enable_autoregressive"] is True and s["num_chunks"] == 3 and s["chunk_overlap"] == 1
    assert s["num_conditional_frames"] == 0 and s["control_weight"] == 0.8 and s["guidance"] == 2
    assert s["fps"] == 10 and s["save_combined_views"] is False and s["num_steps"] == 20
    for v in views:
        key = CAMERA_KEYS[v]
        assert s[key] == {"control_path": str(clip.video("depth", v))}
    assert "prompt" not in s
    # the rank loop received the exact sample (it dumps it next to its outputs before we clean the work dir)
    assert not (spool / "work" / "j_av").exists()


def test_rgb_conditioning_block_and_limits(worker, tmp_path):
    sock, _ = worker
    clip = av7_clip(tmp_path / "clips", seconds=1)  # 87 frames -> 29 @10 -> single chunk
    views = clip.manifest.camera_names[:2]

    async def go():
        await wait_ready(sock)
        ev = [m async for m in protocol.call(sock, "run", job=job(tmp_path, clip, views, rgb=True, job_id="j_rgb",
                                                                    extra={"num_conditional_frames": 2}))]
        assert ev[-1]["event"] == "done", ev[-1]
        s = ev[-1]["manifest"]["sample"]
        assert s["num_conditional_frames"] == 2 and "enable_autoregressive" not in s
        assert s[CAMERA_KEYS[views[0]]]["input_path"] == str(clip.video("rgb", views[0]))
        ev = [m async for m in protocol.call(sock, "run", job=job(tmp_path, clip, views, prompt="BLOCKME", job_id="j_b"))]
        assert ev[-1]["event"] == "failed" and "guardrails" in ev[-1]["error"]
        bad = job(tmp_path, clip, views, job_id="j_c")
        bad["inputs"]["controls"]["hdmap_bbox"] = {"scene_dir": "/scene", "weight": None}
        ev = [m async for m in protocol.call(sock, "run", job=bad)]
        assert ev[-1]["event"] == "failed" and "render" in ev[-1]["error"]
        assert (await protocol.call_result(sock, "status"))["state"] == "ready"

    run(go())


def test_build_sample_limits(tmp_path):
    p = build_parser()
    add_common_args(p)
    w = Transfer25AVWorker(p.parse_args(["--socket", "x", "--engine", "fake", "--nproc", "2"]))
    clip = av7_clip(tmp_path / "clips", seconds=1)
    views = clip.manifest.camera_names[:3]
    paths = {v: "/c.mp4" for v in views}
    with pytest.raises(ValueError, match="GPUs"):
        w.build_sample("j", {"prompt": "p", "controls": {"hdmap_bbox": {}}}, clip.manifest.model_dump(),
                       {"rgb": {}, "controls": {"hdmap_bbox": {"paths": paths}}}, views)
    man = clip.manifest.model_dump()
    man["frames"] = 60  # 20 @10 fps < 29
    with pytest.raises(ValueError, match="one chunk"):
        w.build_sample("j", {"prompt": "p", "controls": {"hdmap_bbox": {}}}, man,
                       {"rgb": {}, "controls": {"hdmap_bbox": {"paths": {v: "/c" for v in views[:2]}}}}, views[:2])
    with pytest.raises(ValueError, match="RGB"):
        w.build_sample("j", {"prompt": "p", "controls": {"hdmap_bbox": {}}, "extra": {"num_conditional_frames": 1}},
                       clip.manifest.model_dump(), {"rgb": {}, "controls": {"hdmap_bbox": {"paths": {v: "/c" for v in views[:2]}}}},
                       views[:2])
