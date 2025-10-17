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

# Cosmos Transfer1 Model Checkpoints
COSMOS_TRANSFER1_7B_CHECKPOINT = "nvidia/Cosmos-Transfer1-7B"
COSMOS_TOKENIZER_CHECKPOINT = "nvidia/Cosmos-Tokenize1-CV8x8x8-720p"
COSMOS_GUARDRAIL_CHECKPOINT = "nvidia/Cosmos-Guardrail1"

# 3rd Party Model Checkpoints
GROUNDING_DINO_MODEL_CHECKPOINT = "IDEA-Research/grounding-dino-tiny"
T5_MODEL_CHECKPOINT = "google-t5/t5-11b"
LLAMA_GUARD_3_MODEL_CHECKPOINT = "meta-llama/Llama-Guard-3-8B"

# Model License Information
MODEL_LICENSES = {
    COSMOS_TRANSFER1_7B_CHECKPOINT: {
        "license": "NVIDIA Open Model License",
        "url": "https://www.nvidia.com/en-us/agreements/enterprise-software/nvidia-open-model-license/"
    },
    COSMOS_TOKENIZER_CHECKPOINT: {
        "license": "NVIDIA Open Model License", 
        "url": "https://www.nvidia.com/en-us/agreements/enterprise-software/nvidia-open-model-license/"
    },
    COSMOS_GUARDRAIL_CHECKPOINT: {
        "license": "NVIDIA Open Model License",
        "url": "https://www.nvidia.com/en-us/agreements/enterprise-software/nvidia-open-model-license/"
    },
    GROUNDING_DINO_MODEL_CHECKPOINT: {
        "license": "Apache License 2.0",
        "url": "https://github.com/IDEA-Research/Grounding-DINO-1.5-API/blob/master/LICENSE"
    },
    T5_MODEL_CHECKPOINT: {
        "license": "Apache License 2.0",
        "url": "https://github.com/google-research/text-to-text-transfer-transformer/blob/main/LICENSE"
    },
    LLAMA_GUARD_3_MODEL_CHECKPOINT: {
        "license": "Llama 3 Community License (Meta)",
        "url": "https://github.com/meta-llama/llama-models/blob/main/models/llama3/LICENSE"
    }
}
