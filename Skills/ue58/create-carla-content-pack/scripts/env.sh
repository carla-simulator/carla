#!/usr/bin/env bash
# Environment for create-carla-content-pack; variables are listed in SKILL.md.
# Source before the skill's other scripts: source scripts/env.sh
set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an export wins.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_PRESET="${CARLA_PRESET:-Release}"
export PYTHON="${PYTHON:-python3}"

carla_ue58_is_root() {
  [ -f "${1:-}/CMakePresets.json" ] && [ -f "${1:-}/Util/ContentPacks/carla_pack.py" ]
}

if [ -z "${CARLA_UE58_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/UE58/carla" "${HOME}/carla-ue58" "/workspace/carla"; do
    if carla_ue58_is_root "${_c}"; then CARLA_UE58_ROOT="${_c}"; break; fi
  done
fi
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"
export CARLA_BUILD_DIR="${CARLA_UE58_ROOT}/Build/${CARLA_PRESET}"

if [ -z "${CARLA_UNREAL_ENGINE_PATH:-}" ] && [ -f "${CARLA_BUILD_DIR}/CMakeCache.txt" ]; then
  CARLA_UNREAL_ENGINE_PATH="$(sed -n 's|^CARLA_UNREAL_ENGINE_PATH:[A-Z]*=||p' "${CARLA_BUILD_DIR}/CMakeCache.txt" | head -1)"
fi
export CARLA_UNREAL_ENGINE_PATH="${CARLA_UNREAL_ENGINE_PATH:-}"
export CARLA_PACK_TOOL="${CARLA_UE58_ROOT}/Util/ContentPacks/carla_pack.py"
export CARLA_PACKS_ROOT="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Plugins/Packs"

# CARLA_PACK_BASE, else the newest metadata the preset's package target wrote;
# carla-pack's own default refuses once a checkout holds more than one.
carla_pack_default_base() {
  if [ -n "${CARLA_PACK_BASE:-}" ]; then printf '%s' "${CARLA_PACK_BASE}"; return; fi
  ls -1t "${CARLA_BUILD_DIR}"/Package/*-release-metadata.tar.gz 2>/dev/null | head -1
}

echo "[env] CARLA_UE58_ROOT          = ${CARLA_UE58_ROOT:-<unset>}"
echo "[env] CARLA_UNREAL_ENGINE_PATH = ${CARLA_UNREAL_ENGINE_PATH:-<unset>}"
echo "[env] CARLA_PRESET             = ${CARLA_PRESET}  -> ${CARLA_BUILD_DIR}"
echo "[env] base release metadata    = $(carla_pack_default_base || true)"
