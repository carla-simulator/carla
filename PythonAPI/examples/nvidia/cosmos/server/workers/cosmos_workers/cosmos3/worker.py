"""Cosmos 3 worker: drives a ``vllm serve <Cosmos3> --omni`` process over its videos API.

Mapping of a carla-cosmos job to vLLM-Omni's transfer request
(``vllm_omni/diffusion/models/cosmos3/transfer.py``, verified 2026-08-26):

* controls ``depth``/``seg``/``wsm`` -> ``extra_params[<hint>] = {"control_path": <worker-local mp4>}``
  (vLLM-Omni cannot receive control videos by upload; the path must exist on this
  filesystem, which it does: the API server materialises inputs under ``/state``);
* ``edge``/``blur`` with ``derive`` -> ``{}`` (computed from the uploaded RGB), or a
  ``control_path`` when a video was uploaded; ``preset_edge_threshold`` /
  ``preset_blur_strength`` pass through from ``request.extra``;
* control weight -> ``control_weight`` (vLLM-Omni >= 2026-08-20 only; normalised
  across active hints server-side);
* RGB video -> multipart ``input_reference`` (needed for derived hints; also sets
  aspect ratio and fps inheritance);
* ``num_frames = clip frames``, ``fps = clip fps``, ``seed``, ``num_inference_steps``,
  ``guidance_scale``, ``resolution`` bucket (``256|480|720``), and every other
  key of ``request.extra`` verbatim (``control_guidance``,
  ``num_video_frames_per_chunk``, ``control_guidance_interval``, ``flow_shift``,
  ``show_control_condition``, ``guardrails`` ...).

Progress: vLLM-Omni's job ``progress`` only ever reads 0 or 100, so this worker
reports a time-based estimate calibrated on previous jobs (message says so).
Cancellation: ``DELETE /v1/videos/{id}`` (cancels the asyncio task; whether the
engine aborts mid-denoise is not guaranteed upstream).

Guardrails follow ``COSMOS_GUARDRAILS`` (``--no-guardrails`` on the vLLM side when
off); a request may downgrade with ``extra["guardrails"] = false``.  A blocked
prompt surfaces as a failed job with the upstream 400 message.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import shlex
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Any

from ..common import http
from ..common.base import CancelledJob, RunContext, RunResult, Worker
from ..common.hfcache import materialize_snapshot
from ..common.ranks import visible_gpus

log = logging.getLogger("cosmos_worker.cosmos3")

# repos cosmos-guardrail opens through NLTK's hardened (O_NOFOLLOW) loader — see load()
GUARDRAIL_REPOS = ("nvidia/Cosmos-1.0-Guardrail",)

HINTS = ("edge", "blur", "depth", "seg", "wsm")
DERIVABLE = ("edge", "blur")
HINT_OPTION_KEYS = {"edge": ("preset_edge_threshold",), "blur": ("preset_blur_strength",)}
DEFAULT_CHUNK = {"wsm": 101}
SMOKE_TIMEOUT_S = 1800.0


def _canonical(name: str) -> str:
    return name.replace(":", "_")


class Cosmos3Worker(Worker):
    """One vLLM-Omni server per worker; Nano (TP=1) or Super (TP from ``--tp``).

    Latency mode adds ``--cfg-parallel 2`` / ``--ulysses N`` so one query spans every GPU the
    worker was given (``tp x cfg x ulysses == len(CUDA_VISIBLE_DEVICES)``).
    """

    name = "cosmos3"
    backends = ("cosmos3-nano",)

    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.port = args.port or http.free_port()
        self.base = f"http://127.0.0.1:{self.port}"
        self.proc: subprocess.Popen | None = None
        self.storage = Path(args.storage_dir or tempfile.mkdtemp(prefix="cosmos3-out-"))
        self.model_path, self.model_sha, self.offline = _resolve_model(args.model, args.hf_home)
        self.served_name = args.served_model_name or (args.model if not Path(args.model).exists() else "cosmos3")
        self.guardrails = args.guardrails
        self.vllm_version: dict[str, Any] = {}
        self.parallel = {k: v for k, v in (("tp", args.tp), ("cfg", args.cfg_parallel), ("ulysses", args.ulysses))
                         if v > 1}
        self._rate_samples: list[float] = []  # seconds per (frame * step)

    # -- lifecycle ---------------------------------------------------------------------------
    def load(self) -> None:
        self.storage.mkdir(parents=True, exist_ok=True)
        if self.guardrails:
            # cosmos-guardrail reads the blocklist's nltk_data through NLTK's hardened opener
            # (O_NOFOLLOW): the HF-cache symlinks must be real files or vLLM-Omni dies at start-up
            # with "Security Violation [pathsec.open]: refusing to follow a symlink".
            for repo in GUARDRAIL_REPOS:
                materialize_snapshot(self.args.hf_home, repo)
        cmd = self._vllm_command()
        env = dict(os.environ)
        env.setdefault("HF_HOME", self.args.hf_home)
        env["VLLM_OMNI_SERVER_STORAGE__PATH"] = str(self.storage)
        env.setdefault("VLLM_OMNI_VIDEO_SYNC_TIMEOUT", str(int(self.args.request_timeout)))
        if self.offline:
            env["HF_HUB_OFFLINE"] = "1"
            env["TRANSFORMERS_OFFLINE"] = "1"
        log.info("starting vLLM-Omni: %s", " ".join(shlex.quote(c) for c in cmd))
        self.proc = subprocess.Popen(cmd, env=env, stdout=sys.stderr, stderr=subprocess.STDOUT)
        t0 = time.monotonic()
        while True:
            if self.proc.poll() is not None:
                raise RuntimeError(f"vLLM-Omni exited with code {self.proc.returncode} during start-up "
                                   f"(see the worker log above)")
            try:
                status, _, _ = http.request("GET", f"{self.base}/health", timeout=5.0)
                if status == 200:
                    break
            except (http.HttpError, OSError):
                pass
            if time.monotonic() - t0 > self.args.startup_timeout:
                self.proc.terminate()
                raise RuntimeError(f"vLLM-Omni not healthy after {self.args.startup_timeout:.0f}s")
            time.sleep(2.0)
        try:
            self.vllm_version = http.get_json(f"{self.base}/version", timeout=10.0) or {}
        except (http.HttpError, OSError, ValueError):
            self.vllm_version = {}
        log.info("vLLM-Omni healthy after %.0fs (model %s, sha %s, version %s)", time.monotonic() - t0,
                 self.model_path, self.model_sha, self.vllm_version.get("version"))

    def shutdown(self) -> None:
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(30)
            except subprocess.TimeoutExpired:
                self.proc.kill()

    def _vllm_command(self) -> list[str]:
        if self.args.vllm_cmd:
            tmpl = self.args.vllm_cmd.format(port=self.port, model=self.model_path, python=sys.executable,
                                             storage=self.storage, tp=self.args.tp, cfg=self.args.cfg_parallel,
                                             ulysses=self.args.ulysses)
            return shlex.split(tmpl)
        cmd = ["vllm", "serve", self.model_path, "--omni", "--host", "127.0.0.1", "--port", str(self.port),
               "--served-model-name", self.served_name, "--init-timeout", str(int(self.args.startup_timeout))]
        if self.args.tp > 1:
            cmd += ["--tensor-parallel-size", str(self.args.tp)]
        # latency mode: one query over several GPUs — CFG branches on separate ranks, Ulysses splits the
        # sequence (flags per vllm-omni entrypoints/cli/serve.py @ d3c990d; Cosmos3 supports both)
        if self.args.cfg_parallel > 1:
            cmd += ["--cfg-parallel-size", str(self.args.cfg_parallel)]
        if self.args.ulysses > 1:
            cmd += ["--ulysses-degree", str(self.args.ulysses)]
        if not self.guardrails:
            cmd.append("--no-guardrails")
        if self.args.vae_tiling:
            cmd.append("--vae-use-tiling")
        for extra in self.args.vllm_arg:
            cmd += shlex.split(extra)
        return cmd

    # -- smoke -------------------------------------------------------------------------------------
    def smoke(self) -> None:
        if self.args.skip_smoke:
            return
        with tempfile.TemporaryDirectory(dir=self.storage, prefix="smoke-") as tmp:
            ctrl = Path(tmp) / "depth.png"
            _write_gradient_png(ctrl, 256, 256)
            out = Path(tmp) / "out"
            out.mkdir()
            job = {
                "job_id": "smoke",
                "request": {"prompt": "a quiet suburban street at noon, photorealistic", "seed": 1,
                            "num_steps": 2, "resolution": "256", "controls": {"depth": {}},
                            "extra": {"guardrails": bool(self.guardrails)}},
                "manifest": {"frames": 5, "fps": 10, "rig": {"cameras": [{"name": "camera:front:wide:120fov"}]}},
                "views": ["camera:front:wide:120fov"],
                "inputs": {"rgb": {}, "controls": {"depth": {"path": str(ctrl), "weight": None}}},
                "out_dir": str(out),
            }
            ctx = RunContext(job_id="smoke", _report=lambda f, m: None)
            res = self.run(job, ctx, timeout=SMOKE_TIMEOUT_S, calibrate=False)
            produced = out / res.files[0]["name"]
            if not produced.exists() or produced.stat().st_size == 0:
                raise RuntimeError("smoke sample produced no video")

    # -- run -----------------------------------------------------------------------------------------
    def run(self, job: dict[str, Any], ctx: RunContext, timeout: float | None = None,
            calibrate: bool = True) -> RunResult:
        request = job["request"]
        manifest = job["manifest"]
        views = job.get("views") or [manifest["rig"]["cameras"][0]["name"]]
        if len(views) != 1:
            raise ValueError(f"cosmos3 backends generate one view per job, got {len(views)}")
        view = views[0]
        inputs = job.get("inputs", {})
        out_dir = Path(job["out_dir"])
        out_dir.mkdir(parents=True, exist_ok=True)

        fields, files, extra = self._build_request(request, manifest, inputs, view)
        ctx.progress(0.02, "submitting to vLLM-Omni")
        t_submit = time.monotonic()
        status, _, body = http.post_multipart(f"{self.base}/v1/videos", fields, files,
                                              timeout=self.args.request_timeout)
        resp = json.loads(body)
        vid = resp["id"]
        log.info("job %s -> vLLM-Omni %s (hints %s, %s frames @ %s fps, res %s)", ctx.job_id, vid,
                 [h for h in HINTS if h in extra], fields.get("num_frames"), fields.get("fps"),
                 extra.get("resolution"))

        frames = int(fields.get("num_frames", manifest.get("frames", 0)) or 0)
        steps = int(fields.get("num_inference_steps", 50) or 50)
        est = self._estimate(frames, steps)
        deadline = t_submit + (timeout or self.args.request_timeout)
        try:
            while True:
                if ctx.cancelled:
                    self._delete(vid)
                    raise CancelledJob(ctx.job_id)
                info = http.get_json(f"{self.base}/v1/videos/{vid}", timeout=30.0)
                st = info.get("status")
                if st == "completed":
                    break
                if st == "failed":
                    err = info.get("error") or {}
                    msg = err.get("message") or "vLLM-Omni reported failure"
                    if err.get("code") == 400 and "guardrail" in msg.lower():
                        msg = f"blocked by Cosmos 3 guardrails: {msg}"
                    raise RuntimeError(msg)
                elapsed = time.monotonic() - t_submit
                if time.monotonic() > deadline:
                    self._delete(vid)
                    raise TimeoutError(f"vLLM-Omni job {vid} still '{st}' after {elapsed:.0f}s")
                frac = 0.05 + 0.90 * min(1.0, elapsed / est) if est else 0.05
                remaining = f", ~{max(0.0, est - elapsed):.0f}s left (estimate)" if est else ""
                ctx.progress(min(frac, 0.95), f"{st.replace('_', ' ')} on vLLM-Omni, {elapsed:.0f}s{remaining}")
                time.sleep(self.args.poll)
        except http.HttpError as exc:
            if exc.status == 400:
                raise RuntimeError(f"vLLM-Omni rejected the request: {exc}") from exc
            raise
        # completed
        ctx.progress(0.96, "downloading result")
        name = f"{_canonical(view)}.mp4"
        http.download(f"{self.base}/v1/videos/{vid}/content", out_dir / name, timeout=600.0)
        inference_s = info.get("inference_time_s")
        if calibrate and inference_s and frames and steps:
            self._rate_samples.append(float(inference_s) / (frames * steps))
            self._rate_samples = self._rate_samples[-20:]
        if not self.args.keep_vllm_outputs:
            self._delete(vid)
        ctx.progress(1.0, "done")
        return RunResult(
            files=[{"name": name, "view": view, "kind": "video"}],
            manifest={
                "backend_impl": "vllm-omni",
                "model": self.served_name,
                "model_path": self.model_path,
                "model_sha": self.model_sha,
                "vllm_version": self.vllm_version,
                "guardrails": bool(self.guardrails) and extra.get("guardrails", True) is not False,
                "gpus": visible_gpus(),
                "parallel": self.parallel,
                "form": {k: v for k, v in fields.items() if k not in ("prompt", "negative_prompt")},
                "extra_params": extra,
                "input_reference": bool(files),
                "inference_time_s": inference_s,
                "stage_durations": info.get("stage_durations"),
                "peak_memory_mb": info.get("peak_memory_mb"),
                "wall_s": time.monotonic() - t_submit,
            },
        )

    def _build_request(self, request: dict[str, Any], manifest: dict[str, Any], inputs: dict[str, Any],
                       view: str) -> tuple[dict[str, str], list[tuple[str, Path, str | None]], dict[str, Any]]:
        extra: dict[str, Any] = {}
        user_extra = dict(request.get("extra") or {})
        controls = inputs.get("controls", {})
        weights_given = any(c.get("weight") is not None for c in controls.values())
        for hint, spec in controls.items():
            if hint not in HINTS:
                raise ValueError(f"control '{hint}' is not a Cosmos 3 hint {HINTS}")
            entry: dict[str, Any] = {}
            path = spec.get("path") or (spec.get("paths") or {}).get(view) or spec.get("scene_dir")
            if spec.get("scene_dir") and not spec.get("path"):
                raise ValueError(f"control '{hint}' arrived as a scene package; the API server must render it "
                                 f"to a video before dispatch (wsm renderer worker)")
            if path:
                entry["control_path"] = str(path)
            elif spec.get("derive"):
                if hint not in DERIVABLE:
                    raise ValueError(f"control '{hint}' cannot be derived on the fly (only {DERIVABLE})")
                if not inputs.get("rgb", {}).get(view):
                    raise ValueError(f"deriving '{hint}' needs the RGB video for view {view}")
            else:
                raise ValueError(f"control '{hint}' has neither a video nor derive=true")
            if weights_given:
                entry["control_weight"] = float(spec.get("weight") if spec.get("weight") is not None else 1.0)
            for key in HINT_OPTION_KEYS.get(hint, ()):
                if key in user_extra:
                    entry[key] = user_extra.pop(key)
            extra[hint] = entry
        if not extra:
            raise ValueError("no control hint given")

        resolution = request.get("resolution") or self.args.default_resolution
        extra["resolution"] = str(resolution)
        frames = int(manifest["frames"])
        extra.setdefault("max_frames", frames)
        for k, v in user_extra.items():
            extra[k] = v
        if not self.guardrails:
            extra.pop("guardrails", None)  # server started with --no-guardrails; key would be redundant
        fields: dict[str, str] = {
            "model": self.served_name,
            "prompt": request["prompt"],
            "seed": str(int(request.get("seed") or 0)),
            "num_frames": str(frames),
            "fps": str(int(manifest["fps"])),
        }
        if request.get("negative_prompt"):
            fields["negative_prompt"] = request["negative_prompt"]
        if request.get("num_steps"):
            fields["num_inference_steps"] = str(int(request["num_steps"]))
        if request.get("guidance") is not None:
            fields["guidance_scale"] = str(float(request["guidance"]))
        fields["extra_params"] = json.dumps(extra)
        files: list[tuple[str, Path, str | None]] = []
        rgb = inputs.get("rgb", {}).get(view)
        if rgb:
            files.append(("input_reference", Path(rgb), "video/mp4"))
        return fields, files, extra

    def _estimate(self, frames: int, steps: int) -> float | None:
        if not self._rate_samples or not frames or not steps:
            return None
        return statistics.median(self._rate_samples) * frames * steps + 5.0

    def _delete(self, vid: str) -> None:
        try:
            http.delete(f"{self.base}/v1/videos/{vid}", timeout=30.0)
        except (http.HttpError, OSError) as exc:
            log.debug("delete %s: %s", vid, exc)


def _resolve_model(model: str, hf_home: str) -> tuple[str, str | None, bool]:
    """Repo id -> cached snapshot path (offline) when present, else the repo id (online)."""
    p = Path(model)
    if p.exists():
        sha = p.name if p.parent.name == "snapshots" else None
        return str(p), sha, True
    cache = Path(hf_home) / "hub" / ("models--" + model.replace("/", "--"))
    ref = cache / "refs" / "main"
    if ref.exists():
        sha = ref.read_text().strip()
        snap = cache / "snapshots" / sha
        if snap.exists():
            return str(snap), sha, True
    return model, None, False


def _write_gradient_png(path: Path, w: int, h: int) -> None:
    """Grey vertical gradient PNG (a plausible 'depth' control) with zlib + struct only."""
    import struct
    import zlib

    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter none
        v = 255 - int(255 * y / max(1, h - 1))
        raw.extend(bytes([v, v, v]) * w)

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 6)) + chunk(b"IEND", b"")
    path.write_bytes(png)


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--model", default=os.environ.get("COSMOS3_MODEL", "nvidia/Cosmos3-Nano"),
                   help="HF repo id or local snapshot path")
    p.add_argument("--served-model-name", default=None,
                   help="model name used in requests (default: the repo id; required when --model is a path)")
    p.add_argument("--tp", type=int, default=1, help="tensor parallel size (Super: 2..8)")
    p.add_argument("--cfg-parallel", type=int, default=1,
                   help="CFG-parallel size (2: positive/negative branches on separate GPUs; latency mode)")
    p.add_argument("--ulysses", type=int, default=1,
                   help="Ulysses sequence-parallel degree; tp x cfg x ulysses = GPUs given (latency mode)")
    p.add_argument("--port", type=int, default=0, help="vLLM-Omni port (0 = free port)")
    p.add_argument("--hf-home", default=os.environ.get("HF_HOME") or str(Path(os.environ.get("COSMOS_MODELS_DIR", "/models")) / "hf"))
    p.add_argument("--storage-dir", default=None, help="where vLLM-Omni writes outputs (default: temp dir)")
    p.add_argument("--default-resolution", default="720", choices=["256", "480", "704", "720"])
    p.add_argument("--guardrails", dest="guardrails", action="store_true",
                   default=http.env_flag("COSMOS_GUARDRAILS", True))
    p.add_argument("--no-guardrails", dest="guardrails", action="store_false")
    p.add_argument("--vae-tiling", action="store_true", default=http.env_flag("COSMOS3_VAE_TILING", True),
                   help="pass --vae-use-tiling (big VRAM saver for video, +~13%% latency)")
    p.add_argument("--vllm-arg", action="append", default=[], help="extra `vllm serve` args (repeatable)")
    p.add_argument("--vllm-cmd", default=os.environ.get("COSMOS_VLLM_CMD"),
                   help="override the full server command (template: {port} {model} {python} {storage} {tp}); tests")
    p.add_argument("--startup-timeout", type=float, default=3600.0)
    p.add_argument("--request-timeout", type=float, default=7200.0)
    p.add_argument("--poll", type=float, default=2.0)
    p.add_argument("--skip-smoke", action="store_true")
    p.add_argument("--keep-vllm-outputs", action="store_true")
    return p
