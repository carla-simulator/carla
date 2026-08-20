#! /usr/bin/env bash

set -euo pipefail

CARLA_VERSION=0.10.0
# Pin the renderer version used by this integration. A floating latest tag can
# change its gRPC surface underneath the checked-in stubs.
NUREC_IMAGE_DEFAULT="nvcr.io/nvidia/nre/nre-ga:26.04.01"
PYTHON_EXECUTABLE=python
# Default test scene from the public HuggingFace dataset; --full-dataset
# downloads every scene instead (hundreds of GB).
DATASET_REPO="nvidia/PhysicalAI-Autonomous-Vehicles-NuRec"
DATASET_SCENE="00040136-e651-4abd-991d-0655ccda9430"
DATASET_RELEASE="26.04_release"
FULL_DATASET=0

CARLA_ROOT=$(realpath "${BASH_SOURCE[0]}")
CARLA_ROOT=$(dirname "$CARLA_ROOT")
CARLA_ROOT=$CARLA_ROOT/../../../..
CARLA_ROOT=$(realpath "$CARLA_ROOT")
PYTHON_API_ROOT=$CARLA_ROOT/PythonAPI
CARLA_NUREC_ROOT=$PYTHON_API_ROOT/examples/nvidia/nurec

options=$( \
    getopt \
    -o "i:s:" \
    --long "python:,scene:,full-dataset" \
    -n 'install_nurec.sh' -- "$@")

eval set -- "$options"

while true; do
    case "$1" in
        -i|--python)
            PYTHON_EXECUTABLE=$2
            shift 2
            ;;
        -s|--scene)
            DATASET_SCENE=$2
            shift 2
            ;;
        --full-dataset)
            FULL_DATASET=1
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            echo Unknown option $1
            ;;
    esac
done

# Default to an isolated environment beside the example. --python remains an
# escape hatch for callers that already created a compatible environment.
if [ "$PYTHON_EXECUTABLE" = python ]; then
    if [ ! -x "$CARLA_NUREC_ROOT/.venv/bin/python" ]; then
        python3 -m venv "$CARLA_NUREC_ROOT/.venv"
    fi
    PYTHON_EXECUTABLE="$CARLA_NUREC_ROOT/.venv/bin/python"
fi

echo "Using python interpreter $PYTHON_EXECUTABLE."

PYTHON_VERSION=$($PYTHON_EXECUTABLE -V)
IFS=. read PYTHON_MAJOR PYTHON_MINOR PYTHON_PATCH <<< "$($PYTHON_EXECUTABLE -c 'import sys; print(".".join(map(str, sys.version_info[:3])))')"

command_exists() { command -v "$1" >/dev/null 2>&1; }

# The user we want to grant docker access to (the one who invoked sudo, if any)
TARGET_USER="${SUDO_USER:-$USER}"


# Function to check if the requested test data already exists
check_hf_dataset() {
    local dataset_path="$CARLA_NUREC_ROOT/PhysicalAI-Autonomous-Vehicles-NuRec"
    if [ "$FULL_DATASET" -eq 1 ]; then
        local check_path="$dataset_path"
    else
        local check_path="$dataset_path/sample_set/$DATASET_RELEASE/$DATASET_SCENE"
    fi
    if [ -d "$check_path" ]; then
        echo "Test data already exists at $(realpath "$check_path"), skipping download."
        echo "Delete it to force a re-download."
        return 0
    fi
    return 1
}

# Function to check if NuRec container exists
check_NuRec_container() {
    local container_name=$1
    if docker image inspect "$container_name" >/dev/null 2>&1; then
        return 0
    fi
    return 1
}

# Function to validate HuggingFace PAT format (basic check)
validate_hf_pat() {
    if [[ ! $1 =~ ^hf_[a-zA-Z0-9]{32,}$ ]]; then
        echo "Error: Invalid HuggingFace PAT format" >&2
        return 1
    fi
    return 0
}

