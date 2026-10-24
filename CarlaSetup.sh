#! /bin/bash

set -e

interactive=0
skip_prerequisites=0
launch=0
python_root=

workspace_path="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
echo "workspace_path=$workspace_path"

if [ -d "/opt/homebrew/opt/gnu-getopt/bin" ]; then
    export PATH="/opt/homebrew/opt/gnu-getopt/bin:$PATH"
fi

options=$(\
    getopt \
    -o "i,p,l,pyroot:" \
    --long "interactive,skip-prerequisites,launch,python-root:" \
    -n 'CarlaSetup.sh' -- "$@")

eval set -- "$options"
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
        -pyroot|--python-root)
            python_root=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            ;;
    esac
done

# Check if root is asking for a password:
if [[ "$OSTYPE" != "darwin"* ]] && [ "$(uname -s 2>/dev/null)" != "Darwin" ]; then
    if [ -z "$EUID" ]; then
        EUID=$(id -u)
    fi
    if [ "$EUID" -ne 0 ] && [ $interactive -eq 0 ] && [ $skip_prerequisites -eq 0 ]; then
        if ! sudo -n true 2>/dev/null; then
            echo "Please run 'sudo -v' before running this script, or pass --interactive or --skip-prerequisites."
            exit 1
        fi
    fi
fi

# Check for Git credentials:
# if [ -z "$GIT_LOCAL_CREDENTIALS" ]; then
#     if [ $interactive -eq 1 ]; then
#         echo "Warning: git credentials are not set. You may be required to manually enter them later."
#     else
#         echo "Git credentials are not set, can not continue setup in unattended mode."
#         exit 1
#     fi
# else
#     echo "Found git credentials."
# fi

# -- PREREQUISITES INSTALL STEP --
if [ $skip_prerequisites -eq 0 ]; then
    python_path=python3
    if [ "$python_root" != "" ]; then
        python_path=${python_root}/python3
    elif [ -x "/opt/homebrew/bin/python3.10" ]; then
        python_path="/opt/homebrew/bin/python3.10"
    fi
    if [[ "$OSTYPE" == "darwin"* ]] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
        echo "macOS detected: verifying prerequisites..."
        if command -v brew >/dev/null 2>&1; then
            echo "Homebrew is available."
        else
            echo "Warning: Homebrew not found. Please install prerequisites via Homebrew (cmake, ninja, gnu-getopt, etc.)."
        fi
        git lfs install
        $python_path -m pip install -r requirements.txt --break-system-packages 2>/dev/null || $python_path -m pip install -r requirements.txt || true
    else
        echo "Installing prerequisites..."
        bash -x Util/SetupUtils/InstallPrerequisites.sh --python-path=$python_path
    fi
else
    echo "Skipping prerequisites install step."
fi

# -- CLONE CONTENT --
if [ -d $workspace_path/Unreal/CarlaUnreal/Content/Carla ]; then
    echo "Found CARLA content."
else
    echo "Could not find CARLA content. Downloading..."
    mkdir -p $workspace_path/Unreal/CarlaUnreal/Content
    git \
        -C $workspace_path/Unreal/CarlaUnreal/Content \
        clone \
        -b ue58-dev-carla \
        https://bitbucket.org/carla-simulator/carla-content.git \
        Carla
fi

# -- DLSS SDK (required, NVIDIA-licensed, never vendored) --
# Fetches github.com/NVIDIA/DLSS into ~/SDKs/DLSS (unless DLSS_SDK already
# points at a valid SDK) so the engine's DLSS-RR denoiser / DLSS upscaler
# plugin builds with NGX support. The engine build requires the SDK;
# building without DLSS (no-op plugin, NFOR / spatial upscale fallbacks)
# is an explicit opt-in via DLSS_SDK=disabled.
bash Util/SetupUtils/SetupDLSS.sh || exit 1
if [ -z "$DLSS_SDK" ] && [ -f "$HOME/SDKs/DLSS/lib/Linux_x86_64/libnvsdk_ngx.a" ]; then
    export DLSS_SDK=$HOME/SDKs/DLSS
fi

# -- DOWNLOAD + BUILD UNREAL ENGINE --
if [ -z "$CARLA_UNREAL_ENGINE_PATH" ] && [ -d ../UnrealEngine5_carla ]; then
    export CARLA_UNREAL_ENGINE_PATH="$(cd ../UnrealEngine5_carla && pwd)"
fi

if [ -n "$CARLA_UNREAL_ENGINE_PATH" ] && [ -d "$CARLA_UNREAL_ENGINE_PATH" ]; then
    echo "Found CARLA Unreal Engine at $CARLA_UNREAL_ENGINE_PATH"
