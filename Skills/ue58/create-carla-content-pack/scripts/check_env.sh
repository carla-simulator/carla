#!/usr/bin/env bash
# Prerequisite checks for create-carla-content-pack. Read-only, no sudo, fast.
# Hard blockers: no ue58-dev checkout, no engine, no editor binary, no base release.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== Checkout and engine =="
if [ -z "${CARLA_UE58_ROOT}" ] || ! carla_ue58_is_root "${CARLA_UE58_ROOT}"; then
  bad "no ue58-dev checkout with Util/ContentPacks/carla_pack.py — export CARLA_UE58_ROOT"
else
  ok "checkout ${CARLA_UE58_ROOT}"
fi
EDITOR_CMD="${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor-Cmd"
if [ -z "${CARLA_UNREAL_ENGINE_PATH}" ]; then
  bad "CARLA_UNREAL_ENGINE_PATH unset and not in ${CARLA_BUILD_DIR}/CMakeCache.txt"
elif [ -x "${EDITOR_CMD}" ]; then
  ok "engine ${CARLA_UNREAL_ENGINE_PATH} (UnrealEditor-Cmd present: the cooker)"
else
  bad "no ${EDITOR_CMD} — build the engine and carla-unreal-editor (build-carla-ue58)"
fi
# The cook loads the project's editor modules.
if ls "${CARLA_UE58_ROOT}"/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux/libUnrealEditor-Carla*.so >/dev/null 2>&1; then
  ok "Carla editor module built"
else
  bad "libUnrealEditor-Carla.so missing — build the carla-unreal-editor target first"
fi
"${PYTHON}" -c 'import sys; sys.exit(sys.version_info < (3, 8))' 2>/dev/null \
  && ok "$("${PYTHON}" -V 2>&1) (carla_pack.py needs 3.8+)" \
  || bad "${PYTHON} is missing or older than 3.8"

echo "== Base release (what the pack is cooked against) =="
BASE="$(carla_pack_default_base)"
if [ -z "${BASE}" ]; then
  bad "no release metadata in ${CARLA_BUILD_DIR}/Package/ — build the package (package-carla-ue58) or export CARLA_PACK_BASE=<the target package's *_release-metadata.tar.gz>"
elif [ -e "${BASE}" ]; then
  ok "base ${BASE}"
  PKG_DIR="$(dirname "${BASE}")"
  STAMP="$(cat "${PKG_DIR}"/Carla-*/BaseRelease 2>/dev/null | head -1)"
  [ -n "${STAMP}" ] && ok "package next to it is stamped '${STAMP}'"
else
  bad "CARLA_PACK_BASE=${BASE} does not exist"
fi
N="$(find "${CARLA_UE58_ROOT}/Build" -name '*-release-metadata.tar.gz' 2>/dev/null | wc -l)"
R="$(ls -1d "${CARLA_UE58_ROOT}"/Unreal/CarlaUnreal/Releases/*/ 2>/dev/null | wc -l)"
if [ $((N + R)) -gt 1 ]; then
  warn "${N} metadata tarball(s) under Build/ and ${R} Releases/ dir(s): bare carla-pack refuses with 'several base releases found'; pack.sh passes --base for you"
fi

echo "== Packs in this checkout =="
if [ -d "${CARLA_PACKS_ROOT}" ]; then
  for d in "${CARLA_PACKS_ROOT}"/*/; do
    [ -f "${d}carla-pack.json" ] || continue
    n="$(basename "${d}")"
    m="$("${PYTHON}" -c 'import json,sys; print(", ".join(x["name"] for x in json.load(open(sys.argv[1])).get("maps", [])) or "-")' "${d}carla-pack.json" 2>/dev/null)"
    ok "${n}: maps ${m}"
  done
else
  ok "no Plugins/Packs yet (init creates it)"
fi
# A pack enabled in the project gets cooked into the CARLA package itself.
if grep -q '"Packs/' "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/CarlaUnreal.uproject" 2>/dev/null; then
  bad "CarlaUnreal.uproject references a pack — remove it; the package target fails its leak check"
fi
if pgrep -x UnrealEditor >/dev/null 2>&1; then
  warn "an UnrealEditor is running — save and close it before building (it holds the pack's assets)"
fi

AVAIL_GB=$(df -BG --output=avail "${CARLA_UE58_ROOT:-.}" 2>/dev/null | tail -1 | tr -dc '0-9')
# A pack needs its staged cook, the unpacked folder and the tarball: ~3x its size.
if [ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -lt 25 ]; then
  warn "only ${AVAIL_GB} GB free — a pack needs ~3x its size (AdditionalMaps: ~7 GB -> ~21 GB)"
else
  ok "disk ${AVAIL_GB:-?} GB free"
fi

echo
[ "${rc}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                  || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${rc}"
