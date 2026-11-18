#!/usr/bin/env bash
# Self-contained environment for the install-python-api skill.
# Source before the other scripts:  source env.sh
#
# This skill installs the CARLA *client* (the `carla` python package) into an
# interpreter. It never touches the simulator.
#
#   PYTHON               interpreter to install INTO. Default: python3 on PATH.
#                        Same variable every other python-api skill uses, so one
#                        setting serves the whole group. Set it when the agent's
#                        own python is not the one that should carry the client —
#                        the usual case under uvx/pipx, where the MCP server runs
#                        in an isolated env whose python is first on PATH.
#   CARLA_PACKAGE_ROOT   extracted CARLA release (the dir holding CarlaUE4.sh,
#                        CarlaUnreal.sh or CarlaUnreal.exe). Its bundled wheel/egg
#                        is the preferred source, because it matches the
#                        simulator exactly.
#   CARLA_TARGET, CARLA_ROOT, CARLA_UE58_ROOT, CARLA_UE5_ROOT, CARLA_UE4_ROOT
#                        searched too, in that order: PythonAPI/carla/dist, and
#                        Build/<preset>/PythonAPI/dist on a UE5 checkout.
#   CARLA_HOST/PORT      a running server, used only to read its version for the
#                        match check (default 127.0.0.1:2000).
#
# Sets no shell options: this file is sourced.

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See Skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

_KEEP_PY="${PYTHON:-}"
if [ -n "${CARLA_ENV_ACTIVATE:-}" ] && [ -f "${CARLA_ENV_ACTIVATE}" ]; then
  # shellcheck disable=SC1090
  source "${CARLA_ENV_ACTIVATE}"
fi
[ -n "${_KEEP_PY}" ] && PYTHON="${_KEEP_PY}"

# The interpreter to install into. `python3` from PATH is the right default for a
# human in a shell; it is the WRONG default when this runs as a child of an
# isolated MCP server (uvx/pipx put their own python first), which is exactly why
# the PYTHON variable exists.
export PYTHON="${PYTHON:-python3}"
# Absolute path of that interpreter, for reporting and for pip's target.
PYTHON_BIN="$(command -v "${PYTHON}" 2>/dev/null || true)"
export PYTHON_BIN

export CARLA_PACKAGE_ROOT="${CARLA_PACKAGE_ROOT:-}"
export CARLA_TARGET="${CARLA_TARGET:-}"
export CARLA_ROOT="${CARLA_ROOT:-}"
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"
export CARLA_UE5_ROOT="${CARLA_UE5_ROOT:-}"
export CARLA_UE4_ROOT="${CARLA_UE4_ROOT:-}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"

# Python X.Y and the wheel tag that must match a candidate wheel (cp310, cp311…).
if [ -n "${PYTHON_BIN}" ]; then
  CARLA_PY_XY="$("${PYTHON_BIN}" -c 'import sys;print("%d.%d"%sys.version_info[:2])' 2>/dev/null || true)"
  CARLA_PY_TAG="cp${CARLA_PY_XY//./}"
else
  CARLA_PY_XY=""; CARLA_PY_TAG=""
fi
export CARLA_PY_XY CARLA_PY_TAG

unset _KEEP_PY

echo "[env] PYTHON             = ${PYTHON_BIN:-<not found: ${PYTHON}>} ${CARLA_PY_XY:+(${CARLA_PY_XY}, ${CARLA_PY_TAG})}"
echo "[env] CARLA_PACKAGE_ROOT = ${CARLA_PACKAGE_ROOT:-<unset>}"
for _v in CARLA_TARGET CARLA_ROOT CARLA_UE58_ROOT CARLA_UE5_ROOT CARLA_UE4_ROOT; do
  [ -n "${!_v}" ] && printf '[env] %-18s = %s\n' "${_v}" "${!_v}"
done
unset _v
