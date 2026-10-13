#!/usr/bin/env bash
# Step 02 — prepare the CARLA Python client env. Manager-agnostic: this does NOT
# create an env. Activate one first (see SKILL.md), then this installs the client
# build deps into it.
#
# Why an env with a compatible interpreter matters: system python on recent
# distros may be too new for CARLA's boost.python bindings (3.10-3.12 build;
# 3.13/anaconda-base are too new, L5) and may be externally-managed (PEP 668).
# Bring any env whose
# `python3` (or `python<pin>` if you set CARLA_PY_VERSION) is compatible; this
# step installs `requirements.txt` + a pinned numpy<2 (L6) into THAT interpreter.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
carla_require_build_python || exit 1   # sets CARLA_PY_BIN + CARLA_PY_VERSION

REQ="${CARLA_UE4_ROOT}/PythonAPI/carla/requirements.txt"
[ -f "${REQ}" ] || { echo "[client-env] ERROR: ${REQ} not found — is CARLA_UE4_ROOT a carla checkout?"; exit 1; }

# numpy<2: CARLA's bindings are compiled against the numpy 1.x C-API and crash on
# import under 2.x (L6). Installed into the ACTIVE interpreter, whatever provided
# it — no environment-manager command is invoked.
echo "[client-env] installing client build deps + numpy<2 into ${CARLA_PY_BIN} (${CARLA_PY_VERSION})..."
"${CARLA_PY_BIN}" -m pip install --upgrade -r "${REQ}" "numpy<2.0.0"

echo "[client-env] interpreter:"
"${CARLA_PY_BIN}" --version
echo "[client-env] DONE — env ready for step 04 (make PythonAPI)."
