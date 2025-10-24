#! /usr/bin/env bash

set -euo pipefail

PYTHON_EXECUTABLE=python3

CARLA_ROOT=$(realpath "$BASH_SOURCE[0]")
CARLA_ROOT=$(dirname "$CARLA_ROOT")
CARLA_ROOT=$CARLA_ROOT/../../../..
CARLA_ROOT=$(realpath "$CARLA_ROOT")
PYTHON_API_ROOT=$CARLA_ROOT/PythonAPI
CARLA_NUREC_ROOT=$PYTHON_API_ROOT/examples/nvidia/nurec

options=$( \
    getopt \
    -o "i:" \
    --long "python:" \
    -n 'install_nurec.sh' -- "$@")

eval set -- "$options"

while true; do
    case "$1" in
        -i|--python)
            PYTHON_EXECUTABLE=$2
            shift 2
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

echo Using python interpreter $PYTHON_EXECUTABLE.

command_exists() { command -v "$1" >/dev/null 2>&1; }

# The user we want to grant docker access to (the one who invoked sudo, if any)
TARGET_USER="${SUDO_USER:-$USER}"


# Function to check if HuggingFace dataset exists
check_hf_dataset() {
    local dataset_path="PhysicalAI-Autonomous-Vehicles-NuRec"
    local dataset_path_real=$(realpath "$dataset_path")
    if [ -d "$dataset_path" ]; then
        echo "HuggingFace dataset already exists, skipping download."
        echo "If you need to download it again, delete $dataset_path_real."
        return 0
    fi
    return 1
}

# Function to check if NuRec container exists
check_NuRec_container() {
    local container_name=$1
    if docker images | grep -q "$container_name"; then
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

# Check for other required commands
for cmd in pip; do
    if ! command_exists $cmd; then
        echo "Error: $cmd is not installed. Please install it first."
        exit 1
    fi
done

# Download NuRec GRPC Container
echo "Checking NuRec GRPC container..."
if check_NuRec_container "docker.io/carlasimulator/nvidia-nurec-grpc:0.2.0"; then
    echo "NuRec GRPC container already exists, skipping download."
else
    echo "Initiating NuRec GRPC Container Downloads..."
    sudo -E docker pull docker.io/carlasimulator/nvidia-nurec-grpc:0.2.0
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to download NuRec GRPC Container"
        exit 1
    fi
fi

# Download the dataset from HuggingFace
echo "Checking HuggingFace dataset..."
if ! check_hf_dataset; then
    echo "Installing HuggingFace CLI..."
    $PYTHON_EXECUTABLE -m pip install --upgrade huggingface_hub || {
        echo "Error: Failed to install HuggingFace CLI"
        exit 1
    }

    # Get and validate HuggingFace PAT
    hf_pat=$(get_hf_pat)
    if [ $? -ne 0 ]; then
        exit 1
    fi

    # Strip any newlines or whitespace from the token
    hf_pat=$(echo "$hf_pat" | tr -d '\n\r' | xargs)

    echo "Downloading the dataset from HuggingFace using CLI..."
    
    # Login to HuggingFace using the token
    echo "$hf_pat" | hf auth login --token "$hf_pat" || {
        echo "Error: Failed to authenticate with HuggingFace"
        exit 1
    }
    
    # Download the dataset using HuggingFace CLI
    echo "Downloading dataset with HuggingFace CLI..."
    hf download nvidia/PhysicalAI-Autonomous-Vehicles-NuRec --repo-type dataset --local-dir PhysicalAI-Autonomous-Vehicles-NuRec || {
        echo "Error: Failed to download the NuRec dataset from HuggingFace"
        exit 1
    }
fi

# Set the NuRec image path
NUREC_IMAGE="docker.io/carlasimulator/nvidia-nurec-grpc:0.2.0"
export NUREC_IMAGE
echo "NUREC_IMAGE: $NUREC_IMAGE"

# Function to make NUREC_IMAGE export persistent
make_nurec_export_persistent() {
    local export_line="export NUREC_IMAGE=\"$NUREC_IMAGE\""
    local profile_file="$HOME/.bashrc"

    # Check if the export already exists and matches the current value
    if grep -q "export NUREC_IMAGE=" "$profile_file"; then
        # If the value is different, update it
        if ! grep -qF "$export_line" "$profile_file"; then
            echo "Updating NUREC_IMAGE export in $profile_file..."
            # Use sed to replace the line
            sed -i '/export NUREC_IMAGE=/c\'"$export_line" "$profile_file"
            echo "Updated NUREC_IMAGE export in $profile_file"
        else
            echo "NUREC_IMAGE export already exists and is up to date in $profile_file"
        fi
    else
        echo "Making NUREC_IMAGE export persistent..."
        echo "" >> "$profile_file"
        echo "# NuRec environment variable" >> "$profile_file"
        echo "$export_line" >> "$profile_file"
        echo "Added NUREC_IMAGE export to $profile_file"
    fi
}

# Call the function to persist the export
make_nurec_export_persistent

# Install Python dependencies
echo "Installing Python dependencies..."

# Install base dependencies
echo "Installing base dependencies..."
$PYTHON_EXECUTABLE -m pip install pygame numpy nvidia-nvjpeg-cu12 imageio|| {
    echo "Error: Failed to install pygame and numpy"
    exit 1
}

# Install Carla Wheel
echo "Installing Carla Wheel..."

WHEEL=$(ls $CARLA_ROOT/PythonAPI/carla/dist/carla-0.9.16-cp310-cp310-*.whl | head -n 1)
$PYTHON_EXECUTABLE -m pip install ${WHEEL} || {
    echo "Error: Failed to install Carla Wheel"
    exit 1
}

# Install project requirements
echo "Installing project requirements..."
$PYTHON_EXECUTABLE -m pip install -r $CARLA_NUREC_ROOT/requirements.txt || {
    echo "Error: Failed to install project requirements"
    exit 1
}

# Install and setup GRPC Protos
echo "Setting up GRPC Protos..."
$PYTHON_EXECUTABLE -m pip install -r $CARLA_NUREC_ROOT/nre/grpc/requirements.txt || {
    echo "Error: Failed to install GRPC requirements"
    exit 1
}

$PYTHON_EXECUTABLE $CARLA_NUREC_ROOT/nre/grpc/update_generated.py || {
    echo "Error: Failed to update generated GRPC files"
    exit 1
}

# Make the script executable
chmod +x "$0"

echo "Setup completed successfully!"
echo ""
echo "🔔 IMPORTANT: Environment Variable Notice"
echo "=============================================="
echo "The NUREC_IMAGE environment variable has been added to your ~/.bashrc file"
echo "and will be available in new terminal sessions."
echo ""
echo "To use it in your CURRENT terminal session, run one of these commands:"
echo ""
echo "  Option 1 (Reload bashrc):"
echo "    source ~/.bashrc"
echo ""
echo "  Option 2 (Export manually for this session):"
echo "    export NUREC_IMAGE=\"$NUREC_IMAGE\""
echo ""
echo "  Option 3 (Start a new terminal session)"
echo ""
echo "You can verify the variable is set by running:"
echo "    echo \$NUREC_IMAGE"
