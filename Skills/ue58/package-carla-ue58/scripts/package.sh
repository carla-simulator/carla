#!/usr/bin/env bash
# Cook, package and inspect a CARLA UE 5.8 package.
#
#   bash package.sh scope              what the configured cook scope is
#   bash package.sh build              cmake --build ... --target package[-CONFIG]
#   bash package.sh inspect            check the ARTIFACT for the failures a green build hides
#   bash package.sh list [PATTERN]     grep the pak index for staged paths
#
# Env: CONFIG=shipping|development|debug|debuggame|test   (default shipping)
#      PKG=<path to a package root>                       (default: newest built)
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
set +e

MODE="${1:-}"; shift 2>/dev/null || true
[ -n "${CARLA_UE58_ROOT}" ] || { echo "CARLA_UE58_ROOT is not set — run check_env.sh" >&2; exit 2; }
BUILD_DIR="$(carla_ue58_build_dir)"

pkg_root() {
  if [ -n "${PKG:-}" ]; then echo "${PKG}"; return; fi
  # Newest package directory across presets.
  ls -1dt "${CARLA_UE58_ROOT}"/Build/*/Package/Carla-*-Linux-* 2>/dev/null \
    | grep -v '\.tar\.gz$' | head -1
}

case "${MODE}" in

scope)
  if ! carla_ue58_configured; then
    echo "not configured for preset ${CARLA_PRESET} — run build-carla-ue58's configure first" >&2
    exit 3
  fi
  MAPS="$(carla_ue58_cmake_opt CARLA_MAPS_TO_COOK)"
  echo "preset          : ${CARLA_PRESET}  (${BUILD_DIR})"
  echo "package config  : $(carla_ue58_cmake_opt CARLA_UNREAL_PACKAGE_BUILD_TYPE)"
  echo "ROS 2           : $(carla_ue58_cmake_opt ENABLE_ROS2)"
  echo "no compression  : $(carla_ue58_cmake_opt CARLA_UNREAL_PACKAGE_NO_COMPRESSION)"
  if [ -z "${MAPS}" ]; then
    echo "cook scope      : <empty> — falls back to DefaultGame.ini MapsToCook:"
    echo "                    Town10HD_Opt, OpenDriveMap, TestMaps/EmptyMap, Mine_01,"
    echo "                    Town15/Town15   (bCookAll=False, so NOT every map)"
    echo "  WARNING Town15 is in that default list and CANNOT be cooked (573"
    echo "  WARNING unresolvable MaterialInstanceDynamic imports), so the"
    echo "  WARNING out-of-the-box package build FAILS. Set an explicit scope."
  else
    echo "cook scope      :"
    # The value is '+'-separated because it is passed to UAT's -MapsToCook=.
    echo "${MAPS}" | tr '+' '\n' | sed 's/^/  /'
    case "${MAPS}" in *Town15*)
      echo "  WARNING Town15 is in scope and CANNOT be cooked — remove it." ;;
    esac
    case "${MAPS}" in *';'*)
      echo "  WARNING the value contains ';' — UAT wants '+' separators; this cooks nothing." ;;
    esac
  fi
  ;;

build)
  carla_ue58_configured || { echo "not configured — run build-carla-ue58 configure" >&2; exit 3; }
  CONFIG="${CONFIG:-shipping}"
  case "${CONFIG}" in
    shipping)    TARGET=package ;;
    development) TARGET=package-development ;;
    debug)       TARGET=package-debug ;;
    debuggame)   TARGET=package-debuggame ;;
    test)        TARGET=package-test ;;
    *) echo "CONFIG must be shipping|development|debug|debuggame|test" >&2; exit 2 ;;
  esac

  # A running packaged server holds the pak files open. UAT then fails to replace
  # the .ucas, retries, and can still exit BUILD SUCCESSFUL leaving a fresh
  # .pak/.utoc beside a stale .ucas — and the .utoc indexes the .ucas by byte
  # offset, so the result is internally inconsistent.
  if pgrep -x 'CarlaUnreal-Linu' >/dev/null 2>&1 || pgrep -f 'CarlaUnreal-Linux-Shipping' >/dev/null 2>&1; then
    echo "ERROR a packaged CARLA server is running — it holds the pak files open." >&2
    echo "      Stop it first:  pkill -f CarlaUnreal-Linux-" >&2
    exit 4
  fi

  MAPS="$(carla_ue58_cmake_opt CARLA_MAPS_TO_COOK)"
  case "${MAPS}" in *Town15*)
    echo "ERROR Town15 is in CARLA_MAPS_TO_COOK and cannot be cooked (573 import errors)." >&2
    echo "      Re-configure without it, then package." >&2
    exit 5 ;;
  esac
  [ -z "${MAPS}" ] && echo "[pkg] WARNING empty scope -> DefaultGame.ini MapsToCook, which includes the uncookable Town15"

  AV="$(df -Pm "${CARLA_UE58_ROOT}" | awk 'NR==2{print $4}')"
  [ "${AV:-0}" -lt 60000 ] && echo "[pkg] WARNING only $((AV/1024)) GB free; cook+stage+archive needs ~150 GB"

  echo "[pkg] cmake --build ${BUILD_DIR} --target ${TARGET}"
  echo "[pkg] this is UAT BuildCookRun with -pak -iostore: expect 1-2 h and ~12 GB"
  echo "[pkg] the exit code is NOT sufficient — run 'bash package.sh inspect' afterwards"
  cd "${CARLA_UE58_ROOT}" && cmake --build "${BUILD_DIR}" --target "${TARGET}" "$@"
  RC=$?
  echo "[pkg] target ${TARGET} exited ${RC}"
  echo "[pkg] now: bash package.sh inspect"
  exit ${RC}
  ;;

inspect)
  P="$(pkg_root)"
  if [ -z "${P}" ] || [ ! -d "${P}" ]; then
    echo "no package found under ${CARLA_UE58_ROOT}/Build/*/Package — build one first" >&2
    exit 3
  fi
  rc=0
  ok(){   echo "  PASS $*"; }
  warn(){ echo "  WARN $*"; }
  bad(){  echo "  FAIL $*"; rc=1; }

  echo "== Package =="
  ok "${P}"
  ok "size $(du -sh "${P}" 2>/dev/null | cut -f1)"
  if [ -f "${P}/VERSION" ]; then
    echo "  --- VERSION ---"
    sed 's/^/      /' "${P}/VERSION"
  else
    warn "no VERSION file — cannot tell which commits went in"
  fi

  echo "== Binaries =="
  SH="${P}/Linux/CarlaUnreal.sh"
  [ -f "${SH}" ] && ok "launcher CarlaUnreal.sh" || bad "launcher ${SH} missing"
  BIN="$(ls -1 "${P}"/Linux/CarlaUnreal/Binaries/Linux/CarlaUnreal-Linux-* 2>/dev/null | head -1)"
  if [ -n "${BIN}" ]; then
    ok "server binary $(basename "${BIN}") ($(du -h "${BIN}" | cut -f1))"
  else
    bad "no CarlaUnreal-Linux-* binary under Linux/CarlaUnreal/Binaries/Linux"
  fi

  echo "== Pak container =="
  PAKS="${P}/Linux/CarlaUnreal/Content/Paks"
  if [ -d "${PAKS}" ]; then
    for f in pakchunk0-Linux.pak pakchunk0-Linux.ucas pakchunk0-Linux.utoc global.ucas global.utoc; do
      if [ -f "${PAKS}/${f}" ]; then
        ok "$(printf '%-26s %6s  %s' "${f}" "$(du -h "${PAKS}/${f}"|cut -f1)" "$(date -r "${PAKS}/${f}" '+%Y-%m-%d %H:%M:%S')")"
      else
        bad "${f} missing"
      fi
    done
    # The .utoc indexes the .ucas by byte offset, so a fresh .utoc beside a stale
    # .ucas is internally inconsistent. Packaging over a running server is how
    # that happens. Spread beyond a few minutes means different builds.
    NEWEST=$(find "${PAKS}" -maxdepth 1 -type f -printf '%T@\n' 2>/dev/null | sort -n | tail -1 | cut -d. -f1)
    OLDEST=$(find "${PAKS}" -maxdepth 1 -type f -printf '%T@\n' 2>/dev/null | sort -n | head -1 | cut -d. -f1)
    if [ -n "${NEWEST}" ] && [ -n "${OLDEST}" ]; then
      SPREAD=$(( NEWEST - OLDEST ))
      if [ "${SPREAD}" -le 600 ]; then
        ok "all pak files within ${SPREAD}s — one consistent build"
      else
        bad "pak files span $((SPREAD/60)) min — likely a fresh .pak/.utoc over a STALE .ucas"
        bad "  the .utoc indexes the .ucas by byte offset; re-package with no server running"
      fi
    fi
  else
    bad "${PAKS} does not exist"
  fi

  echo "== Staged road data and profiles =="
  PAK="${PAKS}/pakchunk0-Linux.pak"
  if [ -f "${PAK}" ]; then
    # The pak index stores paths as plain strings, so grepping the container is a
    # dependency-free way to answer "did this actually get staged" without
    # UnrealPak (verified against `UnrealPak -List` on a real package).
    # Measured on a CLEAN build: Carla/Config IS staged recursively, so
    # Config/PostProcess ships; Carla/Maps/{OpenDrive,Nav,TM} ship; and of the
    # large maps only the TM subdir ships -- Town1x/OpenDrive does NOT.
    # NOTE the scope below is the tree's CURRENT configure, not what built this
    # package -- the cook scope is not recorded in the artifact. A mismatch here
    # can therefore mean "you re-configured since building", not a bad package.
    SCOPE="$(carla_ue58_cmake_opt CARLA_MAPS_TO_COOK)"
    for pat in Town12/OpenDrive Town12/TM Town13/OpenDrive Town13/TM Config/PostProcess; do
      # grep -c prints 0 and exits 1 on no match, so an `|| echo 0` fallback
      # would emit a second 0 and break the arithmetic test below.
      N="$(grep -a -c -- "${pat}" "${PAK}" 2>/dev/null)"; N="${N:-0}"
      if [ "${N:-0}" -gt 0 ]; then
        ok "${pat} staged"
      else
        case "${pat}" in
          Config/PostProcess)
            bad "${pat} NOT staged — maps render BLACK (Default.json uses AEM_Manual"
            bad "  exposure where the per-town profiles use AEM_Histogram)" ;;
          *)
            # Town11/12/13 keep road data in per-map subdirectories that
            # DirectoriesToAlwaysStageAsUFS does not list, so it only ships if the
            # archive still holds it from an earlier build. Absent is CORRECT for
            # a clean build that did not cook that town.
            TOWN="${pat%%/*}"
            case "${SCOPE}" in
              *"${TOWN}"*)
                bad "${pat} NOT staged, but ${TOWN} IS in the cook scope"
                bad "  ${TOWN} ships without its road network: DirectoriesToAlwaysStageAsUFS"
                bad "  lists Carla/Maps/{OpenDrive,Nav,TM} and Carla/Maps/Town15/* but no"
                bad "  entry for Carla/Maps/${TOWN}/. Add one to DefaultGame.ini." ;;
              *)
                ok "${pat} absent — expected, ${TOWN} is not in the cook scope" ;;
            esac ;;
        esac
      fi
    done
    N15="$(grep -a -c -- 'Maps/Town15' "${PAK}" 2>/dev/null)"; N15="${N15:-0}"
    [ "${N15:-0}" -gt 0 ] && warn "Town15 strings present in the pak index — it cannot cook, so expect 0 map entries"
  fi

  echo "== Cooked maps =="
  UTOC="${PAKS}/pakchunk0-Linux.utoc"
  MAPSCOPE="$(carla_ue58_cmake_opt CARLA_MAPS_TO_COOK)"
  case "${MAPSCOPE}" in
    *OpenDriveMap*) ok "OpenDriveMap is in the cook scope -> generate_opendrive_world() will work" ;;
    "")             warn "empty cook scope; the default list does include OpenDriveMap" ;;
    *)              bad "OpenDriveMap is NOT in the cook scope"
                    bad "  client.generate_opendrive_world() will fail with a bare std::exception:"
                    bad "  that map is the HOST LEVEL for the procedural road (a generated world"
                    bad "  reports its name as 'Carla/Maps/OpenDriveMap'). Add"
                    bad "  /Game/Carla/Maps/OpenDriveMap to CARLA_MAPS_TO_COOK." ;;
  esac
  echo "  (a full cooked-map list needs the engine: UnrealPak ${UTOC##*/} -List)"

  echo "== Bundled client =="
  W="$(ls -1t "${P}"/PythonAPI/carla/dist/carla-*.whl 2>/dev/null | head -1)"
  [ -n "${W}" ] && ok "wheel $(basename "${W}")" \
    || warn "no wheel under PythonAPI/carla/dist — clients must install one separately"
  [ -d "${P}/PythonAPI/examples" ] && ok "$(ls -1 "${P}"/PythonAPI/examples/*.py 2>/dev/null | wc -l) example scripts bundled"

  echo "== Archive =="
  TGZ="$(ls -1t "${CARLA_UE58_ROOT}"/Build/*/Package/*.tar.gz 2>/dev/null | head -1)"
  [ -n "${TGZ}" ] && ok "$(basename "${TGZ}") ($(du -h "${TGZ}"|cut -f1))" || warn "no .tar.gz archive"

  echo "== Packaged-server limitations (measured) =="
  warn "get_available_maps() returns [] from a package: GetAllMapNames() is a raw"
  warn "  FindFilesRecursive(\"*.umap\") that cannot see inside a .pak."
  warn "load_world() resolves ONLY /Game/Carla/Maps/<Name> (FindMapPath's single pak"
  warn "  fallback), so nested large maps like Town12/Town12 fail and imported maps"
  warn "  under /Game/<Pkg>/Maps/ fail. Small maps load by exact name."
  warn "get_world(), spawning and navigation DO work against a packaged server."

  echo "== Result =="
  [ "$rc" -eq 0 ] && echo "  package looks consistent (warnings are non-blocking)" \
    || echo "  PROBLEMS FOUND — see FAIL lines"
  exit $rc
  ;;

list)
  P="$(pkg_root)"
  PAK="${P}/Linux/CarlaUnreal/Content/Paks/pakchunk0-Linux.pak"
  [ -f "${PAK}" ] || { echo "no pak at ${PAK}" >&2; exit 3; }
  PAT="${1:-Carla/Maps}"
  echo "# strings matching '${PAT}' in $(basename "${PAK}")"
  # Not a real index parse — UnrealPak -List does that, and it needs the engine.
  # This is enough to answer "is X in there".
  grep -a -o -- "[A-Za-z0-9_./-]*${PAT}[A-Za-z0-9_./-]*" "${PAK}" 2>/dev/null | sort -u | head -60
  echo "# (for a real listing: \$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealPak ${PAK} -List)"
  ;;

*)
  echo "usage: bash package.sh {scope|build|inspect|list [PATTERN]}" >&2
  exit 2
  ;;
esac
