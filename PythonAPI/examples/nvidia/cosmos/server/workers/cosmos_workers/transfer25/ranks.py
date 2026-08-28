"""Rank loop for Transfer 2.5 general: ``Control2WorldInference`` on every rank (see ``common.ranks``).

``torchrun --nproc_per_node N -m cosmos_workers.transfer25.ranks --context-parallel-size N ...``
with N = the GPUs given to the worker; ``N == 1`` runs the same loop as a plain child process.
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

HINTS = ("edge", "vis", "depth", "seg")


class RealEngine(RankEngine):
    """Wraps ``cosmos_transfer2.inference.Control2WorldInference`` (context parallel when ``cp_size > 1``)."""

    def __init__(self, hints: list[str], scratch: Path, guardrails: bool, offload_guardrails: bool,
                 cp_size: int) -> None:
        from cosmos_oss.init import init_environment
        from cosmos_transfer2.config import InferenceArguments, SetupArguments
        from cosmos_transfer2.inference import Control2WorldInference

        init_environment()
        self._InferenceArguments = InferenceArguments
        setup = SetupArguments(
            output_dir=scratch, model=hints[0] if len(hints) == 1 else "edge",
            disable_guardrails=not guardrails, offload_guardrail_models=offload_guardrails,
            keep_going=True,  # blocked sample -> None instead of an exception; the worker turns it into a failed job
            **({"context_parallel_size": cp_size} if cp_size > 1 else {}),
        )
        self.inf = Control2WorldInference(setup, batch_hint_keys=list(hints))
        self.info = {
            "backend_impl": "cosmos-transfer2.5",
            "experiment": self.inf.experiment,
            "checkpoints": [str(c) for c in self.inf.checkpoint_list],
            "hints_loaded": list(hints),
            "multicontrol": len(hints) > 1,
            "guardrails": guardrails,
            "context_parallel_size": cp_size,
        }

    def generate(self, sample: dict[str, Any], out_dir: Path) -> str | None:
        args = self._InferenceArguments.model_validate(sample)
        paths = self.inf.generate([args], output_dir=out_dir)
        return paths[0] if paths else None


class FakeEngine(RankEngine):
    """Copies the RGB (or first control) as the result; ``BLOCKME`` in the prompt -> blocked.  Tests only."""

    def __init__(self, hints: list[str], delay: float = 0.5) -> None:
        self.delay = delay
        self.info = {"backend_impl": "fake-transfer2.5", "hints_loaded": list(hints), "multicontrol": len(hints) > 1}

    def generate(self, sample: dict[str, Any], out_dir: Path) -> str | None:
        time.sleep(self.delay)
        if "BLOCKME" in sample["prompt"]:
            return None
        out = out_dir / f"{sample['name']}.mp4"
        if is_rank_zero():  # every rank "takes part"; only rank 0 writes (as the real pipeline does)
            out_dir.mkdir(parents=True, exist_ok=True)
            (out_dir / "fake_request.json").write_text(json.dumps({**sample, "rank": os.environ.get("RANK", "0"),
                                                                    "world_size": os.environ.get("WORLD_SIZE", "1")},
                                                                   indent=2, default=str))
            shutil.copyfile(sample["video_path"], out)
            for hint in HINTS:
                cfg = sample.get(hint)
                if cfg is not None:
                    shutil.copyfile(cfg.get("control_path") or sample["video_path"],
                                    out_dir / f"{sample['name']}_control_{hint}.mp4")
        return str(out)


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--hints", default=",".join(HINTS))
    return p


def make_engine(args: argparse.Namespace) -> RankEngine:
    hints = [h.strip() for h in args.hints.split(",") if h.strip()]
    if args.engine == "fake":
        return FakeEngine(hints, delay=args.fake_delay)
    return RealEngine(hints, Path(args.spool) / "engine", args.guardrails, args.offload_guardrails,
                      args.context_parallel_size)


def main(argv: list[str] | None = None) -> int:
    return rank_main(argv, build_parser, make_engine)


if __name__ == "__main__":
    raise SystemExit(main())
