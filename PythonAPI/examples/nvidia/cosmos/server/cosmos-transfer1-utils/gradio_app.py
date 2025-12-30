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
from datetime import datetime

import gradio as gr

from cosmos_transfer1.utils import log
from server import gradio_file_server, gradio_log_file_viewer
from server.deploy_config import Config
from server.model_factory import create_pipeline

pipeline = None
validator = None


def infer_wrapper(
    request_text,
):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_folder = os.path.join(Config.output_dir, f"generation_{timestamp}")
    os.makedirs(output_folder, exist_ok=True)
    try:
        # Parse the request as JSON
        try:
            request_data = json.loads(request_text)
        except json.JSONDecodeError as e:
            return None, f"Error parsing request JSON: {e}\nPlease ensure your request is valid JSON."

        log.info(f"Model parameters: {json.dumps(request_data, indent=4)}")

        args_dict = validator.parse_and_validate(request_data)
        args_dict["output_dir"] = output_folder

        pipeline.infer(args_dict)

    except Exception as e:
        log.error(f"Error during inference: {e}")
        return None, f"Error: {e}"

    # Check if output was generated
    output_path = os.path.join(output_folder, "output.mp4")
    if os.path.exists(output_path):
        # Read the generated prompt
        prompt_path = os.path.join(output_folder, "output.txt")
        final_prompt = args_dict["prompt"]
        if os.path.exists(prompt_path):
            with open(prompt_path, "r", encoding="utf-8") as f:
                final_prompt = f.read().strip()

        return (
            output_path,
            f"Video generated successfully!\nOutput saved to: {output_folder}\nFinal prompt: {final_prompt}",
        )
    else:
        return None, f"Generation failed - no output video was created\nCheck folder: {output_folder}"


def create_gradio_interface():
    with gr.Blocks(title="Cosmos-Transfer1 Video Generation", theme=gr.themes.Soft()) as interface:
        gr.Markdown("# Cosmos-Transfer1: World Generation with Adaptive Multimodal Control")
        gr.Markdown("Upload a video and configure controls to generate a new video with the Cosmos-Transfer1 model.")

        with gr.Row():
            gradio_file_server.file_server_components(Config.uploads_dir, open=False)

        gr.Markdown("---")
        gr.Markdown(f"**Output Directory**: {Config.output_dir}")

        with gr.Row():
            with gr.Column(scale=1):
                # Single request input field
                request_input = gr.Textbox(
                    label="Request (JSON)",
                    value=json.dumps(
                        {
                            "input_video_path": "assets/example1_input_video.mp4",
                            "prompt": "The video captures a stunning, photorealistic scene with remarkable attention to detail, giving it a lifelike appearance that is almost indistinguishable from reality. It appears to be from a high-budget 4K movie, showcasing ultra-high-definition quality with impeccable resolution.",
                            "negative_prompt": "The video captures a game playing, with bad crappy graphics and cartoonish frames. It represents a recording of old outdated games. The lighting looks very fake. The textures are very raw and basic. The geometries are very primitive. The images are very pixelated and of poor CG quality. There are many subtitles in the footage. Overall, the video is unrealistic at all.",
                            "guidance": 7.0,
                            "num_steps": 35,
                            "seed": 1,
                            "sigma_max": 70.0,
                            "blur_strength": "medium",
                            "canny_threshold": "medium",
                            "edge": {"control_weight": 1.0},
                        },
                        indent=2,
                    ),
                    lines=20,
                    interactive=True,
                )

                # Help section
                with gr.Accordion("Request Format Help", open=False):
                    gr.Markdown(
                        """
                    ### Required Fields:
                    At least one of the following controlnet specifications must be provided:
                    - `vis` (object): Vis controlnet (default: {"control_weight": 0.0})
                    - `seg` (object): Segmentation controlnet (default: {"control_weight": 0.0})
                    - `edge` (object): Edge controlnet (default: {"control_weight": 0.0})
                    - `depth` (object): Depth controlnet (default: {"control_weight": 0.0})
                    - `keypoint` (object): Keypoint controlnet (default: {"control_weight": 0.0})
                    - `upscale` (object): Upscale controlnet (default: {"control_weight": 0.0})
                    - `hdmap` (object): HDMap controlnet (default: {"control_weight": 0.0})
                    - `lidar` (object): Lidar controlnet (default: {"control_weight": 0.0})

                    ### Optional Fields:
                    - `input_video_path` (string): Path to the input video file
                    - `prompt` (string): Text prompt describing the desired output
                    - `negative_prompt` (string): What to avoid in the output
                    - `guidance` (float): Guidance scale (1-15, default: 7.0)
                    - `num_steps` (int): Number of inference steps (10-50, default: 35)
                    - `seed` (int): Random seed (default: 1)
                    - `sigma_max` (float): Maximum noise level (0-80, default: 70.0)
                    - `blur_strength` (string): One of ["very_low", "low", "medium", "high", "very_high"] (default: "medium")
                    - `canny_threshold` (string): One of ["very_low", "low", "medium", "high", "very_high"] (default: "medium")
                    ```
                    """
                    )
                with gr.Accordion("Tips", open=False):
                    gr.Markdown(
                        """
                    - **Use the file browser above** to upload your video and copy its path for the `input_video` field
                    - **Describe a single, captivating scene**: Focus on one scene to prevent unnecessary shot changes
                    - **Use detailed prompts**: Rich descriptions lead to better quality outputs
                    - **Experiment with control weights**: Different combinations can yield different artistic effects
                    - **Adjust sigma_max**: Lower values preserve more of the input video structure
                    """
                    )

            with gr.Column(scale=1):
                # Output
                output_video = gr.Video(label="Generated Video", height=400)
                status_text = gr.Textbox(label="Status", lines=5, interactive=False)
                generate_btn = gr.Button("Generate Video", variant="primary", size="lg")

        gradio_log_file_viewer.log_file_viewer(log_file=Config.log_file, num_lines=100, update_interval=1)

        generate_btn.click(
            fn=infer_wrapper,
            inputs=[request_input],
            outputs=[output_video, status_text],
            api_name="generate_video",
        )

    return interface


if __name__ == "__main__":
    # Check if checkpoints exist
    if not os.path.exists(Config.checkpoint_dir):
        print(f"Error: checkpoints directory {Config.checkpoint_dir} not found.")
        exit(1)

    pipeline, validator = create_pipeline(Config)
    interface = create_gradio_interface()

    interface.launch(
        server_name="0.0.0.0",
        server_port=8080,
        share=True,
        debug=True,
        # Configure file upload limits
        max_file_size="500MB",  # Adjust as needed
        allowed_paths=[Config.output_dir, Config.uploads_dir],
    )
