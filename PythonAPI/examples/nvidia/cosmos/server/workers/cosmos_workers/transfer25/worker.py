"""Cosmos Transfer 2.5-2B general worker (``Control2WorldInference`` in a torchrun rank group).

Runs inside the ``transfer25`` venv (cosmos-transfer2.5 v1.5.x, torch cu130).
At start-up it launches one rank per GPU given to the worker (``torchrun
--nproc_per_node N -m cosmos_workers.transfer25.ranks --context-parallel-size N``,
see ``common.ranks``; a single GPU runs the same loop as a plain child process).
Every rank loads the control branches named by ``--hints`` (default: all four ->
the multicontrol experiment, ~4 x 5.5 GB + Reason1-7B text encoder + VAE +
SigLIP2, ~28 GiB resident per rank) and the ranks then serve jobs collectively:
context parallelism splits one clip's latent sequence over the N GPUs.

Job -> ``InferenceArguments`` mapping (``cosmos_transfer2/config.py``):

* RGB video (required by the contract) -> ``video_path``; ``max_frames`` = clip frames
* control with an uploaded video -> ``<hint>: {control_path, control_weight}``;
  ``derive`` -> ``<hint>: {control_weight}`` (the pipeline computes edge/vis from
  the RGB, depth via Video-Depth-Anything, seg via GroundingDINO + SAM2 using
  ``extra["seg_control_prompt"]``)
* ``guidance`` -> int 0..7 (rounded, clamped; default 3), ``num_steps`` (default 35),
  ``resolution`` (``480`` | ``720``), ``seed``, ``negative_prompt`` (Transfer's
  default negative prompt when unset)
* ``extra`` pass-through: ``num_video_frames_per_chunk`` (93), ``num_conditional_frames``
  (0|1|2), ``sigma_max``, ``preset_edge_threshold``, ``preset_blur_strength``,
  ``seg_control_prompt``, ``show_control_condition``, ``show_input``,
  ``keep_input_resolution``, ``guided_generation_*``.

Outputs: the generated ``<view>.mp4`` plus the control videos the model actually
consumed (``control_<hint>.mp4``, useful when the server derived them).

Limits: the pipeline offers no progress hook (time-based estimate reported) and
cannot be interrupted mid-generation; a cancel takes effect before the job
starts or is ignored.  A guardrail block surfaces as a failed job.  The result
manifest carries ``gpus`` and ``parallel: {"context": N}``.
"""

from __future__ import annotations

import argparse
import logging
import os
import shutil
import statistics
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from ..common import http
from ..common.base import RunContext, RunResult, Worker
from ..common.ranks import RankSupervisor, visible_gpus
from .ranks import HINTS

log = logging.getLogger("cosmos_worker.transfer25")

RANKS_MODULE = "cosmos_workers.transfer25.ranks"
CONTROL_OPTION_KEYS = {"edge": ("preset_edge_threshold",), "vis": ("preset_blur_strength",),
                       "seg": ("control_prompt",)}
SAMPLE_EXTRA_KEYS = ("num_video_frames_per_chunk", "num_conditional_frames", "sigma_max", "show_control_condition",
                     "show_input", "keep_input_resolution", "guided_generation_mask",
                     "guided_generation_step_threshold", "guided_generation_foreground_labels",
                     "image_context_path", "context_frame_index")


def _canonical(name: str) -> str:
    return name.replace(":", "_")


# ----------------------------------------------------------------------------- worker

