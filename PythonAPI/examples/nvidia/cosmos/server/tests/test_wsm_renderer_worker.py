import asyncio
import subprocess
import sys
import time

import pytest

from carla_cosmos.synthetic import av7_clip
from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol, video


@pytest.fixture
def worker(tmp_path):
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "wsm.sock"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.wsm_renderer", "--socket", str(sock), "--name", "wsm",
                             "--engine", "fake", "--fake-delay", "0.1", "--scratch-dir", str(tmp_path / "scratch")])
    for _ in range(200):
        if sock.exists():
            break
        time.sleep(0.05)
    yield str(sock)
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


def test_render_and_retime(worker, tmp_path):
    # 303 frames at 30 fps = 10.1 s of ego poses: enough for the 10 fps / 101-frame case below.
    # The synthetic scene package is a real ClipGT package, so the renderer's output length
    # follows its egomotion table (see synthetic.write_scene) - a short scene now fails, as it should.
    clip = av7_clip(tmp_path / "clips", frames=303)
    cams = clip.manifest.camera_names[:3]

    async def go():
        await wait_ready(worker)
        assert (await protocol.call_result(worker, "smoke"))["ok"] is True
        # 30 fps clip: renderer output (30 fps, 300 frames) is cut to the clip's frame count
        j = {"job_id": "r1", "scene_dir": str(clip.scene_dir), "cameras": cams, "fps": 30,
             "frames": clip.manifest.frames, "out_dir": str(tmp_path / "out30")}
        ev = [m async for m in protocol.call(worker, "run", job=j)]
        assert ev[-1]["event"] == "done", ev[-1]
        assert sorted(f["view"] for f in ev[-1]["files"]) == sorted(cams)
        for f in ev[-1]["files"]:
            info = video.probe(tmp_path / "out30" / f["name"])
            assert info["frames"] == clip.manifest.frames and abs(info["fps"] - 30) < 1e-3
            assert f["kind"] == "control"
        # 10 fps clip (Cosmos 3 wsm): decimate 30 -> 10 and cut to 101 frames
        j = {"job_id": "r2", "scene_dir": str(clip.scene_dir), "cameras": cams[:1], "fps": 10, "frames": 101,
             "out_dir": str(tmp_path / "out10")}
        ev = [m async for m in protocol.call(worker, "run", job=j)]
        assert ev[-1]["event"] == "done", ev[-1]
        info = video.probe(tmp_path / "out10" / ev[-1]["files"][0]["name"])
        assert info["frames"] == 101 and abs(info["fps"] - 10) < 1e-3
        assert ev[-1]["manifest"]["renderer"] == "fake" and ev[-1]["manifest"]["fps"] == 10
        # unknown camera and non-divisible fps are rejected
        for bad in ({"cameras": ["camera:roof:360"], "fps": 30, "frames": 30},
                    {"cameras": cams[:1], "fps": 16, "frames": 16}):
            ev = [m async for m in protocol.call(worker, "run", job={"job_id": "bad", "scene_dir": str(clip.scene_dir),
                                                                     "out_dir": str(tmp_path / "bad"), **bad})]
            assert ev[-1]["event"] == "failed"
        # too short a scene for the requested frames
        j = {"job_id": "r3", "scene_dir": str(clip.scene_dir), "cameras": cams[:1], "fps": 30, "frames": 5000,
             "out_dir": str(tmp_path / "out_long")}
        ev = [m async for m in protocol.call(worker, "run", job=j)]
        assert ev[-1]["event"] == "failed" and "too short" in ev[-1]["error"]

    run(go())
