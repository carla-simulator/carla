"""Persistent ``torchrun`` rank loop for Transfer 2.5 auto/multiview.

Loads ``MultiviewInference`` once (all ranks, context parallel over the GPUs)
and then serves samples dropped by the socket worker into a spool directory:

    <spool>/requests/<job>.json   {"sample": {...MultiviewInferenceArguments...}, "out_dir": "..."}
    <spool>/results/<job>.json    {"ok": true, "paths": [...]} | {"ok": false, "error": "..."}
    <spool>/READY                 written by rank 0 after the model is loaded
    <spool>/requests/__shutdown__.json  -> all ranks exit

Rank 0 polls the spool and broadcasts each request to the other ranks with
``torch.distributed.broadcast_object_list``; every rank runs ``generate`` (the
pipeline is collective); rank 0 writes the result.  ``--engine fake`` runs the
same loop single-process without torch (tests).
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import shutil
import sys
import time
import traceback
from pathlib import Path
from typing import Any

from .common import KEY_TO_CAMERA, READY_FILE, SHUTDOWN_FILE

log = logging.getLogger("cosmos_worker.transfer25_av.ranks")


class FakeMultiview:
    info = {"engine": "fake-multiview"}

    def __init__(self, delay: float) -> None:
        self.delay = delay

    def generate(self, sample: dict[str, Any], out_dir: Path) -> list[str]:
        out_dir.mkdir(parents=True, exist_ok=True)
        (out_dir / f"{sample['name']}.json").write_text(json.dumps(sample, indent=2))
        time.sleep(self.delay)
        if "BLOCKME" in sample["prompt"]:
            return []
        paths = []
        for key in KEY_TO_CAMERA:
            cfg = sample.get(key)
            if cfg and cfg.get("control_path"):
                p = out_dir / f"{sample['name']}_{key}.mp4"
                shutil.copyfile(cfg["control_path"], p)
                paths.append(str(p))
        grid = out_dir / f"{sample['name']}_grid.mp4"
        shutil.copyfile(paths[0], grid)
        return [str(out_dir / f"{sample['name']}.mp4")]  # mirrors MultiviewInference.generate's return


class RealMultiview:
    def __init__(self, spool: Path, cp_size: int, guardrails: bool, offload_guardrails: bool) -> None:
        from cosmos_oss.init import init_environment
        from cosmos_transfer2.multiview import MultiviewInference
        from cosmos_transfer2.multiview_config import MultiviewInferenceArguments, MultiviewSetupArguments

        init_environment()
        self._Args = MultiviewInferenceArguments
        setup = MultiviewSetupArguments(output_dir=spool / "engine", model="auto/multiview",
                                        context_parallel_size=cp_size, disable_guardrails=not guardrails,
                                        offload_guardrail_models=offload_guardrails, keep_going=True)
        self.inf = MultiviewInference(setup)
        self.info = {"engine": "cosmos-transfer2.5 auto/multiview", "context_parallel_size": cp_size,
                     "checkpoint": str(setup.checkpoint_path), "experiment": str(setup.experiment),
                     "guardrails": guardrails}

    def generate(self, sample: dict[str, Any], out_dir: Path) -> list[str]:
        return self.inf.generate([self._Args.model_validate(sample)], output_dir=out_dir)


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--spool", required=True)
    p.add_argument("--engine", choices=["real", "fake"], default="real")
    p.add_argument("--context-parallel-size", type=int, default=int(os.environ.get("WORLD_SIZE", "1")))
    p.add_argument("--guardrails", action="store_true", default=True)
    p.add_argument("--no-guardrails", dest="guardrails", action="store_false")
    p.add_argument("--offload-guardrails", action="store_true", default=True)
    p.add_argument("--fake-delay", type=float, default=0.3)
    p.add_argument("--poll", type=float, default=0.5)
    args = p.parse_args(argv)
    logging.basicConfig(level="INFO", format="%(asctime)s %(levelname)s rank%(process)d %(name)s: %(message)s",
                        stream=sys.stderr)
    spool = Path(args.spool)
    req_dir, res_dir = spool / "requests", spool / "results"
    dist = None
    if args.engine == "fake":
        engine: Any = FakeMultiview(args.fake_delay)
        rank, world = 0, 1
    else:
        engine = RealMultiview(spool, args.context_parallel_size, args.guardrails, args.offload_guardrails)
        if args.context_parallel_size > 1:
            import torch.distributed as dist  # noqa: PLC0415

            rank, world = dist.get_rank(), dist.get_world_size()
        else:
            rank, world = 0, 1
    if rank == 0:
        req_dir.mkdir(parents=True, exist_ok=True)
        res_dir.mkdir(parents=True, exist_ok=True)
        (spool / READY_FILE).write_text(json.dumps({**engine.info, "world_size": world}))
        log.info("multiview ranks ready (world %d)", world)

    while True:
        payload: dict[str, Any] | None = None
        if rank == 0:
            while payload is None:
                files = sorted(req_dir.glob("*.json"), key=lambda f: f.stat().st_mtime)
                if files:
                    f = files[0]
                    try:
                        payload = json.loads(f.read_text())
                    except json.JSONDecodeError:
                        time.sleep(0.1)  # half-written; retry
                        continue
                    payload["__file"] = f.name
                    f.unlink()
                else:
                    time.sleep(args.poll)
        if dist is not None:
            box = [payload]
            dist.broadcast_object_list(box, src=0)
            payload = box[0]
        assert payload is not None
        if payload["__file"] == SHUTDOWN_FILE:
            log.info("shutdown requested")
            return 0
        result: dict[str, Any]
        try:
            paths = engine.generate(payload["sample"], Path(payload["out_dir"]))
            result = {"ok": bool(paths), "paths": paths,
                      "error": None if paths else "blocked by Cosmos guardrails (prompt or generated video)"}
        except Exception as exc:  # noqa: BLE001
            log.error("generation failed:\n%s", traceback.format_exc())
            result = {"ok": False, "paths": [], "error": f"{type(exc).__name__}: {exc}"}
        if rank == 0:
            tmp = res_dir / (payload["__file"] + ".tmp")
            tmp.write_text(json.dumps(result))
            os.replace(tmp, res_dir / payload["__file"])
        if dist is not None:
            dist.barrier()


if __name__ == "__main__":
    sys.exit(main())
