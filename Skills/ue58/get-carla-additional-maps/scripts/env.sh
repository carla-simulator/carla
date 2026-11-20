#!/usr/bin/env bash
# Environment for get-carla-additional-maps: the package to install into, and the checkout that builds the pack.
# Source before the skill's other scripts: source scripts/env.sh
set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an export wins.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

# The package and carla_pack.py resolve exactly as in install-carla-content-pack.
. "$(dirname "${BASH_SOURCE[0]}")/../../install-carla-content-pack/scripts/env.sh" >/dev/null

if [ -z "${CARLA_UE58_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/UE58/carla"; do
    if [ -f "${_c}/Util/ContentPacks/AdditionalMaps.json" ]; then CARLA_UE58_ROOT="${_c}"; break; fi
  done
fi
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"

echo "[env] CARLA_PACKAGE_ROOT = ${CARLA_PACKAGE_ROOT:-<unset>}"
echo "[env] CARLA_UE58_ROOT    = ${CARLA_UE58_ROOT:-<unset: only needed to build the pack>}"
echo "[env] CARLA_PRESET       = ${CARLA_PRESET}"
