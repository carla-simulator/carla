#!/usr/bin/env bash
# Environment for the build-carla-ue58-docker skill; source it first. Resolves CARLA_UE58_ROOT and CARLA_UNREAL_ENGINE_PATH,
# and sets CARLA_PRESET (Release), CARLA_HOST:CARLA_PORT (127.0.0.1:2000), PYTHON (python3); DLSS_SDK is optional.

set -euo pipefail

# Paths the install skills recorded fill unset keys; an exported variable wins (Skills/_common/env_common.sh).
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_PRESET="${CARLA_PRESET:-Release}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export PYTHON="${PYTHON:-python3}"

# --- helpers (defined before the resolution below uses them) ------------------

carla_ue58_is_root() {
  # Structural markers (CMake build + CarlaUnreal project), so a tarball or detached HEAD resolves.
  [ -f "${1:-}/CMakePresets.json" ] && [ -d "${1:-}/Unreal/CarlaUnreal" ]
}

carla_ue58_build_dir() { echo "${CARLA_UE58_ROOT}/Build/${CARLA_PRESET}"; }

carla_ue58_configured() { [ -f "$(carla_ue58_build_dir)/CMakeCache.txt" ]; }

# What the tree was actually configured with: every -D must be repeated on re-configure,
# so a forgotten -DENABLE_ROS2=ON silently gives a non-ROS build.
carla_ue58_cmake_opt() {
  local cache="${CARLA_UE58_ROOT:-}/Build/${CARLA_PRESET:-Release}/CMakeCache.txt"
  [ -f "${cache}" ] || return 1
  sed -n "s|^${1}:[A-Z]*=||p" "${cache}" | head -1
}

# Engine major.minor from the engine itself: what tells this group (5.8) from ue5 (5.5).
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

# ue58-dev vs ue5-dev from content, so a detached HEAD or tarball works:
# Carla/Autoware and CMake/DLSS.cmake exist only on ue58-dev.
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
  ls -1dt "${CARLA_UE58_ROOT:-/nonexistent}"/Build/*/Package/Carla-*/CarlaUnreal.sh 2>/dev/null | head -1
}

# --- resolution --------------------------------------------------------------

if [ -z "${CARLA_UE58_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/UE58/carla" "${HOME}/carla-ue58" "/workspace/carla"; do
    if carla_ue58_is_root "${_c}"; then CARLA_UE58_ROOT="${_c}"; break; fi
  done
fi
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"

# Prefer the engine path a configured tree recorded in its CMake cache, then the documented layout.
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
