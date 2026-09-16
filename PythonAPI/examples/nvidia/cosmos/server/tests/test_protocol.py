"""Worker protocol against a real mock-worker subprocess (no API involved)."""

import asyncio
import subprocess
import sys
import time

import pytest

from carla_cosmos_server.config import default_run_dir
from cosmos_workers.common import protocol


@pytest.fixture
def worker(tmp_path):
    run = default_run_dir(tmp_path)
    run.mkdir(parents=True, exist_ok=True)
    sock = run / "w.sock"
    proc = subprocess.Popen([sys.executable, "-m", "cosmos_workers.mock", "--socket", str(sock), "--name", "w",
                             "--backends", "cosmos3-nano", "--delay", "0.6", "--steps", "3", "--load-delay", "0.3"])
    for _ in range(100):
        if sock.exists():
            break
        time.sleep(0.05)
    yield str(sock)
    proc.terminate()
    proc.wait(10)


def run(coro):
    return asyncio.new_event_loop().run_until_complete(coro)


def test_hello_status_smoke_and_run(worker, tmp_path):
    async def go():
        h = await protocol.call_result(worker, "hello")
        assert h["name"] == "w" and h["backends"] == ["cosmos3-nano"] and h["state"] in ("loading", "ready")
        while (await protocol.call_result(worker, "hello"))["state"] != "ready":
            await asyncio.sleep(0.05)
        s = await protocol.call_result(worker, "smoke")
        assert s["ok"] is True and s["seconds"] >= 0
        job = {"job_id": "j1", "request": {"prompt": "p", "controls": {}}, "views": ["camera:front:wide:120fov"],
               "manifest": {"rig": {"cameras": [{"name": "camera:front:wide:120fov"}]}},
               "inputs": {"rgb": {}, "controls": {}}, "out_dir": str(tmp_path / "out")}
        events = [m async for m in protocol.call(worker, "run", job=job)]
        assert [e["event"] for e in events[:-1]] == ["progress"] * 3
        assert events[-1]["event"] == "done"
        assert {f["name"] for f in events[-1]["files"]} == {"camera_front_wide_120fov.mp4", "mock_debug.json"}
        assert (tmp_path / "out" / "mock_debug.json").exists()
        st = await protocol.call_result(worker, "status")
        assert st["state"] == "ready" and st["job_id"] is None
        bad = [m async for m in protocol.call(worker, "bogus")]
        assert bad[-1]["event"] == "failed"
    run(go())


def test_cancel_running_job(worker, tmp_path):
    async def go():
        while (await protocol.call_result(worker, "hello"))["state"] != "ready":
            await asyncio.sleep(0.05)
        job = {"job_id": "j2", "request": {"prompt": "p", "controls": {}}, "views": ["v"],
               "manifest": {"rig": {"cameras": [{"name": "v"}]}}, "inputs": {}, "out_dir": str(tmp_path / "out")}

        async def runner():
            return [m async for m in protocol.call(worker, "run", job=job)]

        t = asyncio.create_task(runner())
        await asyncio.sleep(0.25)
        assert (await protocol.call_result(worker, "status"))["job_id"] == "j2"
        c = await protocol.call_result(worker, "cancel", job_id="j2")
        assert c["cancelled"] is True
        events = await t
        assert events[-1]["event"] == "failed" and events[-1].get("cancelled") is True
        assert (await protocol.call_result(worker, "cancel", job_id="j2"))["cancelled"] is False
        # concurrent second run is refused while busy -> here worker is ready again
        assert (await protocol.call_result(worker, "status"))["state"] == "ready"
    run(go())
