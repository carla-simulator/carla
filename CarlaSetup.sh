#!/bin/bash

set -e

interactive=0
skip_prerequisites=0

options=$(getopt -o "i,p" --long "interactive,skip-prerequisites" -n 'CarlaSetup.sh' -- "$@")
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
CREDENTIAL_INFO=(${GIT_LOCAL_CREDENTIALS//@/ })
GIT_LOCAL_USER=${CREDENTIAL_INFO[0]}
GIT_LOCAL_TOKEN=${CREDENTIAL_INFO[1]}
if [ -z "$GIT_LOCAL_CREDENTIALS" ]; then
    if [ $interactive -eq 1 ]; then
        echo "Warning: git credentials are not set. You may be required to manually enter them later."
    else
        echo "Git credentials are not set, can not continue setup in unatteded mode."
        exit 1
    fi
fi

if [ $skip_prerequisites -eq 0 ]; then
    echo "Installing prerequisites..."
    sudo bash -x Util/SetupUtils/InstallPrerequisites.sh
else
    echo "Skipping prerequisites install step."
fi

if [ ! -z $CARLA_UNREAL_ENGINE_PATH ] && [ -d $CARLA_UNREAL_ENGINE_PATH ]; then
    echo "Found CARLA Unreal Engine at $CARLA_UNREAL_ENGINE_PATH"
elif [ -d ../UnrealEngine5_carla ]; then
    pushd ..
    pushd UnrealEngine5_carla
    echo "Found CARLA Unreal Engine at ../UnrealEngine5_carla"
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$CARLA_UNREAL_ENGINE_PATH >> ~/.bashrc
    popd
    popd
else
    echo "Could not find CARLA Unreal Engine, downloading..."
    pushd ..
    if [ -z "$GIT_LOCAL_CREDENTIALS" ]
    then
        git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
    else
        git clone -b ue5-dev-carla https://$GIT_LOCAL_USER:$GIT_LOCAL_TOKEN@github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
    fi
    pushd UnrealEngine5_carla
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$PWD >> ~/.bashrc
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    popd
    popd
    echo "Installed CARLA Unreal Engine..."
fi
pushd ..
pushd $CARLA_UNREAL_ENGINE_PATH
echo Checking if Unreal Engine is in the most recent commit...
git fetch
if [[ $(git status) =~ "up to date" ]]; then
    echo CARLA Unreal Engine is already in the last commit
else
    echo CARLA Unreal Engine is NOT in the last commit.
    echo Running git clean...
    git clean -fdx
    echo Running git pull...
    git pull
fi
echo "Running CARLA Unreal Engine setup..."
./Setup.sh --force
echo "Creating CARLA Unreal Engine project files..."
./GenerateProjectFiles.sh
echo "Building CARLA Unreal Engine..."
make
popd
popd

echo "Configuring CARLA..."
retry --until=success --times=10 -- cmake -G Ninja -S . -B Build \
    --toolchain=$PWD/CMake/LinuxToolchain.cmake \
    -DLAUNCH_ARGS="-prefernvidia" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_ROS2=ON \
    -DBUILD_CARLA_UNREAL=ON \
    -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
echo "Building CARLA..."
retry --until=success --times=10 -- cmake --build Build
echo "Building + installing Python API..."
cmake --build Build --target carla-python-api-install
echo "Waiting for Content to finish downloading..."
wait #Waitting for content
echo "Installation and build succesful."
echo "Lauching Carla - Unreal Editor..."
cmake --build Build --target launch
