"""Rank loop for Transfer 2.5 auto/multiview: ``MultiviewInference`` on every rank (see ``common.ranks``).

NVIDIA ships 7 views on 8 GPUs; the worker starts ``torchrun --nproc_per_node N -m
cosmos_workers.transfer25_av.ranks --context-parallel-size N`` with N = the GPUs given to the worker
(independent of the number of views).
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import time
from pathlib import Path
from typing import Any

from ..common.ranks import RankEngine, is_rank_zero, rank_main
from .common import KEY_TO_CAMERA


class FakeMultiview(RankEngine):
    info = {"engine": "fake-multiview"}

    def __init__(self, delay: float) -> None:
        self.delay = delay

    def generate(self, sample: dict[str, Any], out_dir: Path) -> list[str]:
        time.sleep(self.delay)
        if "BLOCKME" in sample["prompt"]:
            return []
        if is_rank_zero():
            out_dir.mkdir(parents=True, exist_ok=True)
            (out_dir / f"{sample['name']}.json").write_text(json.dumps(
                {**sample, "world_size": os.environ.get("WORLD_SIZE", "1")}, indent=2))
            paths = []
            for key in KEY_TO_CAMERA:
                cfg = sample.get(key)
                if cfg and cfg.get("control_path"):
                    p = out_dir / f"{sample['name']}_{key}.mp4"
                    shutil.copyfile(cfg["control_path"], p)
                    paths.append(str(p))
            shutil.copyfile(paths[0], out_dir / f"{sample['name']}_grid.mp4")
        return [str(out_dir / f"{sample['name']}.mp4")]  # mirrors MultiviewInference.generate's return


class RealMultiview(RankEngine):
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


def build_parser() -> argparse.ArgumentParser:
    return argparse.ArgumentParser(description=__doc__)


def make_engine(args: argparse.Namespace) -> RankEngine:
    if args.engine == "fake":
        return FakeMultiview(args.fake_delay)
    return RealMultiview(Path(args.spool), args.context_parallel_size, args.guardrails, args.offload_guardrails)


def main(argv: list[str] | None = None) -> int:
    return rank_main(argv, build_parser, make_engine)


if __name__ == "__main__":
    raise SystemExit(main())
