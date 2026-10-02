#!/usr/bin/env bash

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
CONTROL_PYTHON="${ALPAMAYO_CONTROL_PYTHON:-python3}"

exec "$CONTROL_PYTHON" "$HERE/alpamayo_control.py" "$@"