class Transfer25Worker(Worker):
    name = "transfer25"
    backends = ("transfer2.5",)

    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.hints = [h.strip() for h in args.hints.split(",") if h.strip()]
        bad = [h for h in self.hints if h not in HINTS]
        if bad or not self.hints:
            raise SystemExit(f"--hints must be a non-empty subset of {HINTS}, got {self.hints}")
        self.gpus = visible_gpus()
        self.nproc = args.context_parallel_size or len(self.gpus) or 1
        self.scratch = Path(args.scratch_dir or tempfile.mkdtemp(prefix="transfer25-"))
        rank_args = ["--hints", ",".join(self.hints), "--engine", args.engine,
                     "--guardrails" if args.guardrails else "--no-guardrails",
                     "--context-parallel-size", str(self.nproc)]
        if args.engine == "fake":
            rank_args += ["--fake-delay", str(args.fake_delay)]
        elif args.offload_guardrails:
            rank_args.append("--offload-guardrails")
        launched = self.nproc if (args.engine == "real" or args.fake_torchrun) else 1
        self.ranks = RankSupervisor(RANKS_MODULE, self.scratch, launched, args.master_port, rank_args,
                                    startup_timeout=args.startup_timeout, request_timeout=args.request_timeout,
                                    poll=args.poll, env={"HF_HOME": os.environ.get("HF_HOME") or args.hf_home})
        self._rate_samples: list[float] = []

    @property
    def engine_info(self) -> dict[str, Any]:
        return self.ranks.info

    def load(self) -> None:
        self.scratch.mkdir(parents=True, exist_ok=True)
        self.ranks.start()

    def shutdown(self) -> None:
        self.ranks.stop()

    # -- smoke -------------------------------------------------------------------------------------
    def smoke(self) -> None:
        if self.args.skip_smoke:
            return
        with tempfile.TemporaryDirectory(dir=self.scratch, prefix="smoke-") as tmp:
            rgb = Path(tmp) / "rgb.mp4"
            _write_test_video(rgb, frames=93, fps=16, size="320x176")
            out = Path(tmp) / "out"
            job = {
                "job_id": "smoke",
                "request": {"prompt": "a quiet suburban street at noon, photorealistic", "seed": 1, "num_steps": 1,
                            "resolution": "480", "controls": {self.hints[0]: {}}, "extra": {}},
                "manifest": {"frames": 93, "fps": 16, "rig": {"cameras": [{"name": "camera:front:wide:120fov"}]}},
                "views": ["camera:front:wide:120fov"],
                "inputs": {"rgb": {"camera:front:wide:120fov": str(rgb)},
                           "controls": {self.hints[0]: {"derive": True, "weight": None}}},
                "out_dir": str(out),
            }
            res = self.run(job, RunContext(job_id="smoke", _report=lambda f, m: None), calibrate=False)
            if not (out / res.files[0]["name"]).stat().st_size:
                raise RuntimeError("smoke sample produced an empty video")

    # -- run -----------------------------------------------------------------------------------------
    def run(self, job: dict[str, Any], ctx: RunContext, calibrate: bool = True) -> RunResult:
        request, manifest = job["request"], job["manifest"]
        views = job.get("views") or [manifest["rig"]["cameras"][0]["name"]]
        if len(views) != 1:
            raise ValueError(f"transfer2.5 generates one view per job, got {len(views)}")
        view = views[0]
        out_dir = Path(job["out_dir"])
        out_dir.mkdir(parents=True, exist_ok=True)
        sample = self.build_sample(job["job_id"], request, manifest, job.get("inputs", {}), view)
        ctx.check_cancelled()

        frames, steps = int(manifest["frames"]), int(sample["num_steps"])
        est = self._estimate(frames, steps)
        work = self.scratch / "jobs" / job["job_id"]
        work.mkdir(parents=True, exist_ok=True)
        ctx.progress(0.02, f"generating with Transfer 2.5 ({', '.join(sorted(h for h in HINTS if sample.get(h)))}"
                           f"; context parallel x{self.nproc})")

        def on_wait(elapsed: float) -> None:
            frac = 0.05 + 0.90 * min(1.0, elapsed / est) if est else 0.05
            rem = f", ~{max(0.0, est - elapsed):.0f}s left (estimate)" if est else ""
            ctx.progress(min(frac, 0.95), f"denoising, {elapsed:.0f}s{rem}"
                         + (" — cancel requested, will stop after this job" if ctx.cancelled else ""))

        result = self.ranks.submit(job["job_id"], {"sample": sample, "out_dir": str(work)}, on_wait)
        wall = float(result["wall_s"])
        if not result.get("ok"):
            raise RuntimeError(result.get("error") or "blocked by Cosmos guardrails (prompt or generated video)")
        if calibrate:
            self._rate_samples.append(wall / (frames * steps))
            self._rate_samples = self._rate_samples[-20:]

        name = f"{_canonical(view)}.mp4"
        shutil.move(result["paths"][0], out_dir / name)
        files = [{"name": name, "view": view, "kind": "video"}]
        for hint in HINTS:
            c = work / f"{job['job_id']}_control_{hint}.mp4"
            if c.exists():
                shutil.move(c, out_dir / f"control_{hint}.mp4")
                files.append({"name": f"control_{hint}.mp4", "view": view, "kind": "control"})
        shutil.rmtree(work, ignore_errors=True)
        ctx.progress(1.0, "done")
        public = {k: v for k, v in sample.items() if k not in ("prompt", "negative_prompt")}
        return RunResult(files=files, manifest={**self.engine_info, "gpus": self.gpus,
                                                "parallel": {"context": self.nproc}, "sample": public, "wall_s": wall})

    def build_sample(self, job_id: str, request: dict[str, Any], manifest: dict[str, Any],
                     inputs: dict[str, Any], view: str) -> dict[str, Any]:
        rgb = inputs.get("rgb", {}).get(view)
        if not rgb:
            raise ValueError(f"transfer2.5 needs the RGB video for view {view}")
        extra = dict(request.get("extra") or {})
        sample: dict[str, Any] = {
            "name": job_id,
            "prompt": request["prompt"],
            "video_path": str(rgb),
            "seed": int(request.get("seed") or 0),
            "guidance": max(0, min(7, round(float(request["guidance"])))) if request.get("guidance") is not None else 3,
            "num_steps": int(request.get("num_steps") or self.args.default_steps),
            "resolution": str(request.get("resolution") or self.args.default_resolution),
            "max_frames": int(manifest["frames"]),
        }
        if request.get("negative_prompt"):
            sample["negative_prompt"] = request["negative_prompt"]
        for key in SAMPLE_EXTRA_KEYS:
            if key in extra:
                sample[key] = extra.pop(key)
        seg_prompt = extra.pop("seg_control_prompt", None)
        controls = inputs.get("controls", {})
        if not controls:
            raise ValueError("no control given")
        for hint, spec in controls.items():
            if hint not in HINTS:
                raise ValueError(f"control '{hint}' is not a Transfer 2.5 hint {HINTS}")
            if hint not in self.hints:
                raise ValueError(f"control '{hint}' is not loaded on this worker (loaded: {self.hints})")
            cfg: dict[str, Any] = {"control_weight": float(spec["weight"]) if spec.get("weight") is not None else 1.0}
            path = spec.get("path") or (spec.get("paths") or {}).get(view)
            if path:
                cfg["control_path"] = str(path)
            elif not spec.get("derive"):
                raise ValueError(f"control '{hint}' has neither a video nor derive=true")
            for key in CONTROL_OPTION_KEYS.get(hint, ()):
                src = "seg_control_prompt" if key == "control_prompt" else key
                if src == "seg_control_prompt" and seg_prompt is not None:
                    cfg[key] = seg_prompt
                elif src in extra:
                    cfg[key] = extra.pop(src)
            sample[hint] = cfg
        if extra:
            log.warning("ignoring unknown extra keys for transfer2.5: %s", sorted(extra))
        return sample

    def _estimate(self, frames: int, steps: int) -> float | None:
        if not self._rate_samples:
            return None
        return statistics.median(self._rate_samples) * frames * steps + 10.0


