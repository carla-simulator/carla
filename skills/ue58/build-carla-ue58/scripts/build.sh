#!/usr/bin/env bash
# Configure, build and verify CARLA on UE 5.8 through its CMake build system.
#
#   bash build.sh targets              list every target this tree defines
#   bash build.sh options              list every -D option with its default
#   bash build.sh cache                what the tree was ACTUALLY configured with
#   bash build.sh configure            cmake --preset $CARLA_PRESET [+ knobs below]
#   bash build.sh libcarla             carla-server + carla-client
#   bash build.sh pythonapi            carla-python-api-install (builds AND installs)
#   bash build.sh unreal               carla-unreal        (game/server target)
#   bash build.sh editor               carla-unreal-editor
#   bash build.sh launch | launch-only open the editor (with / without building)
#   bash build.sh target <NAME>        any target by name
#   bash build.sh verify               check the ARTIFACTS, not the exit codes
#
# configure knobs (env): ROS2=1  DLSS=<path|disabled>  RSS=1  OSM2ODR=1  PYTORCH=1
#                        MAPS="Town10HD_Opt,Town12"    NO_QT=1   EXTRA="-DFoo=Bar"
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
set +e

MODE="${1:-}"; shift 2>/dev/null || true
[ -n "${CARLA_UE58_ROOT}" ] || { echo "CARLA_UE58_ROOT is not set — run check_env.sh" >&2; exit 2; }
BUILD_DIR="$(carla_ue58_build_dir)"

need_configured() {
  carla_ue58_configured && return 0
  echo "ERROR: ${BUILD_DIR} is not configured. Run: bash build.sh configure" >&2
  exit 3
}

case "${MODE}" in

targets)
  # The generated Help.md is the authoritative list; fall back to cmake's own
  # target listing when the tree has not been configured yet.
  if [ -f "${BUILD_DIR}/Help.md" ]; then
    echo "# targets defined by ${CARLA_UE58_ROOT} (from ${BUILD_DIR}/Help.md)"
    sed -n '/## CMake Targets/,/## CMake Options/p' "${BUILD_DIR}/Help.md" | sed '$d'
  else
    echo "# not configured; listing targets via cmake"
    cmake --build "${BUILD_DIR}" --target help 2>/dev/null | head -60 \
      || echo "run 'bash build.sh configure' first"
  fi
  ;;

options)
  if [ -f "${BUILD_DIR}/Help.md" ]; then
    sed -n '/## CMake Options/,$p' "${BUILD_DIR}/Help.md"
  else
    echo "not configured yet — run 'bash build.sh configure', then this reads ${BUILD_DIR}/Help.md" >&2
    exit 3
  fi
  ;;

cache)
  # Every -D option must be repeated on each re-configure, so the cache is the
  # only reliable answer to "what is this build actually made of".
  need_configured
  echo "# ${BUILD_DIR}/CMakeCache.txt"
  grep -E '^(CMAKE_BUILD_TYPE|BUILD_[A-Z_]+|ENABLE_[A-Z0-9_]+|CARLA_[A-Z0-9_]+):' \
    "${BUILD_DIR}/CMakeCache.txt" | sort
  ;;

