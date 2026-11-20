#!/usr/bin/env bash
# Builds content packs with Util/ContentPacks/carla_pack.py, always against an explicit base.
# Usage: pack.sh bases | towns <Pack> <Map>... | build <Pack> | dry-run <Pack> | check <Pack|tar.gz> | tool <args>
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

CMD="${1:-}"; shift 2>/dev/null || true
die() { echo "[pack] ERROR $*" >&2; exit 1; }
tool() { "${PYTHON}" "${CARLA_PACK_TOOL}" "$@"; }

base_or_die() {
  BASE="$(carla_pack_default_base)"
  [ -n "${BASE}" ] && [ -e "${BASE}" ] || die "no base release metadata: build the package first, or export CARLA_PACK_BASE=<target package's release metadata>"
  echo "[pack] base ${BASE}"
}

engine_or_die() {
  [ -x "${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor-Cmd" ] || die "no UnrealEditor-Cmd under CARLA_UNREAL_ENGINE_PATH='${CARLA_UNREAL_ENGINE_PATH}'"
}

# Newest tarball of a pack, in --out or the default out dir.
newest_tarball() {
  ls -1t "${OUT:-${CARLA_PACKS_ROOT}/$1/Saved/CarlaPack/out}/$1"-*.tar.gz 2>/dev/null | head -1
}

case "${CMD}" in
  bases)
    echo "[pack] base release candidates (default first):"
    echo "  * $(carla_pack_default_base)"
    find "${CARLA_UE58_ROOT}/Build" -name '*-release-metadata.tar.gz' -printf '    %TY-%Tm-%Td %TH:%TM  %p\n' 2>/dev/null | sort -r
    ls -1d "${CARLA_UE58_ROOT}"/Unreal/CarlaUnreal/Releases/*/ 2>/dev/null | sed 's/^/    extracted: /'
    for s in "${CARLA_BUILD_DIR}"/Package/Carla-*/BaseRelease; do
      [ -f "${s}" ] && echo "  package $(dirname "${s}") is stamped '$(cat "${s}")'"
    done
    ;;
  towns)
    PACK="${1:?towns needs <Pack> <Map>...}"; shift
    engine_or_die; base_or_die
    # The deployed package is Shipping; cooked data is the same for every config.
    START=$(date +%s)
    tool create "${PACK}" "$@" --base "${BASE}" --config "${PACK_CONFIG:-Shipping}" ${OUT:+--out "${OUT}"} || die "carla-pack create failed"
    echo "[pack] create took $(( $(date +%s) - START )) s"
    exec "${HERE}/pack.sh" check "${PACK}"
    ;;
  build)
    PACK="${1:?build needs <Pack>}"; shift
    engine_or_die; base_or_die
    START=$(date +%s)
    tool build "${PACK}" --base "${BASE}" --config "${PACK_CONFIG:-Shipping}" ${OUT:+--out "${OUT}"} "$@" || die "carla-pack build failed"
    echo "[pack] build took $(( $(date +%s) - START )) s"
    exec "${HERE}/pack.sh" check "${PACK}"
    ;;
  dry-run)
    PACK="${1:?dry-run needs <Pack>}"; shift
    base_or_die
    tool build "${PACK}" --base "${BASE}" --config "${PACK_CONFIG:-Shipping}" --dry-run "$@"
    ;;
  check)
    TARGET="${1:?check needs <Pack> or a tarball}"; shift
    if [ ! -f "${TARGET}" ]; then
      T="$(newest_tarball "${TARGET}")"
      [ -n "${T}" ] || die "no tarball for pack '${TARGET}' — build it first"
      TARGET="${T}"
    fi
    exec "${PYTHON}" "${HERE}/check_pack.py" "${TARGET}" --tool "${CARLA_PACK_TOOL}" --packs-root "${CARLA_PACKS_ROOT}" "$@"
    ;;
  tool)
    tool "$@"
    ;;
  *)
    sed -n '2,3p' "$0"; exit 2 ;;
esac