# Function to get and validate HuggingFace PAT
get_hf_pat() {
    # Color codes
    local RED='\033[0;31m'
    local GREEN='\033[0;32m'
    local YELLOW='\033[1;33m'
    local BLUE='\033[0;34m'
    local PURPLE='\033[0;35m'
    local CYAN='\033[0;36m'
    local WHITE='\033[1;37m'
    local NC='\033[0m' # No Color
    
    echo "" >&2
    echo -e "${CYAN}============================================================${NC}" >&2
    echo -e "${WHITE}              🔑 ${YELLOW}HUGGINGFACE AUTHENTICATION REQUIRED${WHITE} 🔑${NC}" >&2
    echo -e "${CYAN}============================================================${NC}" >&2
    echo "" >&2
    echo -e "${WHITE}To download the dataset, you need a ${YELLOW}HuggingFace Personal Access Token${WHITE}.${NC}" >&2
    echo "" >&2
    echo -e "${BLUE}📍 If you don't have a token yet:${NC}" >&2
    echo -e "${WHITE}   1. Visit: ${CYAN}https://huggingface.co/settings/tokens${NC}" >&2
    echo -e "${WHITE}   2. Click ${YELLOW}'New token'${NC}" >&2
    echo -e "${WHITE}   3. Choose ${GREEN}'Read'${WHITE} permissions${NC}" >&2
    echo -e "${WHITE}   4. Copy the generated token${NC}" >&2
    echo "" >&2
    echo -e "${YELLOW}⚠️  Your input will be hidden for security${NC}" >&2
    echo "" >&2
    
    # Get HuggingFace PAT
    echo -ne "${PURPLE}🔐 Enter your HuggingFace Personal Access Token: ${NC}" >&2
    read -s hf_pat
    echo >&2
    echo "" >&2

    # Validate HuggingFace PAT
    if ! validate_hf_pat "$hf_pat"; then
        echo -e "${RED}❌ Invalid token format. Please try again.${NC}" >&2
        echo "" >&2
        return 1
    fi
    
    echo -e "${GREEN}✅ Token validated successfully!${NC}" >&2
    echo "" >&2
    echo "$hf_pat"
    return 0
}

# Check for required commands
echo "Checking required commands..."

# Check and install Docker if needed
echo "Checking Docker..."
if ! command_exists docker; then
    echo "Docker not found. Installing Docker for Linux..."
    
    # Check if running as root
    if [ "$EUID" -eq 0 ]; then
        echo "Warning: Running as root. Docker installation will proceed but consider running as non-root user."
    fi
    
    # Install required packages
    echo "Installing required packages..."
    sudo apt-get update
    sudo apt-get install -y ca-certificates curl gnupg
    
    # Add Docker's official GPG key
    echo "Adding Docker's official GPG key..."
    sudo install -m 0755 -d /etc/apt/keyrings
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
    sudo chmod a+r /etc/apt/keyrings/docker.gpg
    
    # Add Docker repository
    echo "Adding Docker repository..."
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
    
    # Update package index and install Docker
    echo "Installing Docker..."
    sudo apt-get update
    sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
    
    # Start Docker service
    echo "Starting Docker service..."
    sudo systemctl start docker
    sudo systemctl enable docker
    
    echo "Ensuring 'docker' group exists..."
    if ! getent group docker >/dev/null; then
        sudo groupadd docker
    fi

    echo "Adding user '${TARGET_USER}' to 'docker' group..."
    sudo usermod -aG docker "${TARGET_USER}"

    echo "Docker installed. Version:"
    sudo docker --version

    echo
    echo "👉 IMPORTANT: You must start a **new login session** for group changes to take effect."
    echo "   Options:"
    echo "     - log out and back in,"
    echo "     - or run:  su - ${TARGET_USER}"
    echo

    # Smoke test in a fresh login shell for the target user (no reliance on newgrp)
    echo "Attempting a non-root test as ${TARGET_USER} in a fresh shell..."
    if sudo -u "${TARGET_USER}" -H sh -lc 'id -nG | grep -qw docker && docker run --rm hello-world >/dev/null 2>&1'; then
        echo "Docker test successful for ${TARGET_USER}."
    else
        echo "Warning: Test could not confirm access yet (likely current session lacks new group)."
        echo "After opening a new session, verify with:"
        echo "  id -nG ${TARGET_USER}"
        echo "  docker run --rm hello-world"
    fi
else
    echo "Docker is already installed"
    docker --version
fi

# Install NVIDIA Container Toolkit
echo "Checking NVIDIA Container Toolkit..."
if ! command_exists nvidia-ctk; then
    echo "NVIDIA Container Toolkit not found. Installing..."
    
    # Add NVIDIA's package repositories
    echo "Adding NVIDIA's package repositories..."
    curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey \
        | sudo gpg --dearmor -o /usr/share/keyrings/nvidia-container-toolkit-keyring.gpg
    
    distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
    curl -fsSL https://nvidia.github.io/libnvidia-container/$distribution/libnvidia-container.list \
        | sed 's#deb https://#deb [signed-by=/usr/share/keyrings/nvidia-container-toolkit-keyring.gpg] https://#g' \
        | sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list
    
    sudo apt-get update
    
    # Install the toolkit
    echo "Installing NVIDIA Container Toolkit..."
    sudo apt-get install -y nvidia-container-toolkit
    
    # Configure Docker runtime
    echo "Configuring Docker runtime for NVIDIA..."
    sudo nvidia-ctk runtime configure --runtime=docker
    sudo systemctl restart docker
    
    echo "NVIDIA Container Toolkit installed successfully"
else
    echo "NVIDIA Container Toolkit is already installed"
    nvidia-ctk --version
