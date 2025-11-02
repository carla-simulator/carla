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

import argparse
import copy
import gc
import json
import os

import torch

from cosmos_transfer1.checkpoints import BASE_7B_CHECKPOINT_AV_SAMPLE_PATH, BASE_7B_CHECKPOINT_PATH
from cosmos_transfer1.diffusion.inference.inference_utils import default_model_names
from cosmos_transfer1.diffusion.inference.preprocessors import Preprocessors
from cosmos_transfer1.diffusion.inference.world_generation_pipeline import DiffusionControl2WorldGenerationPipeline
from cosmos_transfer1.utils import log
from cosmos_transfer1.utils.io import save_video

"""
This module wrapper classes required for the transfer pipeline to work with model server/worker classes.
The pipeline wrapper maintains loaded models across multiple inferences for better performance,
unlike the demo function which discards the pipeline after each inference.

Key Components:
    - TransferValidator: Validates and processes inference parameters
    - WorkerPipeline: Base interface for model server/worker
    - TransferPipeline: pipeline wrapper implementation for video transfer
"""

# todo "keypoint" is causing dependency issue
hint_keys = {"vis", "seg", "edge", "depth"}
hint_keys_av = {"hdmap", "lidar"}
default_prompt = "The video captures a stunning, photorealistic scene with remarkable attention to detail, giving it a lifelike appearance that is almost indistinguishable from reality. It appears to be from a high-budget 4K movie, showcasing ultra-high-definition quality with impeccable resolution."
default_negative_prompt = "The video captures a game playing, with bad crappy graphics and cartoonish frames. It represents a recording of old outdated games. The lighting looks very fake. The textures are very raw and basic. The geometries are very primitive. The images are very pixelated and of poor CG quality. There are many subtitles in the footage. Overall, the video is unrealistic at all."


class TransferValidator:
    """Validates and processes inference paramters.

    This class handles inference parameter validation and validation of controlnet specifications.
    This class allows to use and test the validation independently from the pipeline.

    Args:
        hint_keys (set): Valid hint keys for controlnet specifications

    Attributes:
        valid_keys (set): Set of valid controlnet hint keys

    """

    def __init__(self, hint_keys=hint_keys):
        self.valid_keys = hint_keys

    def extract_params(self, controlnet_specs):
        args_dict = {}
        controlnet_specs_clean = {}

        for key, val in controlnet_specs.items():
            if key in self.valid_keys:
                controlnet_specs_clean[key] = val
            else:
                if type(val) == dict:
                    raise ValueError(f"Invalid hint_key: {key}. Must be one of {self.valid_keys}")
                else:
                    args_dict[key] = val
        return args_dict, controlnet_specs_clean

    def validate_control_spec(self, controlnet_specs_clean):
        for key in controlnet_specs_clean:
            if key not in self.valid_keys:
                log.warning(f"Invalid hint_key: {key}. Must be one of {self.valid_keys}")

        for key, config in controlnet_specs_clean.items():
            if "control_weight" not in config:
                log.warning(f"No control weight specified for {key}. Setting to 0.5.")
                config["control_weight"] = "0.5"
            else:
                # Check if control weight is a path or a scalar
                weight = config["control_weight"]
                if not isinstance(weight, str) or not weight.endswith(".pt"):
                    try:
                        # Try converting to float
                        scalar_value = float(weight)
                        if scalar_value < 0:
                            raise ValueError(f"Control weight for {key} must be non-negative.")
                    except ValueError:
                        raise ValueError(
                            f"Control weight for {key} must be a valid non-negative float or a path to a .pt file."
                        )

    def validate_params(
        self,
        controlnet_specs,
        input_video_path=None,
        prompt=default_prompt,
        negative_prompt=default_negative_prompt,
        guidance=5,
        num_steps=35,
        seed=1,
        sigma_max=70.0,
        blur_strength="medium",
        canny_threshold="medium",
        output_dir: str = "outputs/",
        num_input_frames: int = 1,
    ):
        """Validate and process transfer generation parameters.

        Performs comprehensive validation of all parameters including interdependency
        checks between controlnet specifications and input requirements.

        Naming of the parameters is inline with original CLI and thus the parameters allowed in controlnet_specs.
        This way we can pass a parameter dictionary directly from the json.

        Returns:
            dict: Validated parameter dictionary ready for pipeline use

        Raises:
            ValueError: If parameters are invalid, incompatible, or missing required inputs
        """
        args = argparse.Namespace()

        if sigma_max < 80 and not input_video_path:
            raise ValueError("Must have 'input_video' specified if sigma_max < 80")

        # Video and prompt settings
        args_dict = {}
        if input_video_path:
            args_dict["input_video"] = input_video_path
        if prompt:
            args_dict["prompt"] = prompt
        if negative_prompt:
            args_dict["negative_prompt"] = negative_prompt

        # Generation parameters
        args_dict["guidance"] = guidance
        args_dict["num_steps"] = num_steps
        args_dict["seed"] = seed
        args_dict["sigma_max"] = sigma_max
        args_dict["blur_strength"] = blur_strength
        args_dict["canny_threshold"] = canny_threshold
        args_dict["output_dir"] = output_dir
        args_dict["num_input_frames"] = num_input_frames

        # validate controlnet_specs
        self.validate_control_spec(controlnet_specs)
        args_dict["controlnet_specs"] = controlnet_specs
        log.info(f"Model parameters: {json.dumps(args_dict, indent=4)}")

        # finally validate interdependencies of the controlnet_specs and input_video_path
        if not input_video_path:
            for key in controlnet_specs:
                if key == "vis" or key == "edge":
                    raise ValueError(f"Controlnet '{key}' requires an input video. Please specify 'input_video_path'.")
                else:
                    controlnet = controlnet_specs.get(key)
                    if not controlnet.get("input_control"):
                        raise ValueError(f"Controlnet '{key}' requires an 'input_control' video OR input_video_path.")

        return args_dict

    def parse_and_validate(self, controlnet_specs):
        args_dict, controlnet_specs_clean = self.extract_params(controlnet_specs)
        full_dict = self.validate_params(
            controlnet_specs=controlnet_specs_clean,
            **args_dict,
        )
        return full_dict

    def prune_and_validate(self, controlnet_specs, **kwargs):
        """
        Prune the controlnet_specs dictionary to only include valid keys and validate the values.
        """
        _, controlnet_specs_clean = self.extract_params(controlnet_specs)
        full_dict = self.validate_params(
            controlnet_specs=controlnet_specs_clean,
            **kwargs,
        )
        return full_dict


