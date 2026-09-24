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

from contextlib import contextmanager
from typing import Any

import numpy as np
import torch
from einops import rearrange
from megatron.core import parallel_state
from torch.distributed.device_mesh import DeviceMesh
from torch.distributed.fsdp import FSDPModule, fully_shard
from tqdm import tqdm

from cosmos_predict2.conditioner import DataType, TextCondition
from cosmos_predict2.configs.base.config_text2image import Text2ImagePipelineConfig
from cosmos_predict2.datasets.utils import IMAGE_RES_SIZE_INFO
from cosmos_predict2.models.text2image_dit import MiniTrainDIT
from cosmos_predict2.models.utils import init_weights_on_device, load_state_dict
from cosmos_predict2.module.denoise_prediction import DenoisePrediction
from cosmos_predict2.module.denoiser_scaling import RectifiedFlowScaling
from cosmos_predict2.pipelines.base import BasePipeline
from cosmos_predict2.schedulers.rectified_flow_scheduler import RectifiedFlowAB2Scheduler
from cosmos_predict2.tokenizers.tokenizer import TokenizerInterface
from cosmos_predict2.utils.dtensor_helper import DTensorFastEmaModelUpdater, broadcast_dtensor_model_states
from imaginaire.auxiliary.text_encoder import CosmosTextEncoder, get_cosmos_text_encoder
from imaginaire.lazy_config import LazyDict, instantiate
from imaginaire.utils import log, misc
from imaginaire.utils.ema import FastEmaModelUpdater

IS_PREPROCESSED_KEY = "is_preprocessed"


def sample_batch_image(resolution: str = "1024", aspect_ratio: str = "16:9", batch_size: int = 1) -> dict:
    w, h = IMAGE_RES_SIZE_INFO[resolution][aspect_ratio]
    data_batch = {
        "dataset_name": "image_data",
        "images": torch.randn(batch_size, 3, h, w).cuda(),
        "t5_text_embeddings": torch.randn(batch_size, 512, 1024).cuda(),
        "fps": torch.randint(16, 32, (batch_size,)).cuda(),
        "padding_mask": torch.zeros(batch_size, 1, h, w).cuda(),
    }
    return data_batch


def get_sample_batch(
    resolution: str = "480",
    aspect_ratio: str = "16:9",
    batch_size: int = 1,
) -> dict:
    data_batch = sample_batch_image(resolution, aspect_ratio, batch_size)

    for k, v in data_batch.items():
        if isinstance(v, torch.Tensor) and torch.is_floating_point(data_batch[k]):
            data_batch[k] = v.cuda().to(dtype=torch.bfloat16)

    return data_batch


