#!/usr/bin/env bash

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
NUREC_DIR="$HERE/../../nvidia/nurec"
SCENE_ID="00040136-e651-4abd-991d-0655ccda9430"
DEFAULT_USDZ="$NUREC_DIR/PhysicalAI-Autonomous-Vehicles-NuRec/sample_set/26.04_release/$SCENE_ID/$SCENE_ID.usdz"
NUREC_PYTHON="${NUREC_PYTHON:-$NUREC_DIR/.venv/bin/python}"
NUREC_USDZ="${NUREC_USDZ:-$DEFAULT_USDZ}"

if [[ ! -x "$NUREC_PYTHON" ]]; then
    echo "NuRec environment not found at $NUREC_PYTHON" >&2
    echo "Run ../../nvidia/nurec/install_nurec.sh first." >&2
    exit 1
fi
if [[ ! -f "$NUREC_USDZ" ]]; then
    echo "NuRec scene not found at $NUREC_USDZ" >&2
    echo "Run ../../nvidia/nurec/install_nurec.sh first or set NUREC_USDZ." >&2
    exit 1
fi

exec "$NUREC_PYTHON" "$HERE/alpamayo_control.py" \
    --sensor-backend nurec \
    --nurec-usdz "$NUREC_USDZ" \
    "$@"
