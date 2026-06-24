# SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os


def _parse_visible_devices(env_value: str) -> int:
    env_value = (env_value or "").strip()
    if env_value == "":
        return -1
    lower = env_value.lower()
    if lower in ("none", "no", "void", "-1"):
        return 0
    if lower in ("all",):
        return -1
    parts = [p.strip() for p in env_value.split(",") if p.strip() != ""]
    return len(parts) if parts else -1


def _get_visibility_env_value() -> str:
    """Return the raw visibility env string to consider for limiting GPUs.

    Rules:
    - Prefer CUDA_VISIBLE_DEVICES if set and non-empty.
    - If CUDA is not set, use NVIDIA_VISIBLE_DEVICES unless it's the special value 'void',
      which indicates the NVIDIA Container Toolkit placeholder and should be ignored.
    - Return empty string if nothing usable is set so callers treat as 'unset'.
    """
    cuda_val = os.environ.get("CUDA_VISIBLE_DEVICES", "").strip()
    if cuda_val:
        return cuda_val
    nvidia_val = os.environ.get("NVIDIA_VISIBLE_DEVICES", "").strip()
    if nvidia_val.lower() == "void":
        return ""  # ignore placeholder
    return nvidia_val


def detect_num_gpus() -> int:
    """Detect the number of available GPUs.

    Priority:
    1) Explicit NUM_GPU env override if > 0
    2) nvidia-smi query count (respecting CUDA_VISIBLE_DEVICES; ignores NVIDIA_VISIBLE_DEVICES=void)
    3) torch.cuda.device_count()
    4) CUDA/NVIDIA_VISIBLE_DEVICES count (NVIDIA 'void' ignored)
    5) WORLD_SIZE (if running under torchrun)
    6) default=1
    """
    # 1) Explicit override
    try:
        if "NUM_GPU" in os.environ:
            v = int(os.environ["NUM_GPU"])
            if v > 0:
                return v
    except Exception:
        pass

    # 2) nvidia-smi
    try:
        import shutil
        import subprocess

        if shutil.which("nvidia-smi"):
            out = subprocess.check_output(
                ["nvidia-smi", "--query-gpu=name", "--format=csv,noheader"],
                stderr=subprocess.DEVNULL,
            )
            total = len([line for line in out.decode().strip().splitlines() if line.strip()])
            if total > 0:
                vis = _parse_visible_devices(_get_visibility_env_value())
                if vis == 0:
                    return 1  # keep behavior consistent with previous default
                if vis > 0:
                    total = min(total, vis)
                return max(total, 1)
    except Exception:
        pass

    # 3) torch
    try:
        import torch

        count = torch.cuda.device_count()
        if count > 0:
            return count
    except Exception:
        pass

    # 4) visibility envs
    vis = _parse_visible_devices(_get_visibility_env_value())
    if vis > 0:
        return vis

    # 5) WORLD_SIZE
    try:
        ws = int(os.environ.get("WORLD_SIZE", "1"))
        if ws > 0:
            return ws
    except Exception:
        pass

    # 6) default
    return 1


class Config:
    checkpoint_dir = os.getenv("CHECKPOINT_DIR", "checkpoints")
    output_dir = os.getenv("OUTPUT_DIR", "outputs/")
    uploads_dir = os.getenv("UPLOADS_DIR", "uploads/")
    log_file = os.getenv("LOG_FILE", "output.log")
    num_gpus = detect_num_gpus()
    factory_module = os.getenv("FACTORY_MODULE", "cosmos_transfer1.diffusion.inference.transfer_pipeline")
    factory_function = os.getenv("FACTORY_FUNCTION", "create_transfer_pipeline")
