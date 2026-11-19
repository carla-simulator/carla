#!/usr/bin/env bash
# Environment for install-carla-content-pack: the CARLA package to install into and the carla_pack.py to use.
# Source before the skill's other scripts: source scripts/env.sh
set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an export wins.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export PYTHON="${PYTHON:-python3}"
export CARLA_PRESET="${CARLA_PRESET:-Release}"

carla_is_package() { [ -f "${1:-}/CarlaUnreal.sh" ] || [ -f "${1:-}/CarlaUnreal.exe" ]; }

# CARLA_PACKAGE_ROOT, CARLA_TARGET, $PWD, then the newest package a ue58-dev checkout built.
if [ -z "${CARLA_PACKAGE_ROOT:-}" ] || ! carla_is_package "${CARLA_PACKAGE_ROOT}"; then
  for _c in "${CARLA_TARGET:-}" "${PWD}"; do
    if carla_is_package "${_c}"; then CARLA_PACKAGE_ROOT="${_c}"; break; fi
  done
fi
if [ -z "${CARLA_PACKAGE_ROOT:-}" ] || ! carla_is_package "${CARLA_PACKAGE_ROOT}"; then
  for _r in "${CARLA_UE58_ROOT:-}" "${PWD}" "${HOME}/UE58/carla"; do
    [ -n "${_r}" ] || continue
    _p="$(ls -1dt "${_r}"/Build/"${CARLA_PRESET}"/Package/Carla-*/ 2>/dev/null | head -1)"
    if [ -n "${_p}" ] && carla_is_package "${_p%/}"; then CARLA_PACKAGE_ROOT="${_p%/}"; break; fi
  done
fi
export CARLA_PACKAGE_ROOT="${CARLA_PACKAGE_ROOT:-}"

# The package's own copy matches its release; a checkout's copy is the fallback.
CARLA_PACK_TOOL=""
for _t in "${CARLA_PACKAGE_ROOT}/Tools/carla_pack.py" "${CARLA_UE58_ROOT:-/nonexistent}/Util/ContentPacks/carla_pack.py"; do
  if [ -f "${_t}" ]; then CARLA_PACK_TOOL="${_t}"; break; fi
done
export CARLA_PACK_TOOL

echo "[env] CARLA_PACKAGE_ROOT = ${CARLA_PACKAGE_ROOT:-<unset>}"
echo "[env] CARLA_PACK_TOOL    = ${CARLA_PACK_TOOL:-<not found>}"