class Text2ImagePipeline(BasePipeline):
    def __init__(self, device: str = "cuda", torch_dtype: torch.dtype = torch.bfloat16):
        super().__init__(device=device, torch_dtype=torch_dtype)
        self.text_encoder: CosmosTextEncoder = None
        self.dit: MiniTrainDIT = None
        self.dit_ema: torch.nn.Module = None
        self.tokenizer: TokenizerInterface = None
        self.conditioner = None
        self.text_guardrail_runner = None
        self.model_names = ["text_encoder", "dit", "tokenizer"]  # , "vae", "image_encoder"]
        self.height_division_factor = 16
        self.width_division_factor = 16
        self.use_unified_sequence_parallel = False

    @staticmethod
    def from_config(
        config: LazyDict[Text2ImagePipelineConfig],
        dit_path: str = "",
        use_text_encoder: bool = True,
        device: str = "cuda",
        torch_dtype: torch.dtype = torch.bfloat16,
        load_ema_to_reg: bool = False,
    ) -> Any:
        # Create a pipe
        pipe = Text2ImagePipeline(device=device, torch_dtype=torch_dtype)
        pipe.config = config
        pipe.precision = {
            "float32": torch.float32,
            "float16": torch.float16,
            "bfloat16": torch.bfloat16,
        }[config.precision]
        pipe.tensor_kwargs = {"device": "cuda", "dtype": pipe.precision}
        log.warning(f"precision {pipe.precision}")

        # 1. set data keys and data information
        pipe.sigma_data = config.sigma_data
        pipe.setup_data_key()

        # 2. setup up diffusion processing and scaling~(pre-condition), sampler
        pipe.scheduler = RectifiedFlowAB2Scheduler(
            sigma_min=config.timestamps.t_min,
            sigma_max=config.timestamps.t_max,
            order=config.timestamps.order,
            t_scaling_factor=config.rectified_flow_t_scaling_factor,
        )
        pipe.scaling = RectifiedFlowScaling(
            pipe.sigma_data, config.rectified_flow_t_scaling_factor, config.rectified_flow_loss_weight_uniform
        )

        # 3. Set up tokenizer
        pipe.tokenizer = instantiate(config.tokenizer)
        assert pipe.tokenizer.latent_ch == pipe.config.state_ch, (
            f"latent_ch {pipe.tokenizer.latent_ch} != state_shape {pipe.config.state_ch}"
        )

        # 4. Load text encoder
        if use_text_encoder:
            pipe.text_encoder = get_cosmos_text_encoder(config=config.text_encoder, device=device)
        else:
            pipe.text_encoder = None

        # 5. Initialize conditioner
        pipe.conditioner = instantiate(config.conditioner)
        assert sum(p.numel() for p in pipe.conditioner.parameters() if p.requires_grad) == 0, (
            "conditioner should not have learnable parameters"
        )

        if config.guardrail_config.enabled:
            from cosmos_predict2.auxiliary.guardrail.common import presets as guardrail_presets

            pipe.text_guardrail_runner = guardrail_presets.create_text_guardrail_runner(
                config.guardrail_config.checkpoint_dir, config.guardrail_config.offload_model_to_cpu
            )
        else:
            pipe.text_guardrail_runner = None

        # 6. Set up DiT
        if dit_path:
            log.info(f"Loading DiT from {dit_path}")
        else:
            log.warning("dit_path not provided, initializing DiT with random weights")
        with init_weights_on_device():
            dit_config = config.net
            pipe.dit = instantiate(dit_config).eval()  # inference

        if dit_path:
            state_dict = load_state_dict(dit_path)
            prefix_to_load = "net_ema." if load_ema_to_reg else "net."
            # drop net. prefix
            state_dict_dit_compatible = dict()
            for k, v in state_dict.items():
                if k.startswith(prefix_to_load):
                    state_dict_dit_compatible[k[len(prefix_to_load) :]] = v
                else:
                    state_dict_dit_compatible[k] = v
            pipe.dit.load_state_dict(state_dict_dit_compatible, strict=False, assign=True)
            del state_dict, state_dict_dit_compatible
            log.success(f"Successfully loaded DiT from {dit_path}")

        # 6-2. Handle EMA
        if config.ema.enabled:
            pipe.dit_ema = instantiate(dit_config).eval()
            pipe.dit_ema.requires_grad_(False)

            pipe.dit_ema_worker = FastEmaModelUpdater()  # default when not using FSDP

            s = config.ema.rate
            pipe.ema_exp_coefficient = np.roots([1, 7, 16 - s**-2, 12 - s**-2]).real.max()
            # copying is only necessary when starting the training at iteration 0.
            # Actual state_dict should be loaded after the pipe is created.
            pipe.dit_ema_worker.copy_to(src_model=pipe.dit, tgt_model=pipe.dit_ema)

        if any(t.is_meta for t in pipe.dit.parameters()):
            pipe.dit = pipe.dit.to_empty(device=device)   # no base checkpoint: the harmonizer .pkl supplies the weights
        pipe.dit = pipe.dit.to(device=device, dtype=torch_dtype)
        torch.cuda.empty_cache()

        # 7. training states
        if parallel_state.is_initialized():
            pipe.data_parallel_size = parallel_state.get_data_parallel_world_size()
        else:
            pipe.data_parallel_size = 1

        return pipe

    def setup_data_key(self) -> None:
        self.input_video_key = self.config.input_video_key
        self.input_image_key = self.config.input_image_key

    def apply_fsdp(self, dp_mesh: DeviceMesh) -> None:
        self.dit.fully_shard(mesh=dp_mesh)
        self.dit = fully_shard(self.dit, mesh=dp_mesh, reshard_after_forward=True)
        broadcast_dtensor_model_states(self.dit, dp_mesh)
        if self.dit_ema:
            self.dit_ema.fully_shard(mesh=dp_mesh)
            self.dit_ema = fully_shard(self.dit_ema, mesh=dp_mesh, reshard_after_forward=True)
            broadcast_dtensor_model_states(self.dit_ema, dp_mesh)
            self.dit_ema_worker = DTensorFastEmaModelUpdater()
            # No need to copy weights to EMA when applying FSDP, it is already copied before applying FSDP.

    def apply_cp(self) -> None:
        pass

    def denoising_model(self) -> MiniTrainDIT:
        return self.dit

    def encode_prompt(self, prompts: str | list[str], max_length: int = 512, return_mask: bool = False) -> torch.Tensor:
        return self.text_encoder.encode_prompts(prompts, max_length=max_length, return_mask=return_mask)  # type: ignore

    @torch.no_grad()
    def decode(self, latent: torch.Tensor) -> torch.Tensor:
        return self.tokenizer.decode(latent / self.sigma_data)

    def _augment_image_dim_inplace(self, data_batch: dict, input_key: str = None) -> None:  # noqa: RUF013
        input_key = "images"
        if input_key in data_batch:
            # Check if the data has already been augmented and avoid re-augmenting
            if IS_PREPROCESSED_KEY in data_batch and data_batch[IS_PREPROCESSED_KEY] is True:
                assert data_batch[input_key].shape[2] == 1, (
                    f"Image data is claimed be augmented while its shape is {data_batch[input_key].shape}"
                )
                return
            else:
                data_batch[input_key] = rearrange(data_batch[input_key], "b c h w -> b c 1 h w").contiguous()
                data_batch[IS_PREPROCESSED_KEY] = True

    @torch.no_grad()
    def encode(self, state: torch.Tensor) -> torch.Tensor:
        return self.tokenizer.encode(state) * self.sigma_data

    @staticmethod
    def get_context_parallel_group() -> Any | None:
        if parallel_state.is_initialized():
            return parallel_state.get_context_parallel_group()
        return None

    def broadcast_split_for_model_parallelsim(
        self,
        x0_B_C_T_H_W: torch.Tensor,
        condition: torch.Tensor,
        epsilon_B_C_T_H_W: torch.Tensor,
        sigma_B_T: torch.Tensor,
    ) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor, torch.Tensor]:
        """
        Broadcast and split the input data and condition for model parallelism.
        Currently, we only support context parallelism, but it's disabled for text2image.
        """
        # Always disable context parallelism for text2image
        self.dit.disable_context_parallel()
        return x0_B_C_T_H_W, condition, epsilon_B_C_T_H_W, sigma_B_T

    def get_data_and_condition(
        self, data_batch: dict[str, torch.Tensor]
    ) -> tuple[torch.Tensor, torch.Tensor, TextCondition]:
        self._augment_image_dim_inplace(data_batch)

        # Latent state
        raw_state = data_batch["images"]
        latent_state = self.encode(raw_state).contiguous().float()

        # Condition
        condition = self.conditioner(data_batch)
        condition = condition.edit_data_type(DataType.IMAGE)
        return raw_state, latent_state, condition

    def denoise(
        self, xt_B_C_T_H_W: torch.Tensor, sigma: torch.Tensor, condition: TextCondition, use_cuda_graphs: bool = False
    ) -> DenoisePrediction:
        """
        Performs denoising on the input noise data, noise level, and condition

        Args:
            xt (torch.Tensor): The input noise data.
            sigma (torch.Tensor): The noise level.
            condition (TextCondition): conditional information, generated from self.conditioner
            use_cuda_graphs (bool, optional): Whether to use CUDA Graphs for inference. Defaults to False.

        Returns:
            DenoisePrediction: The denoised prediction, it includes clean data predicton (x0), \
                noise prediction (eps_pred).
        """
        if sigma.ndim == 1:
            sigma_B_T = rearrange(sigma, "b -> b 1")
        elif sigma.ndim == 2:
            sigma_B_T = sigma
        else:
            raise ValueError(f"sigma shape {sigma.shape} is not supported")
        sigma_B_1_T_1_1 = rearrange(sigma_B_T, "b t -> b 1 t 1 1")
        # get precondition for the network
        c_skip_B_1_T_1_1, c_out_B_1_T_1_1, c_in_B_1_T_1_1, c_noise_B_1_T_1_1 = self.scaling(sigma=sigma_B_1_T_1_1)

        # forward pass through the network
        net_output_B_C_T_H_W = self.dit(
            x_B_C_T_H_W=(xt_B_C_T_H_W * c_in_B_1_T_1_1).to(
                **self.tensor_kwargs
            ),  # Eq. 7 of https://arxiv.org/pdf/2206.00364.pdf
            timesteps_B_T=c_noise_B_1_T_1_1.squeeze(dim=[1, 3, 4]).to(
                **self.tensor_kwargs
            ),  # Eq. 7 of https://arxiv.org/pdf/2206.00364.pdf
            **condition.to_dict(),
            use_cuda_graphs=use_cuda_graphs,
        ).float()

        x0_pred_B_C_T_H_W = c_skip_B_1_T_1_1 * xt_B_C_T_H_W + c_out_B_1_T_1_1 * net_output_B_C_T_H_W

        # get noise prediction
        eps_pred_B_C_T_H_W = (xt_B_C_T_H_W - x0_pred_B_C_T_H_W) / sigma_B_1_T_1_1

        return DenoisePrediction(x0_pred_B_C_T_H_W, eps_pred_B_C_T_H_W, None)

    @torch.no_grad()
    def __call__(
        self,
        prompt: str,
        negative_prompt: str = "",
        aspect_ratio: str = "16:9",
        seed: int = 0,
        guidance: float = 4.0,
        num_sampling_step: int = 35,
        use_cuda_graphs: bool = False,
    ) -> torch.Tensor | None:
        # Parameter check
        width, height = IMAGE_RES_SIZE_INFO[self.config.resolution][aspect_ratio]
        height, width = self.check_resize_height_width(height, width)

        # Run text guardrail on the prompt
        if self.text_guardrail_runner is not None:
            from cosmos_predict2.auxiliary.guardrail.common import presets as guardrail_presets

            log.info("Running guardrail check on prompt...")
            if not guardrail_presets.run_text_guardrail(prompt, self.text_guardrail_runner):
                return None
            else:
                log.success("Passed guardrail on prompt")

        # get sample batch
        data_batch = get_sample_batch(resolution=self.config.resolution, aspect_ratio=aspect_ratio, batch_size=1)
        data_batch["t5_text_embeddings"] = self.encode_prompt(prompt).to(dtype=self.torch_dtype)
        data_batch["neg_t5_text_embeddings"] = self.encode_prompt(negative_prompt).to(dtype=self.torch_dtype)

        # preprocess
        self._augment_image_dim_inplace(data_batch)
        input_key = "images"
        n_sample = data_batch[input_key].shape[0]
        _T, _H, _W = data_batch[input_key].shape[-3:]
        state_shape = [
            self.config.state_ch,
            self.tokenizer.get_latent_num_frames(_T),
            _H // self.tokenizer.spatial_compression_factor,
            _W // self.tokenizer.spatial_compression_factor,
        ]
        # Obtains the latent state and condition.
        _, latent_state, _ = self.get_data_and_condition(data_batch)

        if negative_prompt:
            condition, uncondition = self.conditioner.get_condition_with_negative_prompt(data_batch)
        else:
            condition, uncondition = self.conditioner.get_condition_uncondition(data_batch)

        # Ensures we are conditioning on IMAGE data type.
        condition = condition.edit_data_type(DataType.IMAGE)
        uncondition = uncondition.edit_data_type(DataType.IMAGE)

        # Context parallelism is disabled for text2image
        _, condition, _, _ = self.broadcast_split_for_model_parallelsim(latent_state, condition, None, None)
        _, uncondition, _, _ = self.broadcast_split_for_model_parallelsim(latent_state, uncondition, None, None)

        log.info("Starting image generation...")

        x_sigma_max = (
            misc.arch_invariant_rand(
                (n_sample,) + tuple(state_shape),  # noqa: RUF005
                torch.float32,
                self.tensor_kwargs["device"],
                seed,
            )
            * self.scheduler.config.sigma_max
        )

        # ------------------------------------------------------------------ #
        # Sampling loop driven by `RectifiedFlowAB2Scheduler`
        # ------------------------------------------------------------------ #
        scheduler = self.scheduler

        # Construct sigma schedule (L + 1 entries including simga_min) and timesteps
        scheduler.set_timesteps(num_sampling_step, device=x_sigma_max.device)

        # Bring the initial latent into the precision expected by the scheduler
        sample = x_sigma_max.to(dtype=torch.float32)

        x0_prev: torch.Tensor | None = None

        for i, _ in enumerate(tqdm(scheduler.timesteps, desc="Generating image", leave=False)):
            # Current noise level (sigma_t).
            sigma_t = scheduler.sigmas[i].to(sample.device, dtype=torch.float32)

            # `x0_fn` expects `sigma` as a tensor of shape [B] or [B, T]. We
            # pass a 1-D tensor broadcastable to any later shape handling.
            sigma_in = sigma_t.repeat(sample.shape[0])

            # x0 prediction with conditional and unconditional branches
            cond_x0 = self.denoise(sample, sigma_in, condition, use_cuda_graphs=use_cuda_graphs).x0
            uncond_x0 = self.denoise(sample, sigma_in, uncondition, use_cuda_graphs=use_cuda_graphs).x0
            x0_pred = cond_x0 + guidance * (cond_x0 - uncond_x0)

            # Scheduler step (handles float64 internally, returns original dtype)
            sample, x0_prev = scheduler.step(
                x0_pred=x0_pred,
                i=i,
                sample=sample,
                x0_prev=x0_prev,
            )

        sigma_min = scheduler.sigmas[-1].to(sample.device, dtype=torch.float32)
        sigma_in = sigma_min.repeat(sample.shape[0])

        # Final clean pass.
        cond_x0 = self.denoise(sample, sigma_in, condition, use_cuda_graphs=use_cuda_graphs).x0
        uncond_x0 = self.denoise(sample, sigma_in, uncondition, use_cuda_graphs=use_cuda_graphs).x0
        samples = cond_x0 + guidance * (cond_x0 - uncond_x0)

        # decode
        image = self.decode(samples)

        log.success("Image generation completed successfully")
        return image

    @contextmanager
    def ema_scope(self, context: Any = None, is_cpu: bool = False):
        if self.config.ema.enabled:
            # https://github.com/pytorch/pytorch/issues/144289
            for module in self.dit.modules():
                if isinstance(module, FSDPModule):
                    module.reshard()
            self.dit_ema_worker.cache(self.dit.parameters(), is_cpu=is_cpu)
            self.dit_ema_worker.copy_to(src_model=self.dit_ema, tgt_model=self.dit)
            if context is not None:
                log.info(f"{context}: Switched to EMA weights")
        try:
            yield None
        finally:
            if self.config.ema.enabled:
                for module in self.dit.modules():
                    if isinstance(module, FSDPModule):
                        module.reshard()
                self.dit_ema_worker.restore(self.dit.parameters())
                if context is not None:
                    log.info(f"{context}: Restored training weights")
