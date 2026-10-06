#!/usr/bin/env bash
# Self-contained environment for the package-carla-ue4 skill.
# Source this before running the skill's scripts:  source env.sh
#
# carla-agentic-tools is a STANDALONE repo — it does not live inside a CARLA
# checkout — so the target instance is chosen at runtime. Both roots below are
# overridable and must ultimately point at a real, built CARLA + UE4:
#
#   CARLA_UE4_ROOT  the carla source checkout (branch ue4-dev) to package
#   UE4_ROOT        the built CarlaUnreal UE 4.26 fork
#
# No environment-manager assumption is made: the wheel stage uses whatever
# `python3` is active (references/packaging.md P1).

# Sets no shell options on purpose: this file is sourced, and `set -e` here
# would change its callers' error semantics. Each script sets its own.

# --- Optional environment hook ----------------------------------------------
# CARLA_ENV_ACTIVATE, when set, names an activation script to source into this
# (possibly non-interactive) shell — the one hook for driving this skill without
# an already-active environment. Unset, it is a silent no-op. Nothing else is
# detected: no environment-manager binary is probed, no dotfile is searched for.
# Roots the caller exported explicitly outrank anything that script sets.
# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

_KEEP_CARLA_ROOT="${CARLA_UE4_ROOT:-}" _KEEP_UE4_ROOT="${UE4_ROOT:-}"
if [ -n "${CARLA_ENV_ACTIVATE:-}" ] && [ -f "${CARLA_ENV_ACTIVATE}" ]; then
  # shellcheck disable=SC1090
  source "${CARLA_ENV_ACTIVATE}"
fi
[ -n "${_KEEP_CARLA_ROOT}" ] && CARLA_UE4_ROOT="${_KEEP_CARLA_ROOT}"
[ -n "${_KEEP_UE4_ROOT}" ] && UE4_ROOT="${_KEEP_UE4_ROOT}"

# --- Resolve the target CARLA checkout --------------------------------------
# Precedence: explicit CARLA_UE4_ROOT  >  $PWD if it is a checkout  >  the
# path-derived guess (only meaningful when this repo was dropped INTO a checkout).
_SKILL_SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_DERIVED_ROOT="$(cd "${_SKILL_SCRIPTS_DIR}/../../../../.." && pwd)"
_UPROJECT_REL="Unreal/CarlaUE4/CarlaUE4.uproject"

if [ -z "${CARLA_UE4_ROOT:-}" ]; then
  if [ -f "${PWD}/${_UPROJECT_REL}" ]; then
    CARLA_UE4_ROOT="${PWD}"
  elif [ -f "${_DERIVED_ROOT}/${_UPROJECT_REL}" ]; then
    CARLA_UE4_ROOT="${_DERIVED_ROOT}"
  fi
fi
export CARLA_UE4_ROOT="${CARLA_UE4_ROOT:-}"

# UE4_ROOT has no derivable default from a standalone repo — export it, or let
# check_env.sh fail loudly with the path it looked for.
export UE4_ROOT="${UE4_ROOT:-}"

# --- Client Python ----------------------------------------------------------
# The wheel stage runs `python3 -m build`, so `python3` must import `build`.
# Verified up front, before the expensive editor compile.
# Sets CARLA_PY_BIN and CARLA_PY_ARG:
#   - no pin  -> plain `python3` (the active env); CARLA_PY_ARG stays empty so
#     `make package` is NOT told a version and the active env wins.
#   - pinned  -> `python<pin>`; CARLA_PY_ARG="--python-version=<pin>" is
#     forwarded so a version-suffixed interpreter is used deliberately.
# Non-zero with actionable, manager-agnostic guidance on failure.
carla_require_wheel_python() {
  local pin="${1:-}"
  local py
  if [ -n "${pin}" ]; then
    py="python${pin}"
    CARLA_PY_ARG="--python-version=${pin}"
  else
    py="python3"
    CARLA_PY_ARG=""
  fi
  CARLA_PY_BIN="$(command -v "${py}" 2>/dev/null || true)"
  if [ -z "${CARLA_PY_BIN}" ]; then
    echo "[env] no '${py}' on PATH." >&2
    echo "[env] Activate the environment holding your CARLA client python" >&2
    echo "[env] in this shell, or set CARLA_ENV_ACTIVATE to its activate" >&2
    echo "[env] script. Do NOT set CARLA_PY_VERSION unless you truly need a" >&2
    echo "[env] version-suffixed interpreter (it must resolve INSIDE that env)." >&2
    return 1
  fi
  if ! "${CARLA_PY_BIN}" -c 'import build' 2>/dev/null; then
    echo "[env] ${CARLA_PY_BIN} cannot 'import build' (needed by the wheel stage)." >&2
    echo "[env] Install it there, e.g.:  ${CARLA_PY_BIN} -m pip install build" >&2
    return 1
  fi
  # Soft signal only: confirm this is likely the client env. A miss is fine —
  # the wheel gets built and installed into this interpreter regardless.
  if "${CARLA_PY_BIN}" -c 'import carla' 2>/dev/null; then
    echo "[env] python: ${CARLA_PY_BIN} (has carla + build)"
  else
    echo "[env] python: ${CARLA_PY_BIN} (has build; carla not importable here — the wheel installs into it)"
  fi
  export CARLA_PY_BIN CARLA_PY_ARG
  return 0
}

# --- ROS 2 native interface (opt-in) ----------------------------------------
# ROS2=1 cooks a package WITH CARLA's native ROS 2 interface. `Package.sh` has
# no ROS 2 option of its own: support is inherited from the editor build, and
# `make package` DEPENDS on CarlaUE4Editor, so it re-runs BuildCarlaUE4.sh and
# rewrites Config/OptionalModules.ini. Without --ros2 in ARGS that rewrite says
# `Ros2 OFF` and the cooked package silently has no ROS 2 (references/ros2.md).
export ROS2="${ROS2:-0}"
CARLA_ROS2_ARG=""
[ "${ROS2}" = "1" ] && CARLA_ROS2_ARG="--ros2"
export CARLA_ROS2_ARG

# Build-time ROS 2 state of the checkout. BuildCarlaUE4.sh writes every module
# flag onto ONE space-separated line, so match the token pair anywhere.
# Echoes: on | off | absent.
carla_ros2_ini_state() {
  local ini="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Config/OptionalModules.ini"
  [ -f "${ini}" ] || { echo absent; return 0; }
  grep -q 'Ros2 ON' "${ini}" && echo on || echo off
}

unset _KEEP_CARLA_ROOT _KEEP_UE4_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] UE4_ROOT        = ${UE4_ROOT:-<unset — export it>}"
echo "[env] ROS2            = ${ROS2} (checkout currently built with Ros2 $(carla_ros2_ini_state))"
