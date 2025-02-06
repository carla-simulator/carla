#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Initialize variables with default values.
interactive=0
skip_prerequisites=0
launch=0
python_root=""

# Determine the workspace path.
workspace_path="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"
echo "workspace_path=${workspace_path}"

# Parse command-line options.
options=$(getopt -o "ipl" --long "interactive,skip-prerequisites,launch,python-root:" -n 'CarlaSetup.sh' -- "$@")
eval set -- "${options}"
while true; do
    case "$1" in
        -i|--interactive)
            interactive=1
            shift
            ;;
        -p|--skip-prerequisites)
            skip_prerequisites=1
            shift
            ;;
        -l|--launch)
            launch=1
            shift
            ;;
        --python-root)
            python_root="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Invalid option: $1"
            exit 1
            ;;
    esac
done

# Function to check for root privileges.
check_root() {
    if [[ "$EUID" -ne 0 ]]; then
        if [[ $interactive -eq 0 && $skip_prerequisites -eq 0 ]]; then
            echo "Please run this script as root or pass --interactive to be prompted for privileges."
            exit 1
        fi
    fi
}

# Function to check for Git credentials.
check_git_credentials() {
    if [[ -z "$GIT_LOCAL_CREDENTIALS" ]]; then
        if [[ $interactive -eq 1 ]]; then
            echo "Warning: Git credentials are not set. You may be prompted later."
        else
            echo "Git credentials are not set; cannot continue in unattended mode."
            exit 1
        fi
    else
        echo "Found Git credentials."
    fi
}

# Function to install prerequisites.
install_prerequisites() {
    local python_path="python3"
    if [[ -n "$python_root" ]]; then
        python_path="${python_root}/python3"
    fi
    echo "Installing prerequisites..."
    sudo -E bash -x Util/SetupUtils/InstallPrerequisites.sh --python-path="$python_path"
}

# Function to clone CARLA content.
clone_carla_content() {
    if [[ -d "${workspace_path}/Unreal/CarlaUnreal/Content" ]]; then
        echo "Found CARLA content."
    else
        echo "Could not find CARLA content. Downloading..."
        mkdir -p "${workspace_path}/Unreal/CarlaUnreal/Content"
        git -C "${workspace_path}/Unreal/CarlaUnreal/Content" clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla
    fi
}

# Function to download and build Unreal Engine.
build_unreal_engine() {
    export CARLA_UNREAL_ENGINE_PATH="${workspace_path}/UnrealEngine5_carla"
    echo "CARLA_UNREAL_ENGINE_PATH=${CARLA_UNREAL_ENGINE_PATH}"

    if [[ -d "${CARLA_UNREAL_ENGINE_PATH}" ]]; then
        echo "Found CARLA Unreal Engine at ${CARLA_UNREAL_ENGINE_PATH}"
    else
        echo "Could not find CARLA Unreal Engine, downloading..."
        pushd ..
        local ue5_url="https://github.com/CarlaUnreal/UnrealEngine.git"
        if [[ -n "$GIT_LOCAL_CREDENTIALS" ]]; then
            IFS='@' read -r git_user git_token <<< "$GIT_LOCAL_CREDENTIALS"
            ue5_url="https://${git_user}:${git_token}@github.com/CarlaUnreal/UnrealEngine.git"
        fi
        git clone -b ue5-dev-carla "$ue5_url" UnrealEngine5_carla
        pushd UnrealEngine5_carla
        echo -e '\n# CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='"$PWD" >> ~/.bashrc
        export CARLA_UNREAL_ENGINE_PATH="$PWD"
        echo "Running Unreal Engine pre-build steps..."
        bash -x Setup.sh
        bash -x GenerateProjectFiles.sh
        echo "Building Unreal Engine 5..."
        make
        popd
        popd
    fi
}

# Function to build CARLA.
build_carla() {
    echo "Configuring the CARLA CMake project..."
    cmake -G Ninja -S . -B Build \
        --toolchain="$PWD/CMake/Toolchain.cmake" \
        -DLAUNCH_ARGS="-prefernvidia" \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_ROS2=ON \
        -DPython_ROOT_DIR="${python_root}" \
        -DPython3_ROOT_DIR="${python_root}" \
        -DCARLA_UNREAL_ENGINE_PATH="${CARLA_UNREAL_ENGINE_PATH}"
    echo "Building CARLA..."
    cmake --build Build
    echo "Installing Python API..."
    cmake --build Build --target carla-python-api-install
    echo "CARLA Python API build+install succeeded."
}

# Function to launch CARLA.
launch_carla() {
    if [[ $launch -eq 1 ]]; then
        echo "Launching Carla - Unreal Editor..."
        cmake --build Build --target launch
    fi
}

# Main script execution.
check_root
check_git_credentials
if [[ $skip_prerequisites -eq 0 ]]; then
    install_prerequisites
else
    echo "Skipping prerequisites install step."
fi
clone_carla_content
build_unreal_engine
build_carla
launch_carla
