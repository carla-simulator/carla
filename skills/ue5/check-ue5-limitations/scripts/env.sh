#!/usr/bin/env bash
# Self-contained environment for the check-ue5-limitations skill (CARLA on UE 5.5,
# branch ue5-dev). Source before the other scripts:  source env.sh
#
#   CARLA_UE5_ROOT   a ue5-dev checkout (UE 5.5). Auto-detected from a few usual
#                    places when unset.
#   CARLA_UE58_ROOT  optional: a ue58-dev checkout to diff against. When both are
#                    present the report is measured rather than recited.
#   CARLA_PRESET     CMake preset whose cache is read (default Release).
#
# This skill reads. It builds nothing and launches nothing.
#
# Sets no shell options: this file is sourced.

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export PYTHON="${PYTHON:-python3}"
export CARLA_PRESET="${CARLA_PRESET:-Release}"
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"

# A UE5-era CARLA checkout: CMake-driven (no Makefile) with the renamed Unreal
# project directory. Distinguishing 5.5 from 5.8 needs content, see below.
carla_ue5_is_root() {
  [ -n "${1:-}" ] && [ -f "$1/CMakePresets.json" ] && [ -d "$1/Unreal/CarlaUnreal" ]
}

if [ -z "${CARLA_UE5_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/UE5/carla" "${HOME}/carla-ue5" "/workspace/carla-ue5"; do
    if carla_ue5_is_root "${_c}"; then export CARLA_UE5_ROOT="${_c}"; break; fi
  done
fi
export CARLA_UE5_ROOT="${CARLA_UE5_ROOT:-}"

carla_ue5_branch() {
  [ -n "${CARLA_UE5_ROOT}" ] || { echo unknown; return; }
  git -C "${CARLA_UE5_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown
}

# Which UE5 line is this really? The branch name is a hint, not proof (people
# rename). These three markers are what actually separate the trees, and they are
# the same markers this skill reports on:
#   - ros2/middleware/            5.8 only (RMW abstraction)
#   - Autoware publishers         5.8 only
#   - MountExternalPackageRoots   5.8 only (OFPA large-map mount)
carla_ue5_flavor() {
  [ -n "${CARLA_UE5_ROOT}" ] || { echo unknown; return; }
  local mw=0 aw=0
  [ -d "${CARLA_UE5_ROOT}/LibCarla/source/carla/ros2/middleware" ] && mw=1
  [ -f "${CARLA_UE5_ROOT}/LibCarla/source/carla/ros2/publishers/AutowareGNSSPublisher.cpp" ] && aw=1
  if [ "${mw}" -eq 1 ] || [ "${aw}" -eq 1 ]; then echo "ue58"; else echo "ue5"; fi
}

# The engine branch a tree expects, straight from its own files rather than from
# an assumption: 5.5 trees name ue5-dev-carla, 5.8 trees name ue58-dev-carla.
carla_ue5_expected_engine() {
  [ -n "${CARLA_UE5_ROOT}" ] || return 1
  grep -rhoE "ue5[0-9]*-dev-carla" "${CARLA_UE5_ROOT}/CMakePresets.json" \
       "${CARLA_UE5_ROOT}"/CMake/*.cmake "${CARLA_UE5_ROOT}"/*.md 2>/dev/null \
    | sort | uniq -c | sort -rn | awk 'NR==1{print $2}'
}

carla_ue5_cmake_opt() {
  local cache="${CARLA_UE5_ROOT}/Build/${CARLA_PRESET}/CMakeCache.txt"
  [ -f "${cache}" ] || return 1
  local line
  line="$(grep -m1 "^${1}:" "${cache}" 2>/dev/null)" || return 1
  printf '%s' "${line#*=}"
}

echo "[env] CARLA_UE5_ROOT  = ${CARLA_UE5_ROOT:-<unset>}  (branch $(carla_ue5_branch), flavor $(carla_ue5_flavor))"
echo "[env] CARLA_UE58_ROOT = ${CARLA_UE58_ROOT:-<unset>}  (optional, enables a measured diff)"
