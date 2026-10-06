#!/usr/bin/env bash
# Self-contained environment for the run-carla-ue58-server skill (CARLA on UE 5.8).
# Source before the skill's other scripts:  source scripts/env.sh
#
# UE 5.8 CARLA is built with CMake, not the UE4 Makefile: there is no `Makefile`
# and no `Util/BuildTools/` in this tree. Everything goes through
#   cmake --preset <PRESET>                        (configure; once per option change)
#   cmake --build Build/<PRESET> --target <TARGET>
#
#   CARLA_UE58_ROOT             the carla checkout on branch ue58-dev
#   CARLA_UNREAL_ENGINE_PATH    the CarlaUnreal/UnrealEngine fork, branch ue58-dev-carla
#   CARLA_PRESET                CMake preset: Release (default) | Development | Debug
#   CARLA_HOST / CARLA_PORT     where a running simulator listens (127.0.0.1:2000)
#   PYTHON                      interpreter that imports carla   (default python3)
#   DLSS_SDK                    optional NVIDIA DLSS SDK checkout

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_PRESET="${CARLA_PRESET:-Release}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export PYTHON="${PYTHON:-python3}"

# --- helpers (defined before the resolution below uses them) ------------------

carla_ue58_is_root() {
  # Structural marker set, so a tarball or a detached HEAD still resolves:
  # CMake-driven build + the CarlaUnreal project directory.
  [ -f "${1:-}/CMakePresets.json" ] && [ -d "${1:-}/Unreal/CarlaUnreal" ]
}

carla_ue58_build_dir() { echo "${CARLA_UE58_ROOT}/Build/${CARLA_PRESET}"; }

carla_ue58_configured() { [ -f "$(carla_ue58_build_dir)/CMakeCache.txt" ]; }

# What the tree was ACTUALLY configured with. This matters more here than on UE4:
# every -D option must be repeated on each re-configure, so a forgotten
# -DENABLE_ROS2=ON silently yields a non-ROS build with no other symptom.
carla_ue58_cmake_opt() {
  local cache="${CARLA_UE58_ROOT:-}/Build/${CARLA_PRESET:-Release}/CMakeCache.txt"
  [ -f "${cache}" ] || return 1
  sed -n "s|^${1}:[A-Z]*=||p" "${cache}" | head -1
}

# Engine major.minor read from the engine itself — the authoritative
# discriminator between this group and the ue5 group (5.8 vs 5.5).
carla_engine_version() {
  local v="${CARLA_UNREAL_ENGINE_PATH:-}/Engine/Build/Build.version"
  [ -f "${v}" ] || { echo "unknown"; return; }
  sed -n 's/.*"MajorVersion" *: *\([0-9]*\).*/\1/p;' "${v}" | head -1 | tr -d '\n'
  echo -n "."
  sed -n 's/.*"MinorVersion" *: *\([0-9]*\).*/\1/p;' "${v}" | head -1
}

carla_ue58_branch() {
  [ -n "${CARLA_UE58_ROOT:-}" ] || { echo "unknown"; return; }
  git -C "${CARLA_UE58_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown"
}

# ue58-dev vs ue5-dev, read from content rather than the branch name so it works
# on a detached HEAD or a tarball. Carla/Autoware and CMake/DLSS.cmake are
# ue58-dev additions; neither exists on ue5-dev.
carla_ue58_flavor() {
  [ -n "${CARLA_UE58_ROOT:-}" ] || { echo "unknown"; return; }
  if [ -d "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware" ] \
     && [ -f "${CARLA_UE58_ROOT}/CMake/DLSS.cmake" ]; then
    echo "ue58"
  elif [ -d "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal" ]; then
    echo "ue5"
  else
    echo "unknown"
  fi
}

# Newest packaged server launcher produced by the `package` target.
carla_ue58_package_sh() {
  ls -1dt "${CARLA_UE58_ROOT:-/nonexistent}"/Build/*/Package/Carla-*/Linux/CarlaUnreal.sh 2>/dev/null | head -1
}

# --- resolution --------------------------------------------------------------

if [ -z "${CARLA_UE58_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/UE58/carla" "${HOME}/carla-ue58" "/workspace/carla"; do
    if carla_ue58_is_root "${_c}"; then CARLA_UE58_ROOT="${_c}"; break; fi
  done
fi
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"

# A configured tree already records the engine path in its CMake cache; prefer
# that over guessing, then fall back to the layout the build docs use.
if [ -z "${CARLA_UNREAL_ENGINE_PATH:-}" ] && [ -n "${CARLA_UE58_ROOT}" ]; then
  _cached="$(carla_ue58_cmake_opt CARLA_UNREAL_ENGINE_PATH 2>/dev/null || true)"
  if [ -n "${_cached:-}" ] && [ -d "${_cached}" ]; then
    CARLA_UNREAL_ENGINE_PATH="${_cached}"
  else
    for _c in "${HOME}/UE58/UnrealEngine5_carla" \
              "$(dirname "${CARLA_UE58_ROOT}")/UnrealEngine5_carla"; do
      if [ -f "${_c}/Engine/Build/Build.version" ]; then CARLA_UNREAL_ENGINE_PATH="${_c}"; break; fi
    done
  fi
fi
export CARLA_UNREAL_ENGINE_PATH="${CARLA_UNREAL_ENGINE_PATH:-}"

echo "[env] CARLA_UE58_ROOT          = ${CARLA_UE58_ROOT:-<unset>}  (branch $(carla_ue58_branch), flavor $(carla_ue58_flavor))"
echo "[env] CARLA_UNREAL_ENGINE_PATH = ${CARLA_UNREAL_ENGINE_PATH:-<unset>}  (engine $(carla_engine_version))"
echo "[env] CARLA_PRESET             = ${CARLA_PRESET}  -> $(carla_ue58_build_dir)"
echo "[env] CARLA_HOST:PORT          = ${CARLA_HOST}:${CARLA_PORT}"
echo "[env] PYTHON                   = ${PYTHON}"