def _write_test_video(path: Path, frames: int, fps: int, size: str) -> None:
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-f", "lavfi", "-i", f"testsrc=size={size}:rate={fps}",
                    "-frames:v", str(frames), "-c:v", "libx264", "-preset", "ultrafast", "-pix_fmt", "yuv420p",
                    str(path)], check=True)


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--hints", default=os.environ.get("TRANSFER25_HINTS", ",".join(HINTS)),
                   help="control branches to load (comma list; >1 loads the multicontrol model)")
    p.add_argument("--hf-home", default=os.environ.get("HF_HOME") or str(Path(os.environ.get("COSMOS_MODELS_DIR", "/models")) / "hf"))
    p.add_argument("--scratch-dir", default=None)
    p.add_argument("--default-resolution", default="720", choices=["480", "720"])
    p.add_argument("--default-steps", type=int, default=35)
    p.add_argument("--guardrails", dest="guardrails", action="store_true",
                   default=http.env_flag("COSMOS_GUARDRAILS", True))
    p.add_argument("--no-guardrails", dest="guardrails", action="store_false")
    p.add_argument("--offload-guardrails", action="store_true", default=True)
    p.add_argument("--context-parallel-size", type=int, default=0,
                   help="ranks = GPUs the clip is split over (default: count of CUDA_VISIBLE_DEVICES)")
    p.add_argument("--master-port", type=int, default=12342, help="torchrun rendezvous port (unique per worker)")
    p.add_argument("--engine", choices=["real", "fake"], default=os.environ.get("TRANSFER25_ENGINE", "real"))
    p.add_argument("--fake-delay", type=float, default=0.5)
    p.add_argument("--fake-torchrun", action="store_true",
                   help="with --engine fake also launch the ranks through torchrun (needs torch; tests)")
    p.add_argument("--startup-timeout", type=float, default=3600.0)
    p.add_argument("--request-timeout", type=float, default=7200.0)
    p.add_argument("--poll", type=float, default=1.0)
    p.add_argument("--skip-smoke", action="store_true")
    return p
