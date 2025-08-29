#!/bin/bash

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

# Start script for the gradio app:
# - Verifies that CHECKPOINT_DIR and GRADIO_APP exist
# - Creates directories specified by environment variables
# - Installs gradio
# - Starts the app, teeing output to the log file

# Get the repo root (expected to be called from within the repo)
REPO_ROOT="$(git rev-parse --show-toplevel)"

# Export environment variables if not already set
export CHECKPOINT_DIR=${CHECKPOINT_DIR:-$REPO_ROOT/checkpoints}
export OUTPUT_DIR=${OUTPUT_DIR:-$REPO_ROOT/outputs/gradio/output}
export UPLOADS_DIR=${UPLOADS_DIR:-$REPO_ROOT/outputs/gradio/uploads}
export LOG_FILE=${LOG_FILE:-$REPO_ROOT/logs/$(date +%Y%m%d_%H%M%S).txt}
export GRADIO_APP=${GRADIO_APP:-$REPO_ROOT/server/gradio_app.py}

# Verify that checkpoints directory exists
echo "Checking if checkpoints directory exists: $CHECKPOINT_DIR"
if [ ! -d "$CHECKPOINT_DIR" ]; then
    echo "Error: Checkpoints directory does not exist: $CHECKPOINT_DIR"
    exit 1
fi

# Verify that gradio app exists
echo "Checking if gradio app exists: $GRADIO_APP"
if [ ! -f "$GRADIO_APP" ]; then
    echo "Error: Gradio app does not exist: $GRADIO_APP"
    exit 1
fi

# Create output directories
echo "Creating application directories..."
directories=($CHECKPOINT_DIR $OUTPUT_DIR $UPLOADS_DIR $(dirname $LOG_FILE))
for dir in "${directories[@]}"; do
    if [ -d "$dir" ]; then
        echo "Directory already exists: $dir"
    else
        echo "Creating directory: $dir"
        mkdir -p "$dir"
    fi
done

# Start the app and tee output to the log file
command="cd $REPO_ROOT && PYTHONPATH=. python3 $GRADIO_APP"
echo "Starting the app: $command"
eval $command
