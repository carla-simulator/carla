"""Cosmos Transfer 2.5 ``auto/multiview`` worker (7-camera world scenario -> video).

The model runs context-parallel over N GPUs (N >= number of active views; NVIDIA
ships 7 views on 8 GPUs).  This worker starts ``torchrun --nproc_per_node N
-m cosmos_workers.transfer25_av.ranks`` once, waits for the ranks to load the
checkpoint, and hands jobs over through a spool directory (see ``ranks.py``).

Job -> ``MultiviewInferenceArguments``:

* view ``camera:<...>`` -> key (``front_wide``, ``cross_left``, ``cross_right``,
  ``rear_left``, ``rear_right``, ``rear`` = rear tele, ``front_tele``) with
  ``control_path`` = the ``hdmap_bbox`` control for that view (rendered
  server-side from the scene package or uploaded), ``input_path`` = the RGB
  video when supplied;
* ``num_conditional_frames``: 0 without RGB (pure world-scenario), else
  ``extra`` or 1; ``control_weight`` (one value for all views); ``fps`` = model
  fps (10; the pipeline decimates the 30 fps controls itself), ``num_steps``,
  ``guidance`` (int 0..7), ``seed``, ``negative_prompt``;
* frames: ``29 + 28*(k-1)`` at model fps -> ``enable_autoregressive`` with
  ``num_chunks=k`` when k > 1 (``chunk_overlap`` 1);
* ``save_combined_views=false`` -> one mp4 per view plus a 3x3 grid.

Cancellation cannot interrupt a running multiview generation.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Any

from ..common import http, video
from ..common.base import RunContext, RunResult, Worker
from .common import CAMERA_KEYS, CHUNK_FRAMES, CHUNK_OVERLAP, READY_FILE, SHUTDOWN_FILE

log = logging.getLogger("cosmos_worker.transfer25_av")


def _canonical(name: str) -> str:
    return name.replace(":", "_")


class Transfer25AVWorker(Worker):
    name = "transfer25-av"
    backends = ("transfer2.5-av",)

    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        visible = os.environ.get("CUDA_VISIBLE_DEVICES", "")
        self.nproc = args.nproc or (len([g for g in visible.split(",") if g.strip()]) if visible else 1)
        self.spool = Path(args.spool_dir or tempfile.mkdtemp(prefix="t25av-"))
        self.proc: subprocess.Popen | None = None
        self.engine_info: dict[str, Any] = {}
        self._rate_samples: list[float] = []

    # -- lifecycle -------------------------------------------------------------------------------
    def load(self) -> None:
        for d in ("requests", "results"):
            (self.spool / d).mkdir(parents=True, exist_ok=True)
        ready = self.spool / READY_FILE
        if ready.exists():
            ready.unlink()
        common = ["--spool", str(self.spool), "--engine", self.args.engine,
                  "--guardrails" if self.args.guardrails else "--no-guardrails"]
        if self.args.engine == "fake":
            cmd = [sys.executable, "-m", "cosmos_workers.transfer25_av.ranks", *common, "--fake-delay",
                   str(self.args.fake_delay)]
        else:
            cmd = ["torchrun", "--nproc_per_node", str(self.nproc), "--master_port", str(self.args.master_port),
                   "-m", "cosmos_workers.transfer25_av.ranks", *common, "--context-parallel-size", str(self.nproc)]
            if self.args.offload_guardrails:
                cmd.append("--offload-guardrails")
        env = dict(os.environ)
        env.setdefault("HF_HOME", self.args.hf_home)
        log.info("starting multiview ranks: %s", " ".join(cmd))
        self.proc = subprocess.Popen(cmd, env=env, stdout=sys.stderr, stderr=subprocess.STDOUT)
        t0 = time.monotonic()
        while not ready.exists():
            if self.proc.poll() is not None:
                raise RuntimeError(f"multiview ranks exited with code {self.proc.returncode} during start-up")
            if time.monotonic() - t0 > self.args.startup_timeout:
                self.proc.terminate()
                raise RuntimeError(f"multiview ranks not ready after {self.args.startup_timeout:.0f}s")
            time.sleep(1.0)
        self.engine_info = json.loads(ready.read_text())
        log.info("multiview ranks ready after %.0fs: %s", time.monotonic() - t0, self.engine_info)

    def shutdown(self) -> None:
        if self.proc and self.proc.poll() is None:
            try:
                (self.spool / "requests" / SHUTDOWN_FILE).write_text("{}")
                self.proc.wait(30)
            except (OSError, subprocess.TimeoutExpired):
                self.proc.terminate()
                try:
                    self.proc.wait(30)
                except subprocess.TimeoutExpired:
                    self.proc.kill()

    # -- smoke -----------------------------------------------------------------------------------------
    def smoke(self) -> None:
        if self.args.skip_smoke:
            return
        with tempfile.TemporaryDirectory(dir=self.spool, prefix="smoke-") as tmp:
            cams = list(CAMERA_KEYS)[: min(self.nproc, len(CAMERA_KEYS)) if self.args.engine == "real" else 2]
            paths = {}
            for i, cam in enumerate(cams):
                p = Path(tmp) / f"ctrl_{_canonical(cam)}.mp4"
                video.test_pattern(p, frames=87, fps=30, size="320x176", hue=i * 30)
                paths[cam] = str(p)
            job = {
                "job_id": "smoke", "views": cams,
                "request": {"prompt": "a quiet suburban street at noon, photorealistic", "seed": 1, "num_steps": 1,
                            "controls": {"hdmap_bbox": {}}, "extra": {}},
                "manifest": {"frames": 87, "fps": 30, "rig": {"cameras": [{"name": c} for c in cams]}},
                "inputs": {"rgb": {}, "controls": {"hdmap_bbox": {"paths": paths, "weight": None}}},
                "out_dir": str(Path(tmp) / "out"),
            }
            self.run(job, RunContext(job_id="smoke", _report=lambda f, m: None), calibrate=False)

    # -- run ---------------------------------------------------------------------------------------------
    def run(self, job: dict[str, Any], ctx: RunContext, calibrate: bool = True) -> RunResult:
        request, manifest = job["request"], job["manifest"]
        views = list(job.get("views") or [c["name"] for c in manifest["rig"]["cameras"]])
        sample, n_model = self.build_sample(job["job_id"], request, manifest, job.get("inputs", {}), views)
        ctx.check_cancelled()
        out_dir = Path(job["out_dir"])
        out_dir.mkdir(parents=True, exist_ok=True)
        work = self.spool / "work" / job["job_id"]
        work.mkdir(parents=True, exist_ok=True)

        req = self.spool / "requests" / f"{job['job_id']}.json"
        res = self.spool / "results" / f"{job['job_id']}.json"
        tmp = req.with_suffix(".tmp")
        tmp.write_text(json.dumps({"sample": sample, "out_dir": str(work)}))
        os.replace(tmp, req)
        est = self._estimate(len(views), n_model, int(sample["num_steps"]))
        t0 = time.monotonic()
        ctx.progress(0.02, f"generating {len(views)} view(s) x {n_model} frames with Transfer 2.5 AV")
        while not res.exists():
            if self.proc is not None and self.proc.poll() is not None:
                raise RuntimeError(f"multiview ranks died (exit {self.proc.returncode}) during the job")
            elapsed = time.monotonic() - t0
            if elapsed > self.args.request_timeout:
                raise TimeoutError(f"multiview generation still running after {elapsed:.0f}s")
            frac = 0.05 + 0.90 * min(1.0, elapsed / est) if est else 0.05
            rem = f", ~{max(0.0, est - elapsed):.0f}s left (estimate)" if est else ""
            ctx.progress(min(frac, 0.95), f"denoising, {elapsed:.0f}s{rem}"
                         + (" — cancel requested, will stop after this job" if ctx.cancelled else ""))
            time.sleep(self.args.poll)
        wall = time.monotonic() - t0
        result = json.loads(res.read_text())
        res.unlink()
        if not result.get("ok"):
            raise RuntimeError(result.get("error") or "multiview generation failed")
        if calibrate:
            self._rate_samples.append(wall / (len(views) * n_model * int(sample["num_steps"])))
            self._rate_samples = self._rate_samples[-20:]

        files = []
        for view in views:
            key = CAMERA_KEYS[view]
            src = work / f"{job['job_id']}_{key}.mp4"
            if not src.exists():
                raise RuntimeError(f"multiview produced no video for {view} ({src.name}); "
                                   f"have {sorted(p.name for p in work.glob('*.mp4'))}")
            shutil.move(src, out_dir / f"{_canonical(view)}.mp4")
            files.append({"name": f"{_canonical(view)}.mp4", "view": view, "kind": "video"})
        grid = work / f"{job['job_id']}_grid.mp4"
        if grid.exists():
            shutil.move(grid, out_dir / "grid.mp4")
            files.append({"name": "grid.mp4", "view": None, "kind": "video"})
        shutil.rmtree(work, ignore_errors=True)
        ctx.progress(1.0, "done")
        public = {k: v for k, v in sample.items() if k not in ("prompt", "negative_prompt")}
        return RunResult(files=files, manifest={**self.engine_info, "nproc": self.nproc, "sample": public,
                                                "frames_at_model_fps": n_model, "wall_s": wall})

    def build_sample(self, job_id: str, request: dict[str, Any], manifest: dict[str, Any],
                     inputs: dict[str, Any], views: list[str]) -> tuple[dict[str, Any], int]:
        bad = [v for v in views if v not in CAMERA_KEYS]
        if bad:
            raise ValueError(f"views {bad} are not auto/multiview cameras {list(CAMERA_KEYS)}")
        if len(views) > self.nproc:
            raise ValueError(f"{len(views)} active views need >= {len(views)} GPUs (context parallel), "
                             f"this worker has {self.nproc}")
        ctrl = inputs.get("controls", {}).get("hdmap_bbox")
        if not ctrl:
            raise ValueError("transfer2.5-av needs the 'hdmap_bbox' control")
        if ctrl.get("scene_dir") and not (ctrl.get("paths") or ctrl.get("path")):
            raise ValueError("hdmap_bbox arrived as a scene package; the API server must render it first")
        paths = ctrl.get("paths") or ({views[0]: ctrl["path"]} if ctrl.get("path") and len(views) == 1 else {})
        missing = [v for v in views if v not in paths]
        if missing:
            raise ValueError(f"no hdmap_bbox control video for view(s) {missing}")
        extra = dict(request.get("extra") or {})
        rgb = inputs.get("rgb", {})
        model_fps = int(extra.pop("fps", 10))
        src_fps = int(manifest["fps"])
        if src_fps % model_fps:
            raise ValueError(f"clip fps {src_fps} is not a multiple of model fps {model_fps}")
        n_model = int(manifest["frames"]) // (src_fps // model_fps)
        if n_model < CHUNK_FRAMES:
            raise ValueError(f"{n_model} frames at {model_fps} fps < one chunk ({CHUNK_FRAMES})")
        k = 1 + (n_model - CHUNK_FRAMES) // (CHUNK_FRAMES - CHUNK_OVERLAP)
        has_rgb = all(v in rgb for v in views)
        sample: dict[str, Any] = {
            "name": job_id,
            "prompt": request["prompt"],
            "seed": int(request.get("seed") or 0),
            "guidance": max(0, min(7, round(float(request["guidance"])))) if request.get("guidance") is not None else 3,
            "num_steps": int(request.get("num_steps") or self.args.default_steps),
            "fps": model_fps,
            "control_weight": float(ctrl["weight"]) if ctrl.get("weight") is not None else 1.0,
            "num_conditional_frames": int(extra.pop("num_conditional_frames", 1 if has_rgb else 0)),
            "save_combined_views": False,
        }
        if sample["num_conditional_frames"] > 0 and not has_rgb:
            raise ValueError("num_conditional_frames > 0 needs the RGB video for every requested view")
        if request.get("negative_prompt"):
            sample["negative_prompt"] = request["negative_prompt"]
        if k > 1:
            sample.update(enable_autoregressive=True, num_chunks=int(extra.pop("num_chunks", k)),
                          chunk_overlap=int(extra.pop("chunk_overlap", CHUNK_OVERLAP)))
        for v in views:
            cfg: dict[str, Any] = {"control_path": str(paths[v])}
            if v in rgb and sample["num_conditional_frames"] > 0:
                cfg["input_path"] = str(rgb[v])
            sample[CAMERA_KEYS[v]] = cfg
        if extra:
            log.warning("ignoring unknown extra keys for transfer2.5-av: %s", sorted(extra))
        return sample, n_model

    def _estimate(self, views: int, frames: int, steps: int) -> float | None:
        if not self._rate_samples:
            return None
        return statistics.median(self._rate_samples) * views * frames * steps + 15.0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--nproc", type=int, default=0, help="GPUs / ranks (default: count of CUDA_VISIBLE_DEVICES)")
    p.add_argument("--master-port", type=int, default=12341)
    p.add_argument("--spool-dir", default=None)
    p.add_argument("--hf-home", default=os.environ.get("HF_HOME") or str(Path(os.environ.get("COSMOS_MODELS_DIR", "/models")) / "hf"))
    p.add_argument("--default-steps", type=int, default=35)
    p.add_argument("--guardrails", dest="guardrails", action="store_true",
                   default=http.env_flag("COSMOS_GUARDRAILS", True))
    p.add_argument("--no-guardrails", dest="guardrails", action="store_false")
    p.add_argument("--offload-guardrails", action="store_true", default=True)
    p.add_argument("--engine", choices=["real", "fake"], default=os.environ.get("TRANSFER25_AV_ENGINE", "real"))
    p.add_argument("--fake-delay", type=float, default=0.3)
    p.add_argument("--startup-timeout", type=float, default=3600.0)
    p.add_argument("--request-timeout", type=float, default=7200.0)
    p.add_argument("--poll", type=float, default=1.0)
    p.add_argument("--skip-smoke", action="store_true")
    return p
