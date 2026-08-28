"""Mock worker: returns the input RGB (or a control) after a delay, with progress.

Serves any backend id it is told to.  Useful for API development, the test
suite and ``scripts/smoke_test.sh`` on machines without GPUs.  Behaviour knobs:
``--delay`` seconds per job (default 3), ``--steps`` progress events, ``--fail``
to make every job fail (error-path tests).
"""

from __future__ import annotations

import argparse
import json
import shutil
import time
from pathlib import Path
from typing import Any

from ..common.base import RunContext, RunResult, Worker


def _canonical(name: str) -> str:
    return name.replace(":", "_")


class MockWorker(Worker):
    """CPU-only stand-in for a Cosmos backend."""

    name = "mock"
    backends = ("cosmos3-nano", "cosmos3-super", "transfer2.5", "transfer2.5-av")

    def load(self) -> None:
        time.sleep(self.args.load_delay)

    def smoke(self) -> None:
        time.sleep(0.05)

    def run(self, job: dict[str, Any], ctx: RunContext) -> RunResult:
        if self.args.fail:
            raise RuntimeError("mock worker configured with --fail")
        out = Path(job["out_dir"])
        out.mkdir(parents=True, exist_ok=True)
        views = job.get("views") or [job["manifest"]["rig"]["cameras"][0]["name"]]
        inputs = job.get("inputs", {})
        steps = max(1, int(self.args.steps))
        per = float(self.args.delay) / steps
        files: list[dict[str, Any]] = []
        for i in range(steps):
            ctx.check_cancelled()
            time.sleep(per)
            ctx.progress((i + 1) / steps, f"mock step {i + 1}/{steps}")
        for view in views:
            src = self._pick_source(inputs, view)
            name = f"{_canonical(view)}.mp4"
            if src is not None:
                shutil.copyfile(src, out / name)
            else:
                (out / name).write_bytes(b"")
            files.append({"name": name, "view": view, "kind": "video"})
        (out / "mock_debug.json").write_text(json.dumps({
            "prompt": job["request"]["prompt"],
            "controls": sorted(job["request"].get("controls", {})),
            "inputs": inputs,
        }, indent=2))
        files.append({"name": "mock_debug.json", "kind": "json"})
        return RunResult(files=files, manifest={
            "mock": True,
            "delay": self.args.delay,
            "echoed": "rgb" if any(inputs.get("rgb", {}).values()) else "control",
        })

    @staticmethod
    def _pick_source(inputs: dict[str, Any], view: str) -> str | None:
        rgb = inputs.get("rgb", {}).get(view)
        if rgb:
            return rgb
        for ctrl in inputs.get("controls", {}).values():
            path = ctrl.get("path") or (ctrl.get("paths") or {}).get(view)
            if path:
                return path
        return None


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--delay", type=float, default=3.0, help="seconds per job")
    p.add_argument("--steps", type=int, default=6, help="progress events per job")
    p.add_argument("--load-delay", type=float, default=0.0, help="seconds spent in 'loading'")
    p.add_argument("--fail", action="store_true", help="fail every job")
    return p
