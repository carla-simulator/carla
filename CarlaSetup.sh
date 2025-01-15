#! /bin/bash

set -e

interactive=0
skip_prerequisites=0
launch=0

workspace_path="$(dirname $0)"

options=$(getopt -o "i,p,l" --long "interactive,skip-prerequisites,launch" -n 'CarlaSetup.sh' -- "$@")
eval set -- "$options"
while true; do
    case "$1" in
        -i | --interactive)
            interactive=1
            shift
            ;;
        -p | --skip-prerequisites)
            skip_prerequisites=1
            shift
            ;;
        -l | --launch)
            launch=1
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            ;;
    esac
done

# Check for root privileges:
if [ -z "$EUID" ]; then
    EUID=$(id -u)
fi
if [ "$EUID" -ne 0 ]; then
    if [ $interactive -eq 0 ]; then
        if [ $skip_prerequisites -eq 0 ]; then
            echo "Please run this script as root. Otherwise pass --interactive to be prompted whenever root privileges or Git credentials are needed."
            exit 1
        fi
    fi
fi

# Check for Git credentials:
if [ -z "$GIT_LOCAL_CREDENTIALS" ]; then
    if [ $interactive -eq 1 ]; then
        echo "Warning: git credentials are not set. You may be required to manually enter them later."
    else
        echo "Git credentials are not set, can not continue setup in unattended mode."
        exit 1
    fi
else
    echo "Found git credentials."
fi

# -- PREREQUISITES INSTALL STEP --
if [ $skip_prerequisites -eq 0 ]; then
    echo "Installing prerequisites..."
    sudo -E bash -x Util/SetupUtils/InstallPrerequisites.sh
else
    echo "Skipping prerequisites install step."
fi

# -- CLONE CONTENT --
if [ -d $workspace_path/Unreal/CarlaUnreal/Content ]; then
    echo "Found CARLA content."
else
    echo "Could not find CARLA content. Downloading..."
    mkdir -p $workspace_path/Unreal/CarlaUnreal/Content
    git \
        -C $workspace_path/Unreal/CarlaUnreal/Content \
        clone \
        -b ue5-dev \
        https://bitbucket.org/carla-simulator/carla-content.git \
        Carla
fi

# -- DOWNLOAD + BUILD UNREAL ENGINE --
if [ ! -z $CARLA_UNREAL_ENGINE_PATH ] && [ -d $CARLA_UNREAL_ENGINE_PATH ]; then
    echo "Found CARLA Unreal Engine at $CARLA_UNREAL_ENGINE_PATH"
elif [ -d ../UnrealEngine5_carla ]; then
    echo "Found CARLA Unreal Engine at $workspace_path/UnrealEngine5_carla. Assuming already built..."
else
    echo "Could not find CARLA Unreal Engine, downloading..."
    pushd ..
    if [ -z "$GIT_LOCAL_CREDENTIALS" ]
    then
        UE5_URL=https://github.com/CarlaUnreal/UnrealEngine.git
    else
        GIT_CREDENTIALS_INFO=(${GIT_LOCAL_CREDENTIALS//@/ })
        GIT_LOCAL_USER=${GIT_CREDENTIALS_INFO[0]}
        GIT_LOCAL_TOKEN=${GIT_CREDENTIALS_INFO[1]}
        UE5_URL=https://$GIT_LOCAL_USER:$GIT_LOCAL_TOKEN@github.com/CarlaUnreal/UnrealEngine.git
    fi
    git clone -b ue5-dev-carla $UE5_URL UnrealEngine5_carla
    pushd UnrealEngine5_carla
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$PWD >> ~/.bashrc
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    echo "Running Unreal Engine pre-build steps..."
    bash -x Setup.sh
    bash -x GenerateProjectFiles.sh
    echo "Building Unreal Engine 5..."
    make
    popd
    popd
fi

# -- BUILD CARLA --
echo "Configuring the CARLA CMake project..."
cmake -G Ninja -S . -B Build \
    --toolchain=$PWD/CMake/LinuxToolchain.cmake \
    -DLAUNCH_ARGS="-prefernvidia" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_ROS2=ON \
    -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
echo "Building CARLA..."
cmake --build Build
echo "Installing Python API..."
cmake --build Build --target carla-python-api-install
echo "CARLA Python API build+install succeeded."

# -- POST-BUILD STEPS --
if [ $launch -eq 1 ]; then
    echo "Launching Carla - Unreal Editor..."
    cmake --build Build --target launch
fi
