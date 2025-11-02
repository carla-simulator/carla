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

import json
import os

import torch

from cosmos_transfer1.auxiliary.depth_anything.model.depth_anything import DepthAnythingModel
from cosmos_transfer1.auxiliary.edge_control.edge_control import EdgeControlModel
from cosmos_transfer1.auxiliary.human_keypoint.human_keypoint import HumanKeypointModel
from cosmos_transfer1.auxiliary.sam2.sam2_model import VideoSegmentationModel
from cosmos_transfer1.auxiliary.vis_control.vis_control import VisControlModel
from cosmos_transfer1.diffusion.inference.inference_utils import valid_hint_keys
from cosmos_transfer1.utils import log
from cosmos_transfer1.utils.video_utils import is_valid_video, video_to_tensor


class Preprocessors:
    """Preprocessor class to handle input control generation for various modalities.
    Note that this class will run on each rank, so each file name must be unique per rank to avoid potential file corruption.
    """

    def __init__(self):
        self.depth_model = None
        self.seg_model = None
        self.keypoint_model = None
        self.vis_model = None
        self.edge_model = None

    def __call__(
        self,
        input_video,
        input_prompt,
        control_inputs,
        output_folder,
        regional_prompts=None,
        blur_strength="medium",
        canny_threshold="medium",
    ):
        for hint_key in control_inputs:
            if hint_key in valid_hint_keys:
                if hint_key in ["depth", "seg", "keypoint", "vis", "edge"]:
                    self.gen_input_control(
                        input_video,
                        input_prompt,
                        hint_key,
                        control_inputs[hint_key],
                        output_folder,
                        blur_strength,
                        canny_threshold,
                    )

                # for all hints we need to create weight tensor if not present
                control_input = control_inputs[hint_key]
                # For each control input modality, compute a spatiotemporal weight tensor as long as
                # the user provides "control_weight_prompt". The object specified in the
                # control_weight_prompt will be treated as foreground and have control_weight for these locations.
                # Everything else will be treated as background and have control weight 0 at those locations.
                if control_input.get("control_weight_prompt", None) is not None and hint_key in ["seg"]:
                    prompt = control_input["control_weight_prompt"]
                    log.info(f"{hint_key}: generating control weight tensor with SAM using {prompt=}")
                    out_tensor = os.path.join(
                        output_folder, f"{hint_key}_control_weight_{int(os.environ.get('LOCAL_RANK', 0))}.pt"
                    )
                    out_video = os.path.join(
                        output_folder, f"{hint_key}_control_weight_{int(os.environ.get('LOCAL_RANK', 0))}.mp4"
                    )
                    weight_scaler = (
                        control_input["control_weight"] if isinstance(control_input["control_weight"], float) else 1.0
                    )
                    self.segmentation(
                        in_video=input_video,
                        out_tensor=out_tensor,
                        out_video=out_video,
                        prompt=prompt,
                        weight_scaler=weight_scaler,
                        binarize_video=True,
                    )
                    control_input["control_weight"] = out_tensor
        if regional_prompts and len(regional_prompts):
            log.info(f"processing regional prompts: {regional_prompts}")
            for i, regional_prompt in enumerate(regional_prompts):
                log.info(f"generating regional context for {regional_prompt}")
                out_tensor = os.path.join(
                    output_folder, f"regional_context_r{int(os.environ.get('LOCAL_RANK', 0))}_{i}.pt"
                )
                if "mask_prompt" in regional_prompt:
                    prompt = regional_prompt["mask_prompt"]
                    out_video = os.path.join(
                        output_folder, f"regional_context_r{int(os.environ.get('LOCAL_RANK', 0))}_{i}.mp4"
                    )
                    self.segmentation(
                        in_video=input_video,
                        out_tensor=out_tensor,
                        out_video=out_video,
                        prompt=prompt,
                        weight_scaler=1.0,
                        legacy_mask=True,
                    )
                    if os.path.exists(out_tensor):
                        regional_prompt["region_definitions_path"] = out_tensor
                elif "region_definitions_path" in regional_prompt and isinstance(
                    regional_prompt["region_definitions_path"], str
                ):
                    if is_valid_video(regional_prompt["region_definitions_path"]):
                        log.info(f"converting video to tensor: {regional_prompt['region_definitions_path']}")
                        video_to_tensor(regional_prompt["region_definitions_path"], out_tensor)
                        regional_prompt["region_definitions_path"] = out_tensor
                    else:
                        raise ValueError(f"Invalid video file: {regional_prompt['region_definitions_path']}")
                else:
                    log.info("do nothing!")

        return control_inputs

    def gen_input_control(
        self,
        in_video,
        in_prompt,
        hint_key,
        control_input,
        output_folder,
        blur_strength="medium",
        canny_threshold="medium",
    ):
        # if input control isn't provided we need to run preprocessor to create input control tensor
        # for depth no special params, for SAM we need to run with prompt
        if control_input.get("input_control", None) is None:
            out_video = os.path.join(
                output_folder, f"{hint_key}_input_control_{int(os.environ.get('LOCAL_RANK', 0))}.mp4"
            )
            control_input["input_control"] = out_video
            if hint_key == "seg":
                prompt = control_input.get("input_control_prompt", in_prompt)
                prompt = " ".join(prompt.split()[:128])
                log.info(
                    f"no input_control provided for {hint_key}. generating input control video with SAM using {prompt=}"
                )
                self.segmentation(
                    in_video=in_video,
                    out_video=out_video,
                    prompt=prompt,
                )
            elif hint_key == "depth":
                log.info(
                    f"no input_control provided for {hint_key}. generating input control video with DepthAnythingModel"
                )
                self.depth(
                    in_video=in_video,
                    out_video=out_video,
                )
            elif hint_key == "vis":
                log.info(
                    f"no input_control provided for {hint_key}. generating input control video with VisControlModel"
                )
                self.vis(
                    in_video=in_video,
                    out_video=out_video,
                    blur_strength=blur_strength,
                )
            elif hint_key == "edge":
                log.info(
                    f"no input_control provided for {hint_key}. generating input control video with EdgeControlModel"
                )
                self.edge(
                    in_video=in_video,
                    out_video=out_video,
                    canny_threshold=canny_threshold,
                )
            else:
                log.info(f"no input_control provided for {hint_key}. generating input control video with Openpose")
                self.keypoint(
                    in_video=in_video,
                    out_video=out_video,
                )

    def vis(self, in_video, out_video, blur_strength="medium"):
        if self.vis_model is None:
            self.vis_model = VisControlModel(blur_strength=blur_strength)

        self.vis_model(in_video, out_video)

    def edge(self, in_video, out_video, canny_threshold="medium"):
        if self.edge_model is None:
            self.edge_model = EdgeControlModel(canny_threshold=canny_threshold)

        self.edge_model(in_video, out_video)

    def depth(self, in_video, out_video):
        if self.depth_model is None:
            self.depth_model = DepthAnythingModel()

        self.depth_model(in_video, out_video)

    def keypoint(self, in_video, out_video):
        if self.keypoint_model is None:
            self.keypoint_model = HumanKeypointModel()

        self.keypoint_model(in_video, out_video)

    def segmentation(
        self,
        in_video,
        prompt,
        out_video=None,
        out_tensor=None,
        weight_scaler=None,
        binarize_video=False,
        legacy_mask=False,
    ):
        if self.seg_model is None:
            self.seg_model = VideoSegmentationModel()
        self.seg_model(
            input_video=in_video,
            output_video=out_video,
            output_tensor=out_tensor,
            prompt=prompt,
            weight_scaler=weight_scaler,
            binarize_video=binarize_video,
            legacy_mask=legacy_mask,
        )


if __name__ == "__main__":
    control_inputs = dict(
        {
            "depth": {
                # "input_control": "depth_control_input.mp4",  # if empty we need to run depth
                # "control_weight" : "0.1", # if empty we need to run SAM
                "control_weight_prompt": "a boy",  # SAM weights prompt
            },
            "seg": {
                # "input_control": "seg_control_input.mp4",  # if empty we need to run SAM
                "input_control_prompt": "A boy",
                "control_weight_prompt": "A boy",  # if present we need to generate weight tensor
            },
        },
    )

    preprocessor = Preprocessors()
    input_video = "cosmos_transfer1/models/sam2/assets/input_video.mp4"

    preprocessor(input_video, control_inputs)
    print(json.dumps(control_inputs, indent=4))
