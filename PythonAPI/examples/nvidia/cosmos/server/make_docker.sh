# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

#!/bin/bash

# CARLA Cosmos Transfer Server Build and Deploy Script
# This script automates the setup and deployment of the CARLA Cosmos Transfer server

set -e  # Exit on any error

# Parse command line arguments
DOWNLOAD_CHECKPOINTS=false
DEV_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --download-checkpoints)
            DOWNLOAD_CHECKPOINTS=true
            shift
            ;;
        --dev)
            DEV_MODE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--download-checkpoints] [--dev] [--help]"
            echo ""
            echo "Options:"
            echo "  --download-checkpoints    Download Cosmos-Transfer1-7B checkpoints (requires HF token)"
            echo "  --dev                     Build the Docker image in development mode"
            echo "  -h, --help               Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  HF_TOKEN                 HuggingFace token for checkpoint download"
            exit 0
            ;;
        *)
            echo "Unknown option $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if conda is installed
if ! command -v conda &> /dev/null; then
    print_error "conda is not installed. Please install conda first:"
    print_error "See instructions: https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html"
    exit 1
fi

print_status "Starting CARLA Cosmos Transfer server setup..."
if [ "$DOWNLOAD_CHECKPOINTS" = true ]; then
    print_status "Checkpoint download step is ENABLED"
else
    print_status "Checkpoint download step is DISABLED (use --download-checkpoints to enable)"
fi

# Step 1: Setup conda environment
print_status "Setting up conda environment..."
cd "$(dirname "$0")"  # Ensure we're in the server directory

if conda env list | grep -q carla-cosmos-server; then
    print_warning "carla-cosmos-server environment already exists. Removing it..."
    conda env remove -n carla-cosmos-server -y
fi

print_status "Creating carla-cosmos-server conda environment..."
conda env create --file carla-cosmos-server.yaml

print_status "Activating carla-cosmos-server conda environment..."
eval "$(conda shell.bash hook)"
conda activate carla-cosmos-server

# Step 2: Setup Cosmos Transfer1 repository
print_status "Setting up Cosmos Transfer1 repository..."

# Create repos directory if it doesn't exist
if [ ! -d "repos" ]; then
    mkdir repos
    print_status "Created repos directory"
fi

cd repos

# Clone cosmos-transfer1 if it doesn't exist
if [ ! -d "cosmos-transfer1" ]; then
    print_status "Cloning cosmos-transfer1 repository..."
    git clone --recursive https://github.com/nvidia-cosmos/cosmos-transfer1.git
    cd cosmos-transfer1
    git switch -c transfer1-stable 6f6701d1259bd4021457923e752b1924644dd089
    
else
    cd cosmos-transfer1
    print_warning "cosmos-transfer1 directory already exists. Skipping clone..."
fi


# Step 3: Download checkpoints (optional)
if [ "$DOWNLOAD_CHECKPOINTS" = true ]; then
    # Bringing out own checkpoints script
    # to reduce the number of models downloaded.
    cp -f ../../cosmos-transfer1-utils/checkpoints_compressed.py cosmos_transfer1/
    cp -f ../../cosmos-transfer1-utils/download_checkpoints_compressed.py scripts/
    cp -f ../../cosmos-transfer1-utils/deploy_config.py server/
    cp -f ../../cosmos-transfer1-utils/gradio_app.py server/
    cp -f ../../cosmos-transfer1-utils/start_gradio_app.sh server/
    cp -f ../../cosmos-transfer1-utils/transfer_pipeline.py cosmos_transfer1/diffusion/inference/
    cp -f ../../cosmos-transfer1-utils/preprocessors.py cosmos_transfer1/diffusion/inference/

    cp -f ../../supervisord.conf .
    cp -f ../../Dockerfile.server .
    cp -f ../../requirements_server.txt .
    
    print_status "Downloading checkpoints using automated script..."
    python -m pip install huggingface-hub torch loguru numpy
    
    # Check for HuggingFace token
    if [ -z "$HF_TOKEN" ]; then
        # Prompt for HuggingFace token if not found in environment
        print_warning "HuggingFace token required for checkpoint download."
        print_warning "You can get your token from: https://huggingface.co/settings/tokens"
        read -s -p "Enter your HuggingFace token: " HF_TOKEN
        echo  # Add newline after hidden input
    else
        print_status "Using HuggingFace token from environment variable"
    fi
    
    if [ -z "$HF_TOKEN" ]; then
        print_error "HuggingFace token is required for checkpoint download."
        exit 1
    fi
    
    # Run the download script
    print_status "Running checkpoint download script..."
    PYTHONPATH=$(pwd) python scripts/download_checkpoints_compressed.py --hf_token "$HF_TOKEN" --output_dir checkpoints/
    
    print_status "Checkpoints downloaded successfully!"
else
    print_status "Skipping checkpoint download (use --download-checkpoints to include this step)"
fi

if [ "$DEV_MODE" = "false" ]; then
    print_status "Building docker image (this will take 3-5 hours)..."
    print_warning "Go for a long coffee break! This process takes 3-5 hours."
    # Clear existing .dockerignore contents
    > .dockerignore
else
    print_warning "Excluding checkpoints."
    print_status "Building docker image ..."
    echo "./checkpoints" > .dockerignore
fi

# Step 6: Build docker image
docker build -f Dockerfile.server -t cosmos-transfer1-carla .

print_status "Docker image built successfully!"