class TransferPipeline:
    """Main transfer pipeline implementation for video-to-video generation.

    This pipeline maintains loaded Cosmos models for efficient video transfer inference.
    Models are kept loaded to avoid repeated initialization overhead.

    The pipeline dynamically updates controlnet configurations and reloads models
    only when necessary, optimizing for inference speed over memory usage.

    Args:
        num_gpus (int): Number of GPUs for distributed inference (default: 1)
        checkpoint_dir (str): Directory containing model checkpoints
        checkpoint_name (str): Specific checkpoint file to load
        hint_keys (set): Valid controlnet hint keys for this pipeline
    """

    def __init__(
        self,
        num_gpus: int = 1,
        checkpoint_dir: str = "/mnt/pvc/cosmos-transfer1",
        checkpoint_name=BASE_7B_CHECKPOINT_PATH,
        hint_keys=hint_keys,
    ):
        self.device_rank = 0
        self.process_group = None

        self.preprocessors = Preprocessors()

        if num_gpus > 1:
            from megatron.core import parallel_state

            from cosmos_transfer1.utils import distributed

            distributed.init()
            parallel_state.initialize_model_parallel(context_parallel_size=num_gpus)
            self.process_group = parallel_state.get_context_parallel_group()
            self.device_rank = distributed.get_rank(self.process_group)

        # TODO FIXME: we want to run W/O offloading. therefore we need to give the model at least one control input.
        self.valid_hint_keys = hint_keys
        first_key = next(iter(self.valid_hint_keys))
        self.control_inputs = {
            first_key: {
                "ckpt_path": os.path.join(checkpoint_dir, default_model_names[first_key]),
                "control_weight": 0.5,
            },
        }

        self.checkpoint_dir = checkpoint_dir
        self.video_save_name = "output"

        self.pipeline = DiffusionControl2WorldGenerationPipeline(
            checkpoint_dir=checkpoint_dir,
            checkpoint_name=checkpoint_name,
            control_inputs=self.control_inputs,
            process_group=self.process_group,
            offload_network=False,
            offload_text_encoder_model=False,
            offload_guardrail_models=False,
            offload_prompt_upsampler=False,
            upsample_prompt=False,
            fps=24,
            num_input_frames=1,
            disable_guardrail=True,
        )

    def update_controlnet_spec(
        self,
        checkpoint_dir: str,
        controlnet_specs: dict,
    ):
        """
        Create the controlnet specification defines which control netwworks are active.
        Note that controlnets are active even if the weights are set to 0."""

        config_changed = False

        for hint_key in self.valid_hint_keys:
            if hint_key in controlnet_specs:
                if hint_key not in self.control_inputs:
                    config_changed = True

                # overwrite old parameters
                self.control_inputs[hint_key] = copy.deepcopy(controlnet_specs[hint_key])
                self.control_inputs[hint_key]["ckpt_path"] = os.path.join(checkpoint_dir, default_model_names[hint_key])
            elif hint_key in self.control_inputs:
                # remove old parameters
                del self.control_inputs[hint_key]
                config_changed = True

        log.info(f"{config_changed=}, control_inputs: {json.dumps(self.control_inputs, indent=4)}")

        return config_changed

    def infer(self, args: dict):
        return self.generate(**args)

    def generate(
        self,
        controlnet_specs,
        input_video=None,
        prompt="",
        negative_prompt="",
        guidance=5,
        num_steps=35,
        seed=1,
        sigma_max=70.0,
        blur_strength="medium",
        canny_threshold="medium",
        num_input_frames: int = 1,
        output_dir: str = "outputs/",
    ):
        """Generate video using the transfer pipeline.

        Performs end-to-end video generation including controlnet configuration updates,
        preprocessing, diffusion generation, and output saving. The method automatically
        handles model reloading when controlnet specifications change.

        The method performs these steps:
        1. Updates controlnet specifications and reloads models if needed
        2. Runs preprocessing on the input video
        3. Configures pipeline parameters
        4. Executes diffusion generation
        5. Saves output video and prompt files

        Note:
            Only the primary device (rank 0) saves output files in distributed setups.
            Regional prompts and region definitions are reset to empty lists.
        """

        config_changed = self.update_controlnet_spec(
            checkpoint_dir=self.checkpoint_dir,
            controlnet_specs=controlnet_specs,
        )

        if config_changed:
            self.pipeline.reload_model(self.control_inputs)

        # original code is creating deepcopy. are values touched?
        # TODO add control weights as inference parameter
        current_control_inputs = copy.deepcopy(self.control_inputs)
        log.info(f"current_control_inputs: {json.dumps(current_control_inputs, indent=4)}")

        log.info("Running preprocessor")
        self.preprocessors(
            input_video,
            prompt,
            current_control_inputs,
            output_dir,
        )

        # TODO: add support for regional prompts and region definitions
        if hasattr(self.pipeline, "regional_prompts"):
            self.pipeline.regional_prompts = []
        if hasattr(self.pipeline, "region_definitions"):
            self.pipeline.region_definitions = []

        # WAR these inference parameters are for unknown reasons not part of the generate function
        self.pipeline.guidance = guidance
        self.pipeline.num_steps = num_steps
        self.pipeline.seed = seed
        self.pipeline.sigma_max = sigma_max
        self.pipeline.blur_strength = blur_strength
        self.pipeline.canny_threshold = canny_threshold
        self.pipeline.num_input_frames = num_input_frames

        batch_outputs = self.pipeline.generate(
            prompt=[prompt],
            video_path=[input_video],
            negative_prompt=negative_prompt,
            control_inputs=[current_control_inputs],
            save_folder=output_dir,
            batch_size=1,
        )
        if batch_outputs is None:
            log.critical("Guardrail blocked generation for entire batch.")
        elif self.device_rank == 0:
            videos, final_prompts = batch_outputs
            for i, (video, prompt) in enumerate(zip(videos, final_prompts)):
                video_save_path = os.path.join(output_dir, f"{self.video_save_name}.mp4")
                prompt_save_path = os.path.join(output_dir, f"{self.video_save_name}.txt")
                os.makedirs(os.path.dirname(video_save_path), exist_ok=True)

                save_video(
                    video=video,
                    fps=self.pipeline.fps,
                    H=video.shape[1],
                    W=video.shape[2],
                    video_save_quality=5,
                    video_save_path=video_save_path,
                )

                # Save prompt to text file alongside video
                with open(prompt_save_path, "wb") as f:
                    f.write(prompt.encode("utf-8"))

                log.info(f"Saved video to {video_save_path}")
                log.info(f"Saved prompt to {prompt_save_path}")

    def cleanup(self, cfg):
        """Clean up resources"""
        if cfg.num_gpus > 1:
            import torch.distributed as dist
            from megatron.core import parallel_state

            parallel_state.destroy_model_parallel()
            dist.destroy_process_group()


