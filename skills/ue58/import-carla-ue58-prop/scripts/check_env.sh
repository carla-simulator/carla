#!/usr/bin/env bash
# Prerequisite checks for import-carla-ue58-prop (CARLA on UE 5.8). Read-only, no sudo, fast —
# the MCP check_prerequisites(name) tool must never hang.
# Hard blockers: no checkout, wrong engine major.minor, no engine binary, no cmake.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
# env.sh runs `set -euo pipefail`, and sourcing applies -e to THIS shell. A
# preflight must report every problem rather than stop at the first.
set +e

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== CARLA checkout =="
if [ -z "${CARLA_UE58_ROOT}" ] || ! carla_ue58_is_root "${CARLA_UE58_ROOT}"; then
  bad "no UE5 CARLA checkout — export CARLA_UE58_ROOT=/path/to/carla (branch ue58-dev)"
else
  ok "checkout at ${CARLA_UE58_ROOT} (branch $(carla_ue58_branch))"
  case "$(carla_ue58_flavor)" in
    ue58) ok "flavor ue58-dev (UE 5.8): CMake build, Autoware + DLSS present" ;;
    ue5)  bad "this looks like a ue5-dev checkout (UE 5.5), not ue58-dev"
          bad "  use the ue5 skill group instead — the engine fork and content branch differ" ;;
    *)    warn "cannot tell ue58-dev from ue5-dev in this tree" ;;
  esac
  # No Makefile is the point, not an error: it is what distinguishes this tree
  # from every UE4 skill's assumptions.
  [ -f "${CARLA_UE58_ROOT}/Makefile" ] \
    && warn "a Makefile exists here — is this really a UE5 tree? UE5 CARLA builds with CMake only" \
    || ok "no Makefile / Util/BuildTools (expected: CMake-only build)"
  if [ -d "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Content/Carla" ] \
     && [ -n "$(ls -A "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Content/Carla" 2>/dev/null)" ]; then
    ok "Content/Carla populated ($(ls -1 "${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Content/Carla/Maps" 2>/dev/null | wc -l) map entries)"
  else
    bad "Content/Carla missing — clone carla-content branch ue58-dev-carla into Unreal/CarlaUnreal/Content/Carla"
  fi
fi

echo "== Unreal Engine =="
if [ -z "${CARLA_UNREAL_ENGINE_PATH}" ]; then
  bad "CARLA_UNREAL_ENGINE_PATH unset and no engine found — needs the CarlaUnreal fork, branch ue58-dev-carla"
else
  EV="$(carla_engine_version)"
  case "${EV}" in
    5.8) ok "engine ${EV} at ${CARLA_UNREAL_ENGINE_PATH}" ;;
    5.5) bad "engine is ${EV} (ue5-dev-carla) — ue58-dev needs 5.8; use the ue5 group for this engine" ;;
    unknown) bad "cannot read ${CARLA_UNREAL_ENGINE_PATH}/Engine/Build/Build.version" ;;
    *)   warn "engine reports ${EV}; ue58-dev is validated against 5.8 only" ;;
  esac
  EB="${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor"
  # `make` in the engine can exit 0 with targets dead, so check the binary, not
  # the exit code of whatever built it.
  [ -x "${EB}" ] && ok "UnrealEditor built ($(du -h "${EB}" 2>/dev/null | cut -f1))" \
    || bad "UnrealEditor missing — build the engine (Setup.sh, GenerateProjectFiles.sh, make WITHOUT -j)"
  BR="$(git -C "${CARLA_UNREAL_ENGINE_PATH}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)"
  [ "${BR}" = "ue58-dev-carla" ] && ok "engine branch ${BR}" \
    || warn "engine branch is '${BR}', expected ue58-dev-carla"
fi

echo "== Toolchain =="
if command -v cmake >/dev/null; then
  CV="$(cmake --version | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')"
  if [ "$(printf '%s\n3.28.0\n' "${CV}" | sort -V | head -1)" = "3.28.0" ]; then
    ok "cmake ${CV} (>= 3.28 required)"
  else
    bad "cmake ${CV} is too old — 3.28 or newer is required by CMakePresets.json"
  fi
else
  bad "cmake missing — this build is CMake-only"
fi
command -v ninja >/dev/null && ok "ninja $(ninja --version)" || warn "ninja not on PATH (the presets default to it)"
command -v git >/dev/null && git lfs version >/dev/null 2>&1 && ok "git-lfs present" \
  || warn "git-lfs missing — the content repository needs it"
