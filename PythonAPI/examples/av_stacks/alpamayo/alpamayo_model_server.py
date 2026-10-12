#!/usr/bin/env python3
"""Persistent Alpamayo 2 Super inference service for alpamayo_control.py."""

from __future__ import annotations

import argparse
import json
import os
import socket
import time
import traceback
from typing import Any

import numpy as np

from alpamayo_protocol import WireMessage, recv_message, send_message
from alpamayo_schema import request_to_torch_data, validate_request


DEFAULT_MODEL_ID = "nvidia/Alpamayo2-Super"
NUM_LAYER_PAIRS = 64


def paired_device_map(device_count: int) -> dict[str, int]:
    """Place corresponding VLM/expert layers together for KV-cache locality."""
    if device_count < 2:
        raise ValueError("paired sharding requires at least two visible CUDA GPUs")
    device_map: dict[str, int] = {
        "vlm.model.visual": 0,
        "vlm.model.language_model.embed_tokens": 0,
        "vlm.model.language_model.rotary_emb": 0,
        "vlm.model.language_model.norm": device_count - 1,
        "vlm.lm_head": 0,
        "expert.expert.norm": device_count - 1,
        "expert.expert.rotary_emb": 0,
        "expert.action_in_proj": 0,
        "expert.action_out_proj": 0,
        "expert.action_space": 0,
    }
    for layer_index in range(NUM_LAYER_PAIRS):
        device = min(device_count - 1, layer_index * device_count // NUM_LAYER_PAIRS)
        device_map[f"vlm.model.language_model.layers.{layer_index}"] = device
        device_map[f"expert.expert.layers.{layer_index}"] = device
    return device_map


class MockPredictor:
    def __init__(self, speed_mps: float) -> None:
        self.speed_mps = speed_mps

    def predict(self, message: WireMessage) -> tuple[np.ndarray, dict[str, Any]]:
        validate_request(message.metadata, message.arrays)
        timestamps = np.arange(1, 65, dtype=np.float32) * 0.1
        prediction = np.zeros((64, 3), dtype=np.float32)
        prediction[:, 0] = self.speed_mps * timestamps
        return prediction, {
            "cot": "Mock policy: proceed straight.",
            "inference_seconds": 0.0,
            "mock": True,
        }


class AlpamayoPredictor:
    def __init__(self, model_id: str, diffusion_steps: int, seed: int) -> None:
        os.environ.setdefault("PYTORCH_CUDA_ALLOC_CONF", "expandable_segments:True")
        import torch

        from alpamayo2_super.inference_smoke import validate_model_id
        from alpamayo2_super.models.alpamayo2_super import Alpamayo2Super

        if not torch.cuda.is_available() or torch.cuda.device_count() < 2:
            raise RuntimeError("Alpamayo requires at least two visible CUDA GPUs")
        validate_model_id(model_id)
        print(
            f"Loading {model_id} across {torch.cuda.device_count()} visible GPUs...",
            flush=True,
        )
        self.model = Alpamayo2Super.from_pretrained(
            model_id,
            dtype=torch.bfloat16,
            device_map=paired_device_map(torch.cuda.device_count()),
            low_cpu_mem_usage=True,
        )
        self.model.eval()
        self.diffusion_steps = diffusion_steps
        self.seed = seed
        self.request_count = 0
        print("Alpamayo loaded.", json.dumps(self.gpu_memory()), flush=True)

    @staticmethod
    def gpu_memory() -> list[dict[str, float]]:
        import torch

        return [
            {
                "gpu": index,
                "allocated_gib": round(torch.cuda.memory_allocated(index) / 2**30, 2),
                "reserved_gib": round(torch.cuda.memory_reserved(index) / 2**30, 2),
            }
            for index in range(torch.cuda.device_count())
        ]

    def predict(self, message: WireMessage) -> tuple[np.ndarray, dict[str, Any]]:
        import torch

        from alpamayo2_super import helper
        from alpamayo2_super.input_profiles import select_task_input

        source_data = request_to_torch_data(message.metadata, message.arrays)
        data = select_task_input(source_data, "trajectory")
        model_inputs = helper.prepare_model_inputs(data, self.model.config, self.model.tokenizer)
        model_inputs = helper.to_device(model_inputs, "cuda:0")
        torch.cuda.manual_seed_all(self.seed + self.request_count)
        started = time.monotonic()
        with torch.autocast("cuda", dtype=torch.bfloat16):
            pred_xyz, _, _, extra = self.model.sample_trajectories_from_data(
                data=model_inputs,
                top_p=0.98,
                temperature=0.6,
                num_traj_samples=1,
                diffusion_kwargs={"inference_step": self.diffusion_steps},
                return_extra=True,
            )
        elapsed = time.monotonic() - started
        self.request_count += 1
        prediction = np.ascontiguousarray(
            pred_xyz.detach().cpu().numpy()[0, 0, 0], dtype=np.float32
        )
        if prediction.shape != (64, 3) or not np.isfinite(prediction).all():
            raise RuntimeError(f"model returned invalid trajectory shape {prediction.shape}")
        cot_values = extra.get("cot")
        reasoning = str(cot_values[0, 0, 0]) if cot_values is not None else ""
        return prediction, {
            "cot": reasoning,
            "inference_seconds": elapsed,
            "request_index": self.request_count - 1,
            "gpu_memory": self.gpu_memory(),
            "mock": False,
        }


def serve_connection(connection: socket.socket, predictor: Any) -> None:
    while True:
        try:
            message = recv_message(connection)
        except EOFError:
            return
        if message.kind == "ping":
            send_message(connection, "pong", {"status": "ready"})
            continue
        if message.kind != "predict":
            send_message(connection, "error", {"error": f"unsupported message {message.kind!r}"})
            continue
        try:
            prediction, metadata = predictor.predict(message)
            send_message(connection, "prediction", metadata, {"pred_xyz": prediction})
        except Exception as exc:
            traceback.print_exc()
            send_message(connection, "error", {"error": f"{type(exc).__name__}: {exc}"})


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8767)
    parser.add_argument("--model-id", default=os.environ.get("ALPAMAYO2_MODEL_ID", DEFAULT_MODEL_ID))
    parser.add_argument("--diffusion-steps", type=int, default=10)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--mock", action="store_true")
    parser.add_argument("--mock-speed", type=float, default=5.0)
    args = parser.parse_args()

    predictor: Any = (
        MockPredictor(args.mock_speed)
        if args.mock
        else AlpamayoPredictor(args.model_id, args.diffusion_steps, args.seed)
    )
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((args.host, args.port))
        server.listen(1)
        print(f"Alpamayo model server listening on {args.host}:{args.port}", flush=True)
        try:
            while True:
                connection, address = server.accept()
                print(f"CARLA client connected: {address[0]}:{address[1]}", flush=True)
                try:
                    with connection:
                        connection.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                        serve_connection(connection, predictor)
                except (BrokenPipeError, ConnectionResetError, OSError) as exc:
                    print(f"CARLA client connection ended: {exc}", flush=True)
                print("CARLA client disconnected.", flush=True)
        except KeyboardInterrupt:
            print("Stopping Alpamayo model server.", flush=True)


if __name__ == "__main__":
    main()
