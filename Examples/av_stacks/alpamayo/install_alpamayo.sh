#!/usr/bin/env bash

set -euo pipefail

ALPAMAYO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CARLA_ROOT="$(realpath "${ALPAMAYO_ROOT}/../../..")"
SOURCE_DIR="${ALPAMAYO_ROOT}/.cache/src/alpamayo2"
MODEL_ENV="${ALPAMAYO_ROOT}/.venv-model"
UV_CACHE_DIR="${ALPAMAYO_ROOT}/.cache/uv"
CONTROL_PYTHON="python3"
MODEL_ID="nvidia/Alpamayo2-Super"
MODEL_PYTHON_VERSION="3.12"
DOWNLOAD_MODEL=1

usage() {
    cat <<'EOF'
Install NVIDIA Alpamayo 2 Super for the CARLA closed-loop demo.

Options:
  --python PATH              Python used by run_alpamayo.sh (default: python3)
  --source-dir PATH          Alpamayo source checkout
  --model-env PATH           Python 3.12 model environment
  --model-id ID              Hugging Face model id
  --no-model-download        Install code only; do not fetch gated weights
  -h, --help                 Show this help

The model is gated. Accept its license at:
  https://huggingface.co/nvidia/Alpamayo2-Super
EOF
}

options=$(getopt -o h --long help,python:,source-dir:,model-env:,model-id:,no-model-download -n install_alpamayo.sh -- "$@")
eval set -- "$options"
while true; do
    case "$1" in
        --python) CONTROL_PYTHON="$2"; shift 2 ;;
        --source-dir) SOURCE_DIR="$2"; shift 2 ;;
        --model-env) MODEL_ENV="$2"; shift 2 ;;
        --model-id) MODEL_ID="$2"; shift 2 ;;
        --no-model-download) DOWNLOAD_MODEL=0; shift ;;
        -h|--help) usage; exit 0 ;;
        --) shift; break ;;
        *) echo "Unknown option: $1" >&2; exit 2 ;;
    esac
done

command_exists() { command -v "$1" >/dev/null 2>&1; }

if ! command_exists git; then
    echo "Error: git is required." >&2
    exit 1
fi
if ! command_exists curl; then
    echo "Error: curl is required to bootstrap uv." >&2
    exit 1
fi
if ! command_exists nvidia-smi; then
    echo "Error: an NVIDIA GPU and driver are required." >&2
    exit 1
fi

if ! command_exists uv; then
    echo "Installing uv for the current user..."
    curl -LsSf https://astral.sh/uv/install.sh | sh
    export PATH="${HOME}/.local/bin:${HOME}/.cargo/bin:${PATH}"
fi
if ! command_exists uv; then
    echo "Error: uv installation completed but uv is not on PATH." >&2
    exit 1
fi

if [[ -z "${CUDA_HOME:-}" ]]; then
    if [[ -d /usr/local/cuda-12.8 ]]; then
        export CUDA_HOME=/usr/local/cuda-12.8
    elif command_exists nvcc; then
        CUDA_HOME="$(dirname "$(dirname "$(command -v nvcc)")")"
        export CUDA_HOME
    fi
fi
if [[ -z "${CUDA_HOME:-}" || ! -x "${CUDA_HOME}/bin/nvcc" ]]; then
    echo "Error: CUDA Toolkit 12.x with nvcc is required to build flash-attn." >&2
    echo "Set CUDA_HOME to the toolkit root and rerun this installer." >&2
    exit 1
fi
export PATH="${CUDA_HOME}/bin:${PATH}"
export LD_LIBRARY_PATH="${CUDA_HOME}/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export MAX_JOBS="${MAX_JOBS:-4}"
export UV_CACHE_DIR

echo "Checking the CARLA control-side Python..."
"${CONTROL_PYTHON}" -m pip install --user -r "${ALPAMAYO_ROOT}/requirements.txt"
if ! "${CONTROL_PYTHON}" -c 'import carla' >/dev/null 2>&1; then
    major_minor=$("${CONTROL_PYTHON}" -c 'import sys; print(f"{sys.version_info.major}{sys.version_info.minor}")')
    wheel=$(find "${CARLA_ROOT}/Build/PythonAPI/dist" -maxdepth 1 -type f -name "carla-*cp${major_minor}-cp${major_minor}-*.whl" -print -quit 2>/dev/null || true)
    if [[ -z "$wheel" ]]; then
        echo "Error: CARLA is not importable by ${CONTROL_PYTHON}, and no matching built wheel was found." >&2
        echo "Build/install the CARLA Python API, then rerun this installer." >&2
        exit 1
    fi
    "${CONTROL_PYTHON}" -m pip install --user "$wheel"
fi

mkdir -p "$(dirname "${SOURCE_DIR}")" "${UV_CACHE_DIR}"
if [[ ! -d "${SOURCE_DIR}/.git" ]]; then
    echo "Cloning NVIDIA Alpamayo 2..."
    git clone https://github.com/NVlabs/alpamayo2.git "${SOURCE_DIR}"
else
    echo "Updating the existing Alpamayo 2 checkout..."
    git -C "${SOURCE_DIR}" pull --ff-only origin main
fi

echo "Installing Python ${MODEL_PYTHON_VERSION} and the locked Alpamayo environment..."
uv python install "${MODEL_PYTHON_VERSION}"
(
    cd "${SOURCE_DIR}"
    UV_PROJECT_ENVIRONMENT="${MODEL_ENV}" uv sync --locked --no-dev --python "${MODEL_PYTHON_VERSION}"
)

HF="${MODEL_ENV}/bin/hf"
if [[ ! -x "$HF" ]]; then
    echo "Error: Hugging Face CLI was not installed in ${MODEL_ENV}." >&2
    exit 1
fi

if [[ "$DOWNLOAD_MODEL" -eq 1 ]]; then
    echo
    echo "The gated model requires accepted access at:"
    echo "  https://huggingface.co/${MODEL_ID}"
    if ! "$HF" auth whoami >/dev/null 2>&1; then
        if [[ -n "${HF_TOKEN:-}" ]]; then
            "$HF" auth login --token "${HF_TOKEN}"
        else
            "$HF" auth login
        fi
    fi
    echo "Downloading ${MODEL_ID}; this requires roughly 67 GB in the Hugging Face cache..."
    "$HF" download "${MODEL_ID}"
fi

echo
echo "Alpamayo installation complete."
echo "Model environment: ${MODEL_ENV}"
echo "Source checkout:   ${SOURCE_DIR}"
echo
echo "Run the closed-loop demo:"
echo "  cd ${ALPAMAYO_ROOT}"
echo "  ./run_alpamayo.sh --expected-map Town10HD_Opt"