def create_transfer_pipeline(cfg, create_model=True):
    """Factory function to create transfer pipeline and validator.

    Args:
        cfg: Configuration object with model settings including checkpoint_dir
        create_model (bool): Whether to actually create the model pipeline (default: True)

    Returns:
        tuple: (pipeline, validator) - TransferPipeline instance and TransferValidator
    """
    log.info(f"Initializing model using factory function {cfg.factory_module}.{cfg.factory_function}")

    pipeline = None
    if create_model:
        pipeline = TransferPipeline(
            num_gpus=int(os.environ.get("WORLD_SIZE", 1)),
            checkpoint_dir=cfg.checkpoint_dir,
        )
        gc.collect()
        torch.cuda.empty_cache()

    validator = TransferValidator(hint_keys=hint_keys)
    return pipeline, validator


def create_transfer_pipeline_AV(cfg, create_model=True):
    """Factory function to create AV-specific transfer pipeline and validator.

    Creates a pipeline configured for autonomous vehicle data with specialized
    hint keys (hdmap, lidar) and AV sample checkpoint.

    Args:
        cfg: Configuration object with model settings including checkpoint_dir
        create_model (bool): Whether to actually create the model pipeline (default: True)

    Returns:
        tuple: (pipeline, validator) - AV-configured TransferPipeline and TransferValidator
    """
    log.info(f"Initializing model using factory function {cfg.factory_module}.{cfg.factory_function}")

    pipeline = None
    if create_model:
        pipeline = TransferPipeline(
            num_gpus=int(os.environ.get("WORLD_SIZE", 1)),
            checkpoint_dir=cfg.checkpoint_dir,
            checkpoint_name=BASE_7B_CHECKPOINT_AV_SAMPLE_PATH,
            hint_keys=hint_keys_av,
        )
        gc.collect()
        torch.cuda.empty_cache()

    validator = TransferValidator(hint_keys=hint_keys_av)
    return pipeline, validator