else
    echo "Could not find CARLA Unreal Engine, downloading..."
    pushd ..
    if [ -z "$GIT_LOCAL_CREDENTIALS" ]; then
        UE5_URL=https://github.com/CarlaUnreal/UnrealEngine.git
    else
        GIT_CREDENTIALS_INFO=(${GIT_LOCAL_CREDENTIALS//@/ })
        GIT_LOCAL_USER=${GIT_CREDENTIALS_INFO[0]}
        GIT_LOCAL_TOKEN=${GIT_CREDENTIALS_INFO[1]}
        UE5_URL=https://$GIT_LOCAL_USER:$GIT_LOCAL_TOKEN@github.com/CarlaUnreal/UnrealEngine.git
    fi
    CARLA_UNREAL_COMMIT="0703595a065dee51538720027ce9cc05bd464517"
    git clone -b ue58-dev-carla $UE5_URL UnrealEngine5_carla
    pushd UnrealEngine5_carla
    echo "Checking out pinned Unreal Engine commit $CARLA_UNREAL_COMMIT..."
    git checkout $CARLA_UNREAL_COMMIT
    popd
    export CARLA_UNREAL_ENGINE_PATH="$(cd UnrealEngine5_carla && pwd)"
    popd
fi

# Ensure engine is built:
UE_EDITOR_BIN=""
if [[ "$OSTYPE" == "darwin"* ]] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
    if [ -f "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/.editor_build_complete" ] && \
       ([ -x "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" ] || [ -x "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor" ]); then
        UE_EDITOR_BIN="$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/UnrealEditor"
    fi
else
    if [ -x "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor" ]; then
        UE_EDITOR_BIN="$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor"
    fi
fi

if [ -z "$UE_EDITOR_BIN" ]; then
    echo "UnrealEditor binary not found or build incomplete. Running Unreal Engine build steps..."
    pushd "$CARLA_UNREAL_ENGINE_PATH"
    if [[ "$OSTYPE" == "darwin"* ]] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
        ./Setup.command
        ./GenerateProjectFiles.command

        # Apply macOS compatibility patch if present
        PATCH_FILE="$workspace_path/Util/Patches/UnrealEngine_macOS.patch"
        if [ -f "$PATCH_FILE" ]; then
            if git apply --check "$PATCH_FILE" >/dev/null 2>&1; then
                echo "Applying macOS compatibility patch to Unreal Engine..."
                git apply "$PATCH_FILE"
                echo "Unreal Engine macOS patch successfully applied."
            elif git apply --reverse --check "$PATCH_FILE" >/dev/null 2>&1; then
                echo "Unreal Engine macOS patch is already applied."
            else
                echo "Warning: Unreal Engine patch could not be verified cleanly."
            fi
        fi

        echo "Building Unreal Engine 5 for macOS..."
        ./Engine/Build/BatchFiles/Mac/Build.sh ShaderCompileWorker Mac Development
        ./Engine/Build/BatchFiles/Mac/Build.sh UnrealEditor Mac Development
        touch "$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Mac/.editor_build_complete"
    else
        bash -x Setup.sh
        bash -x GenerateProjectFiles.sh
        echo "Building Unreal Engine 5..."
        make
    fi
    popd
else
    echo "UnrealEditor already built at $UE_EDITOR_BIN"
fi

# -- BUILD CARLA --
echo "Configuring the CARLA CMake project..."
if [[ "$OSTYPE" == "darwin"* ]] || [ "$(uname -s 2>/dev/null)" = "Darwin" ]; then
    python_args=()
    if [ -n "$python_root" ]; then
        python_args+=(-DPython_ROOT_DIR="${python_root}" -DPython3_ROOT_DIR="${python_root}")
    elif [ -x "/opt/homebrew/bin/python3.10" ]; then
        python_args+=(-DPython_EXECUTABLE=/opt/homebrew/bin/python3.10 -DPython3_EXECUTABLE=/opt/homebrew/bin/python3.10)
    fi
    cmake -G Ninja -S . -B Build \
        --toolchain=$PWD/CMake/Toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_ROS2=ON \
        -DCARLA_DLSS_SDK_PATH=disabled \
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON \
        -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON \
        "${python_args[@]}" \
        -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
else
    cmake -G Ninja -S . -B Build \
        --toolchain=$PWD/CMake/Toolchain.cmake \
        -DLAUNCH_ARGS="-prefernvidia" \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_ROS2=ON \
        -DPython_ROOT_DIR=${python_root} \
        -DPython3_ROOT_DIR=${python_root} \
        -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
fi
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
