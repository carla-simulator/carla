#!/usr/bin/env bash

set -euo pipefail

ALPAMAYO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODEL_ENV="${ALPAMAYO2_MODEL_ENV:-${ALPAMAYO_ROOT}/.venv-model}"

export CUDA_VISIBLE_DEVICES="${ALPAMAYO2_CUDA_DEVICES:-1,2,3}"
export PYTORCH_CUDA_ALLOC_CONF="${PYTORCH_CUDA_ALLOC_CONF:-expandable_segments:True}"

exec "${MODEL_ENV}/bin/python" "${ALPAMAYO_ROOT}/alpamayo_model_server.py" "$@"