PV="$("${PYTHON}" -c 'import sys; print("%d.%d" % sys.version_info[:2])' 2>/dev/null || echo '?')"
case "${PV}" in
  3.8|3.9|3.10|3.11|3.12) ok "Python ${PV} (3.8-3.12 supported)" ;;
  *) warn "Python ${PV} is outside the documented 3.8-3.12 range" ;;
esac

echo "== Never run the build under sudo =="
# A single sudo'd UE or CARLA script leaves these root-owned and every later
# build fails in ways that do not name the cause.
POISON=0
for d in "${HOME}/.nuget" "${HOME}/.epic" "${HOME}/.lldbinit"; do
  if [ -e "${d}" ] && [ "$(stat -c %U "${d}" 2>/dev/null)" != "$(id -un)" ]; then
    bad "${d} is owned by $(stat -c %U "${d}") — a sudo'd build poisoned it"
    POISON=1
  fi
done
[ "${POISON}" -eq 0 ] && ok "~/.nuget, ~/.epic, ~/.lldbinit are user-owned" \
  || bad "  recover: sudo chown -R \$USER:\$USER ~/.nuget ~/.epic ~/.lldbinit"

echo "== Configure state (preset ${CARLA_PRESET}) =="
if carla_ue58_configured; then
  ok "configured: $(carla_ue58_build_dir)/CMakeCache.txt"
  # Every -D option must be repeated on each re-configure, so the cache is the
  # only honest answer to "was this built with ROS 2 / DLSS / which maps".
  for opt in CMAKE_BUILD_TYPE ENABLE_ROS2 CARLA_UNREAL_RHI CARLA_UNREAL_PACKAGE_BUILD_TYPE \
             ENABLE_RSS ENABLE_OSM2ODR CARLA_DLSS_SDK_PATH; do
    v="$(carla_ue58_cmake_opt "${opt}")"
    [ -n "${v}" ] && ok "  ${opt} = ${v}"
  done
  MAPS="$(carla_ue58_cmake_opt CARLA_MAPS_TO_COOK)"
  if [ -n "${MAPS}" ]; then
    ok "  CARLA_MAPS_TO_COOK = ${MAPS}"
    # Town15's one-file-per-actor packages reference a MaterialInstanceDynamic
    # that is never saved, so the cook fails with 573 unresolvable imports.
    case "${MAPS}" in *Town15*)
      bad "  Town15 is in the cook list and CANNOT be cooked (573 import errors, content defect)"
      bad "    drop it: -DCARLA_MAPS_TO_COOK=\"...\" without Town15" ;;
    esac
  else
    warn "  CARLA_MAPS_TO_COOK unset — the package will cook every map, including Town15 (which fails)"
  fi
else
  warn "not configured for preset ${CARLA_PRESET} — run: cmake --preset ${CARLA_PRESET}"
fi

echo "== Large maps (World Partition) =="
CARLA_CPP="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Carla.cpp"
if [ -f "${CARLA_CPP}" ]; then
  if grep -q 'MountExternalPackageRoots' "${CARLA_CPP}"; then
    ok "OFPA mount patch present (MountExternalPackageRoots in Carla.cpp)"
  else
    warn "OFPA mount patch ABSENT — Town12/Town13/Town15 load with an empty World Partition"
    warn "  and a black screen: Content/Carla/__External{Actors,Objects}__ is not a mount point"
  fi
fi

echo "== Artifacts =="
PKG="$(carla_ue58_package_sh)"
[ -n "${PKG}" ] && ok "packaged server: ${PKG}" || warn "no package built yet (target: package)"
if [ -n "${CARLA_UE58_ROOT}" ]; then
  W="$(ls -1t "${CARLA_UE58_ROOT}"/Build/*/PythonAPI/dist/carla-*.whl 2>/dev/null | head -1)"
  [ -n "${W}" ] && ok "python wheel: $(basename "${W}")" \
    || warn "no Python API wheel (target: carla-python-api-install)"
  AV="$(df -Pm "${CARLA_UE58_ROOT}" | awk 'NR==2{print $4}')"
  [ "${AV:-0}" -ge 100000 ] && ok "$((AV/1024)) GB free" \
    || warn "only $((AV/1024)) GB free — a full build + package needs ~400 GB"
  # -P and --output are mutually exclusive in coreutils df; -P alone gives the
  # source in field 1 of the second line.
  SRC="$(df -P "${CARLA_UE58_ROOT}" 2>/dev/null | awk 'NR==2{print $1}')"
  case "${SRC}" in
    /dev/*) ok "on a local block device (${SRC})" ;;
    "")     warn "could not determine the backing device" ;;
    *)      warn "${SRC} — never build on network or external storage" ;;
  esac
fi

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