configure)
  ARGS=(--preset "${CARLA_PRESET}")
  [ "${ROS2:-0}" = "1" ]     && ARGS+=(-DENABLE_ROS2=ON)
  [ "${RSS:-0}" = "1" ]      && ARGS+=(-DENABLE_RSS=ON)
  [ "${OSM2ODR:-0}" = "1" ]  && ARGS+=(-DENABLE_OSM2ODR=ON)
  [ "${PYTORCH:-0}" = "1" ]  && ARGS+=(-DENABLE_PYTORCH=ON)
  [ -n "${DLSS:-}" ]         && ARGS+=(-DCARLA_DLSS_SDK_PATH="${DLSS}")
  # System Qt is often installed but not linkable against the UE toolchain; the
  # in-tree guard only checks that it is installed.
  [ "${NO_QT:-0}" = "1" ]    && ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON)
  [ -n "${CARLA_UNREAL_ENGINE_PATH}" ] && ARGS+=(-DCARLA_UNREAL_ENGINE_PATH="${CARLA_UNREAL_ENGINE_PATH}")

  if [ -n "${MAPS:-}" ]; then
    # UAT's -MapsToCook wants '+'-separated PACKAGE paths. Accept a friendly
    # comma-separated list of town names and expand it, since the long-hand form
    # is easy to get wrong (filesystem paths and ';' both silently fail).
    SPEC=""
    IFS=',' read -ra _towns <<<"${MAPS}"
    for t in "${_towns[@]}"; do
      t="$(echo "$t" | xargs)"   # trim
      case "$t" in
        /Game/*)          p="$t" ;;
        Town12|Town13|Town15|Town11)
                          # Large maps live in their own subdirectory.
                          p="/Game/Carla/Maps/${t}/${t}" ;;
        *)                p="/Game/Carla/Maps/${t}" ;;
      esac
      SPEC="${SPEC:+${SPEC}+}${p}"
      if [ "$t" = "Town15" ]; then
        echo "WARNING Town15 cannot be cooked: its one-file-per-actor packages reference a" >&2
        echo "        MaterialInstanceDynamic that is never saved (573 unresolvable imports)." >&2
      fi
    done
    ARGS+=(-DCARLA_MAPS_TO_COOK="${SPEC}")
    echo "[build] cook scope: ${SPEC}"
  fi
  [ -n "${EXTRA:-}" ] && read -ra _x <<<"${EXTRA}" && ARGS+=("${_x[@]}")

  echo "[build] cmake ${ARGS[*]}"
  echo "[build] NOTE these options are stored in the cache; repeat them on EVERY re-configure"
  cd "${CARLA_UE58_ROOT}" && cmake "${ARGS[@]}" "$@"
  ;;

libcarla|pythonapi|unreal|editor|launch|launch-only|package|target)
  need_configured
  case "${MODE}" in
    libcarla)    TARGETS=(carla-server carla-client) ;;
    pythonapi)   TARGETS=(carla-python-api-install) ;;
    unreal)      TARGETS=(carla-unreal) ;;
    editor)      TARGETS=(carla-unreal-editor) ;;
    launch)      TARGETS=(launch) ;;
    launch-only) TARGETS=(launch-only) ;;
    package)     echo "[build] use the package-carla-ue58 skill for packaging" >&2; TARGETS=(package) ;;
    target)      TARGETS=("${1:?usage: bash build.sh target <NAME>}"); shift ;;
  esac
  for t in "${TARGETS[@]}"; do
    echo "[build] cmake --build ${BUILD_DIR} --target ${t}"
    cd "${CARLA_UE58_ROOT}" && cmake --build "${BUILD_DIR}" --target "${t}" "$@"
    RC=$?
    if [ ${RC} -ne 0 ]; then
      echo "[build] target ${t} FAILED (${RC})" >&2
      exit ${RC}
    fi
  done
  echo "[build] done — now verify the artifacts: bash build.sh verify"
  ;;

verify)
  echo "== Engine =="
  EB="${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor"
  [ -x "${EB}" ] && echo "  PASS UnrealEditor $(date -r "${EB}" '+%Y-%m-%d %H:%M')" \
    || echo "  FAIL UnrealEditor missing"

  echo "== LibCarla =="
  for lib in libcarla-server.a libcarla-client.a; do
    # `find`, not a glob: bash has no globstar here and the artifacts land in
    # Build/<preset>/LibCarla/, not Build/<preset>/lib/.
    f="$(find "${BUILD_DIR}" -maxdepth 3 -name "${lib}" -print -quit 2>/dev/null)"
    [ -n "${f}" ] && echo "  PASS ${lib} $(date -r "${f}" '+%Y-%m-%d %H:%M')" \
      || echo "  WARN ${lib} not found under ${BUILD_DIR} (target: carla-server / carla-client)"
  done

  echo "== Python API =="
  W="$(ls -1t "${BUILD_DIR}"/PythonAPI/dist/carla-*.whl 2>/dev/null | head -1)"
  if [ -n "${W}" ]; then
    echo "  PASS wheel $(basename "${W}") $(date -r "${W}" '+%Y-%m-%d %H:%M')"
  else
    echo "  WARN no wheel under ${BUILD_DIR}/PythonAPI/dist"
  fi
  "${PYTHON}" - <<'PY'
import importlib.util as u
def have(mod):
    # find_spec imports parent packages, so a dotted name raises when the parent
    # is missing — which is the condition being tested.
    try:
        return u.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError):
        return False
if not have("carla"):
    print("  WARN `carla` not importable by this interpreter (target: carla-python-api-install)")
else:
    import carla
    if getattr(carla, "__file__", None) is None:
        print(f"  FAIL `carla` resolved to a DIRECTORY, not the client: {getattr(carla,'__path__','?')}")
        print("  FAIL   a folder named carla on sys.path (the CWD counts) shadows the module")
    else:
        try:
            from importlib.metadata import version
            print(f"  PASS carla {version('carla')} importable")
        except Exception:
            print("  PASS carla importable (version unknown)")
PY

  echo "== Carla plugin =="
  PB="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux"
  if [ -d "${PB}" ]; then
    N=$(ls -1 "${PB}"/*.so 2>/dev/null | wc -l)
    echo "  PASS ${N} shared object(s) in Plugins/Carla/Binaries/Linux"
  else
    echo "  WARN ${PB} does not exist yet"
  fi

  echo "== ROS 2 =="
  R="$(carla_ue58_cmake_opt ENABLE_ROS2)"
  if [ "${R}" = "ON" ]; then
    if [ -f "${PB}/libcarla-ros2-native.so" ]; then
      echo "  PASS libcarla-ros2-native.so present"
    else
      echo "  FAIL ENABLE_ROS2=ON but libcarla-ros2-native.so is missing"
    fi
    # A relink can succeed against cached objects and leave a plugin with no ROS
    # symbols at all, so assert on the symbols rather than on the build log.
    # grep -c, never grep -q: -q exits on the first match, nm dies of SIGPIPE,
    # and `set -o pipefail` would turn that into a failed pipeline.
    SO="$(ls -1t "${PB}"/*Carla*.so 2>/dev/null | head -1)"
    if [ -n "${SO}" ] && command -v nm >/dev/null 2>&1; then
      C="$(nm -DC "${SO}" 2>/dev/null | grep -c 'carla::ros2' || true)"
      [ "${C:-0}" -gt 0 ] && echo "  PASS ${C} carla::ros2 symbols in $(basename "${SO}")" \
        || echo "  FAIL 0 carla::ros2 symbols in $(basename "${SO}") — the plugin has no ROS 2 in it"
    fi
  else
    echo "  INFO ENABLE_ROS2=${R:-<unset>} — built without native ROS 2"
  fi

  echo "== Package =="
  P="$(carla_ue58_package_sh)"
  [ -n "${P}" ] && echo "  PASS ${P}" || echo "  INFO no package (package-carla-ue58 skill)"
  ;;

*)
  echo "usage: bash build.sh {targets|options|cache|configure|libcarla|pythonapi|unreal|editor|launch|launch-only|target <NAME>|verify}" >&2
  exit 2
  ;;
esac
