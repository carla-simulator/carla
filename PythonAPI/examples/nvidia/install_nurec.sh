#!/bin/bash

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}



# Function to check if Hugging Face dataset exists
check_hf_dataset() {
    local dataset_path="PhysicalAI-Autonomous-Vehicles-NuRec"
    if [ -d "$dataset_path" ]; then
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

# Function to validate Hugging Face PAT format (basic check)
validate_hf_pat() {
    if [[ ! $1 =~ ^hf_[a-zA-Z0-9]{32,}$ ]]; then
        echo "Error: Invalid Hugging Face PAT format" >&2
        return 1
    fi
    return 0
}

# Function to get and validate Hugging Face PAT
get_hf_pat() {
    # Get Hugging Face PAT
    read -s -p "Please enter your Hugging Face Personal Access Token. If you don't have one, visit: https://huggingface.co/settings/tokens. Enter your token: " hf_pat
    echo  

    # Validate Hugging Face PAT
    if ! validate_hf_pat "$hf_pat"; then
        return 1
    fi
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
    
    # Add current user to docker group (if not root)
    if [ "$EUID" -ne 0 ]; then
        echo "Adding current user ($USER) to docker group..."
        sudo usermod -aG docker "$USER"
        echo "Note: You may need to log out and log back in for group changes to take effect"
        echo "Or run: newgrp docker"
    fi
    
    # Verify Docker installation
    if command_exists docker; then
        echo "Docker installed successfully"
        docker --version
        
        # Test Docker with a simple command
        echo "Testing Docker installation..."
        if [ "$EUID" -eq 0 ]; then
            docker run --rm hello-world >/dev/null 2>&1
        else
            # For non-root users, use newgrp to test with new group membership
            echo "Testing Docker (you may be prompted for password)..."
            newgrp docker <<EOF
docker run --rm hello-world >/dev/null 2>&1
EOF
        fi
        
        if [ $? -eq 0 ]; then
            echo "Docker test successful"
        else
            echo "Warning: Docker test failed. You may need to restart your session or run 'newgrp docker'"
        fi
    else
        echo "Error: Docker installation verification failed"
        exit 1
    fi
else
    echo "Docker is already installed"
    docker --version
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
if check_NuRec_container "docker.io/carlasimulator/nvidia-nurec-grpc:0.1.0"; then
    echo "NuRec GRPC container already exists, skipping download."
else
    echo "Initiating NuRec GRPC Container Downloads..."
    docker pull docker.io/carlasimulator/nvidia-nurec-grpc:0.1.0
    if [ $? -ne 0 ]; then
        echo "Error: Failed to download NuRec GRPC Container"
        exit 1
    fi
fi

# Download the dataset from Hugging Face
echo "Checking Hugging Face dataset..."
if check_hf_dataset; then
    echo "Hugging Face dataset already exists, skipping download."
else
    # Get and validate Hugging Face PAT
    hf_pat=$(get_hf_pat)
    if [ $? -ne 0 ]; then
        exit 1
    fi

    # Strip any newlines or whitespace from the token
    hf_pat=$(echo "$hf_pat" | tr -d '\n\r' | xargs)

    # Create the Hugging Face URL with embedded token
    hf_url="https://user:${hf_pat}@huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec/"

    echo "Downloading the dataset from Hugging Face..."
    git lfs install
    
    # Set git configuration to avoid prompts
    export GIT_TERMINAL_PROMPT=0  # Disable git prompts
    
    # Clone the repository using the URL with embedded token
    echo "Cloning dataset with authentication..."
    git clone "$hf_url"
    
    clone_result=$?
    unset GIT_TERMINAL_PROMPT
    
    if [ $clone_result -ne 0 ]; then
        echo "Error: Failed to download the NuRec dataset from Hugging Face"
        exit 1
    fi
fi

# Set the NuRec image path
NUREC_IMAGE="docker.io/carlasimulator/nvidia-nurec-grpc:0.1.0"
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
pip3 install pygame numpy nvidia-nvjpeg-cu12 imageio|| {
    echo "Error: Failed to install pygame and numpy"
    exit 1
}

# Install Carla Wheel
echo "Installing Carla Wheel..."

WHEEL=$(ls ../../carla/dist/carla-0.9.16-cp310-cp310-*.whl | head -n 1)
python -m pip install ${WHEEL} || {
    echo "Error: Failed to install Carla Wheel"
    exit 1
}

# Install project requirements
echo "Installing project requirements..."
pip3 install -r requirements.txt || {
    echo "Error: Failed to install project requirements"
    exit 1
}

# Install and setup GRPC Protos
echo "Setting up GRPC Protos..."
pip3 install -r grpc_proto/requirements.txt || {
    echo "Error: Failed to install GRPC requirements"
    exit 1
}

python3 grpc_proto/update_generated.py || {
    echo "Error: Failed to update generated GRPC files"
    exit 1
}

# Make the script executable
chmod +x "$0"

echo "Setup completed successfully!"