fi

# Download the NVIDIA NRE container (serves the NuRec gRPC API)
echo "Checking NRE container..."
if check_NuRec_container "$NUREC_IMAGE_DEFAULT"; then
    echo "NRE container already exists, skipping download."
else
    echo "Initiating NRE Container Download..."
    docker pull "$NUREC_IMAGE_DEFAULT"

    if [ $? -ne 0 ]; then
        echo "Error: Failed to download NRE Container"
        exit 1
    fi
fi

# Download test data from HuggingFace. The dataset is gated: accept its terms
# first, then use an existing `hf auth login` session or set HF_TOKEN.
echo "Checking HuggingFace test data..."
if ! check_hf_dataset; then
    echo "Installing HuggingFace CLI..."
    $PYTHON_EXECUTABLE -m pip install --upgrade huggingface_hub || {
        echo "Error: Failed to install HuggingFace CLI"
        exit 1
    }

    if [ "$FULL_DATASET" -eq 1 ]; then
        echo "Downloading the FULL dataset (approximately 1.5 TB)..."
        include_args=()
    else
        echo "Downloading test scene $DATASET_SCENE (use --full-dataset for everything, --scene <uuid> for a different one)..."
        include_args=(--include "sample_set/$DATASET_RELEASE/$DATASET_SCENE/*")
    fi

    HF_CLI=$(dirname "$PYTHON_EXECUTABLE")/hf
    "$HF_CLI" download "$DATASET_REPO" --repo-type dataset "${include_args[@]}" \
        --local-dir "$CARLA_NUREC_ROOT/PhysicalAI-Autonomous-Vehicles-NuRec" || {
        echo "Gated download failed. Retrying with interactive authentication..."
        hf_pat=$(get_hf_pat) || exit 1
        hf_pat=$(echo "$hf_pat" | tr -d '\n\r' | xargs)
        "$HF_CLI" auth login --token "$hf_pat" || {
            echo "Error: Failed to authenticate with HuggingFace"
            exit 1
        }
        "$HF_CLI" download "$DATASET_REPO" --repo-type dataset "${include_args[@]}" \
            --local-dir "$CARLA_NUREC_ROOT/PhysicalAI-Autonomous-Vehicles-NuRec" || {
            echo "Error: Failed to download the NuRec test data from HuggingFace"
            exit 1
        }
    }
fi

# Set the NuRec image path
NUREC_IMAGE="$NUREC_IMAGE_DEFAULT"
export NUREC_IMAGE
echo "NUREC_IMAGE: $NUREC_IMAGE"

# Install Python dependencies
echo "Installing Python dependencies..."

# Install base dependencies
echo "Installing base dependencies..."
"$PYTHON_EXECUTABLE" -m pip install pygame numpy imageio || {
    echo "Error: Failed to install pygame and numpy"
    exit 1
}

# Install Carla Wheel
echo "Installing Carla Wheel..."


WHEEL_NAME_PREFIX=carla-$CARLA_VERSION-cp$PYTHON_MAJOR$PYTHON_MINOR-cp$PYTHON_MAJOR$PYTHON_MINOR

# CARLA UE5 source builds may place the wheel directly in Build/PythonAPI/dist
# or under a configuration directory; packaged releases keep the old layout.
shopt -s nullglob
WHEEL_CANDIDATES=(
    "$CARLA_ROOT"/PythonAPI/carla/dist/$WHEEL_NAME_PREFIX-*.whl
    "$CARLA_ROOT"/Build/PythonAPI/dist/$WHEEL_NAME_PREFIX-*.whl
    "$CARLA_ROOT"/Build/Release/PythonAPI/dist/$WHEEL_NAME_PREFIX-*.whl
)
shopt -u nullglob
WHEEL="${WHEEL_CANDIDATES[0]:-}"
if [ -z "$WHEEL" ]; then
    echo "Error: No carla wheel matching $WHEEL_NAME_PREFIX found."
    echo "Build the PythonAPI first, or install the carla package manually."
    exit 1
fi
"$PYTHON_EXECUTABLE" -m pip install "$WHEEL" || {
    echo "Error: Failed to install Carla Wheel"
    exit 1
}

# Install project requirements (includes grpcio/protobuf pins matching the
# vendored, checked-in gRPC stubs in nre/grpc/protos — no codegen step needed)
echo "Installing project requirements..."
"$PYTHON_EXECUTABLE" -m pip install -r "$CARLA_NUREC_ROOT/requirements.txt" || {
    echo "Error: Failed to install project requirements"
    exit 1
}

# Make the script executable
chmod +x "$0"

echo "Setup completed successfully!"
echo ""
echo "Python environment: $CARLA_NUREC_ROOT/.venv"
echo "Pinned NRE image: $NUREC_IMAGE"
