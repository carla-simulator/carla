#! /usr/bin/env bash
# Install the carla-cosmos client into a venv beside the example
# (mirrors ../nurec/install_nurec.sh). Usage:
#   ./install_client.sh [-i /path/to/python]

set -euo pipefail

CARLA_VERSION=0.10.0
PYTHON_EXECUTABLE=python

COSMOS_ROOT=$(realpath "$(dirname "$(realpath "${BASH_SOURCE[0]}")")/..")
CARLA_ROOT=$(realpath "$COSMOS_ROOT/../../../..")
CLIENT_ROOT=$COSMOS_ROOT/client

options=$(getopt -o "i:" --long "python:" -n 'install_client.sh' -- "$@")
eval set -- "$options"
while true; do
    case "$1" in
        -i|--python) PYTHON_EXECUTABLE=$2; shift 2 ;;
        --) shift; break ;;
        *) echo "Unknown option $1"; exit 1 ;;
    esac
done

# Locate a CARLA wheel first: with no wheel the venv must be able to see an
# already-installed carla package, so it is created with --system-site-packages.
find_wheel() {
    local py_tag=$1
    shopt -s nullglob
    local candidates=(
        "$CARLA_ROOT"/PythonAPI/carla/dist/carla-$CARLA_VERSION-$py_tag-$py_tag-*.whl
        "$CARLA_ROOT"/Build/PythonAPI/dist/carla-$CARLA_VERSION-$py_tag-$py_tag-*.whl
        "$CARLA_ROOT"/Build/Release/PythonAPI/dist/carla-$CARLA_VERSION-$py_tag-$py_tag-*.whl
    )
    shopt -u nullglob
    echo "${candidates[0]:-}"
}

# Default to an isolated environment beside the example. --python remains an
# escape hatch for callers that already created a compatible environment.
if [ "$PYTHON_EXECUTABLE" = python ]; then
    if [ ! -x "$CLIENT_ROOT/.venv/bin/python" ]; then
        IFS=. read -r BASE_MAJOR BASE_MINOR _ <<< "$(python3 -c 'import sys; print(".".join(map(str, sys.version_info[:3])))')"
        VENV_FLAGS=()
        if [ -z "$(find_wheel cp$BASE_MAJOR$BASE_MINOR)" ] && python3 -c 'import carla' 2>/dev/null; then
            echo "No CARLA wheel found; exposing the base interpreter's carla package to the venv."
            VENV_FLAGS+=(--system-site-packages)
        fi
        python3 -m venv "${VENV_FLAGS[@]}" "$CLIENT_ROOT/.venv"
    fi
    PYTHON_EXECUTABLE="$CLIENT_ROOT/.venv/bin/python"
fi
echo "Using python interpreter $PYTHON_EXECUTABLE"

IFS=. read -r PY_MAJOR PY_MINOR _ <<< "$($PYTHON_EXECUTABLE -c 'import sys; print(".".join(map(str, sys.version_info[:3])))')"
if [ "$PY_MAJOR" -lt 3 ] || { [ "$PY_MAJOR" -eq 3 ] && [ "$PY_MINOR" -lt 10 ]; }; then
    echo "Error: carla-cosmos needs Python >= 3.10 (got $PY_MAJOR.$PY_MINOR)"
    exit 1
fi

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "Warning: ffmpeg not found on PATH; video encoding will fail (apt install ffmpeg)."
fi

"$PYTHON_EXECUTABLE" -m pip install --upgrade pip

# Install the CARLA wheel from the source build if present; otherwise the
# environment must already provide the carla package.
WHEEL=$(find_wheel "cp$PY_MAJOR$PY_MINOR")
if [ -n "$WHEEL" ]; then
    echo "Installing CARLA wheel $WHEEL"
    "$PYTHON_EXECUTABLE" -m pip install "$WHEEL"
elif "$PYTHON_EXECUTABLE" -c 'import carla' 2>/dev/null; then
    echo "No wheel found for this interpreter; using the already-importable carla package."
else
    echo "Error: no carla wheel for cp$PY_MAJOR$PY_MINOR under $CARLA_ROOT and 'import carla' fails."
    echo "Build the PythonAPI first, or point -i at a python that has carla installed."
    exit 1
fi

echo "Installing carla-cosmos (editable, with capture, viewer and test extras)..."
"$PYTHON_EXECUTABLE" -m pip install -e "$CLIENT_ROOT[capture,viewer,test]"

echo ""
echo "Setup completed successfully."
echo "Python environment: $CLIENT_ROOT/.venv"
echo "Try: $PYTHON_EXECUTABLE $COSMOS_ROOT/demos/single_view_live.py --port 2000 --capture-only"
