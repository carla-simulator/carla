#!/usr/bin/env bash
# Self-contained environment for the Scenic skills. Source before the other
# scripts:  source scripts/env.sh
#
# Scenic is a pip package with its own `scenic` CLI; nothing is built. What a run
# needs is that CLI, an importable `carla` client matching the server, the world
# model the scenario names, and a running simulator.
#
# A scenario's `model` line decides what else must be importable:
#   model scenic.simulators.carla.model   -> Scenic's own model, ships in the wheel
#   model srunner.scenic.models.model     -> ScenarioRunner's fork, needs SCENARIO_RUNNER_ROOT
#
#   SCENIC_ROOT           Scenic checkout (has src/scenic) or the installed package dir
#   SCENIC_EXAMPLES       directory holding .scenic examples, when a checkout exists
#   SCENARIO_RUNNER_ROOT  scenario_runner checkout — source of srunner.scenic
#   CARLA_ROOT            CARLA release or checkout root — source of the content JSONs
#   CARLA_HOST            simulator address                  (default 127.0.0.1)
#   CARLA_PORT            simulator RPC port                 (default 2000)
#   PYTHON                interpreter that imports scenic and carla (default python3)
#   SCENIC_BIN            the scenic CLI, derived from PYTHON

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

carla_scenic_is_checkout() { [ -d "${1:-}/src/scenic" ] || [ -d "${1:-}/examples/carla" ]; }
carla_sr_is_root()         { [ -f "${1:-}/scenario_runner.py" ] && [ -d "${1:-}/srunner" ]; }

export PYTHON="${PYTHON:-python3}"

# The CLI must come from the same interpreter that will import carla, or a run
# picks up a Scenic from a different environment and fails on the model import.
if [ -z "${SCENIC_BIN:-}" ]; then
  _pybin="$(dirname "$(command -v "${PYTHON}" 2>/dev/null || echo /usr/bin/python3)")"
  if [ -x "${_pybin}/scenic" ]; then SCENIC_BIN="${_pybin}/scenic"; else SCENIC_BIN="$(command -v scenic || true)"; fi
fi
export SCENIC_BIN="${SCENIC_BIN:-}"

# A checkout is preferred over the installed package because only a checkout
# carries examples/ and assets/maps/; the wheel ships models only.
if [ -z "${SCENIC_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/Scenic" "${HOME}/scenic" "/workspace/Scenic"; do
    if carla_scenic_is_checkout "${_c}"; then SCENIC_ROOT="${_c}"; break; fi
  done
fi
# Fall back to wherever the package actually imported from, so the group is
# available on a pip-only install rather than looking unconfigured.
if [ -z "${SCENIC_ROOT:-}" ] && [ -n "${SCENIC_BIN}" ]; then
  SCENIC_ROOT="$("${PYTHON}" -c 'import os,scenic;print(os.path.dirname(scenic.__file__))' 2>/dev/null || true)"
fi
export SCENIC_ROOT="${SCENIC_ROOT:-}"

export SCENIC_EXAMPLES="${SCENIC_EXAMPLES:-}"
if [ -z "${SCENIC_EXAMPLES}" ] && [ -d "${SCENIC_ROOT}/examples" ]; then
  export SCENIC_EXAMPLES="${SCENIC_ROOT}/examples"
fi

if [ -z "${SCENARIO_RUNNER_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/scenario_runner" "/workspace/scenario_runner"; do
    if carla_sr_is_root "${_c}"; then SCENARIO_RUNNER_ROOT="${_c}"; break; fi
  done
fi
export SCENARIO_RUNNER_ROOT="${SCENARIO_RUNNER_ROOT:-}"

if [ -z "${CARLA_ROOT:-}" ]; then
  for _c in "${CARLA_TARGET:-}" "${CARLA_UE5_ROOT:-}" "${CARLA_PACKAGE_ROOT:-}" "${CARLA_UE4_ROOT:-}"; do
    if [ -n "${_c}" ] && [ -d "${_c}/PythonAPI/carla" ]; then CARLA_ROOT="${_c}"; break; fi
  done
fi
export CARLA_ROOT="${CARLA_ROOT:-}"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"

# srunner must be importable for the ScenarioRunner-flavoured scenarios, and the
# checkout has to precede site-packages so a stale pip copy cannot shadow it.
_pp=""
[ -n "${SCENARIO_RUNNER_ROOT}" ] && _pp="${SCENARIO_RUNNER_ROOT}"
export PYTHONPATH="${_pp}${PYTHONPATH:+:${PYTHONPATH}}"
export PYTHONIOENCODING="${PYTHONIOENCODING:-utf-8}"

# Scenic keys its blueprint tables off the *client* version, so this is what
# decides which vehicle/prop ids a scenario may reference. Never guess it.
carla_scenic_client_version() {
  "${PYTHON}" -c 'from importlib.metadata import version; print(version("carla"))' 2>/dev/null \
    || "${PYTHON}" -c 'import carla; print(getattr(carla,"__version__","unknown"))' 2>/dev/null \
    || echo unknown
}

carla_scenic_version() {
  [ -n "${SCENIC_BIN}" ] || { echo "unknown"; return; }
  "${PYTHON}" -c 'from importlib.metadata import version; print(version("scenic"))' 2>/dev/null || echo unknown
}

echo "[env] SCENIC_BIN            = ${SCENIC_BIN:-<unset>}  (scenic $(carla_scenic_version))"
echo "[env] SCENIC_ROOT           = ${SCENIC_ROOT:-<unset>}"
echo "[env] SCENIC_EXAMPLES       = ${SCENIC_EXAMPLES:-<none: wheel ships models only>}"
echo "[env] SCENARIO_RUNNER_ROOT  = ${SCENARIO_RUNNER_ROOT:-<unset>}"
echo "[env] CARLA_ROOT            = ${CARLA_ROOT:-<unset>}  (client $(carla_scenic_client_version))"
echo "[env] CARLA_HOST:PORT       = ${CARLA_HOST}:${CARLA_PORT}"
