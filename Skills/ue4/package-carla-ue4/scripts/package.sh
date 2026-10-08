#!/usr/bin/env bash
# Cook and package CarlaUE4 into Dist/.
#
#   PACKAGES=Carla    -> full release:      Dist/CARLA_<tag>.tar.gz
#   PACKAGES=MyMaps   -> standalone assets: Dist/MyMaps_<tag>.tar.gz
#
# Content selection comes from Config/DefaultGame.ini (release) or from
# <Name>.Package.json (asset package). DefaultGame.ini is never modified.
#
# Knobs (references/packaging.md):
#   PACKAGES=Carla            comma-separated package names
#   PACKAGE_CONFIG=Shipping   Debug | Development | Shipping
#   PACKAGE_ZIP=1             0 -> --no-zip, staged tree only
#   CLEAN_INTERMEDIATE=0      1 -> delete the staged tree after archiving
#   ARCHIVE_SUFIX=            appended to artifact names (CARLA's spelling)
#   TARGET_ARCHIVE=           fold content packages into one named archive
#   PACKAGE_DEST=             dir to relocate verified artifacts into (created if
#                             absent); empty leaves them in Dist/
#   PACKAGE_DEST_MODE=move    move (default) empties Dist/ of the artifact;
#                             copy keeps Dist/ as the canonical build tree
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Capture any EXPLICIT version pin from the caller. Empty is the normal case:
# the wheel stage then uses plain `python3` from whatever environment is active,
# and no version is forwarded to `make`.
_PYV_PIN="${CARLA_PY_VERSION:-}"
# Optional CARLA_ENV_ACTIVATE hook; a no-op when unset.

# Self-contained env for this skill — provides UE4_ROOT / CARLA_UE4_ROOT and
# makes no environment-manager assumption. Keep CARLA_PY_VERSION empty unless
# the caller pinned it, so the active `python3` is used as-is.
# shellcheck disable=SC1091
source "${HERE}/env.sh"
[ -n "${_PYV_PIN}" ] || CARLA_PY_VERSION=""

PACKAGES="${PACKAGES:-Carla}"
PACKAGE_CONFIG="${PACKAGE_CONFIG:-Shipping}"
PACKAGE_ZIP="${PACKAGE_ZIP:-1}"
CLEAN_INTERMEDIATE="${CLEAN_INTERMEDIATE:-0}"
ARCHIVE_SUFIX="${ARCHIVE_SUFIX:-}"
TARGET_ARCHIVE="${TARGET_ARCHIVE:-}"
PACKAGE_DEST="${PACKAGE_DEST:-}"
PACKAGE_DEST_MODE="${PACKAGE_DEST_MODE:-move}"

CONTENT_DIR="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content"
DIST="${CARLA_UE4_ROOT}/Dist"

case "${PACKAGE_CONFIG}" in
  Debug|Development|Shipping) ;;
  *) echo "[package] ERROR: PACKAGE_CONFIG must be Debug, Development or Shipping (got '${PACKAGE_CONFIG}')." >&2; exit 2 ;;
esac

case "${PACKAGE_DEST_MODE}" in
  copy|move) ;;
  *) echo "[package] ERROR: PACKAGE_DEST_MODE must be copy or move (got '${PACKAGE_DEST_MODE}')." >&2; exit 2 ;;
esac

[ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ] \
  || { echo "[package] ERROR: UE4 not built (build UE4 first)." >&2; exit 1; }

IFS=',' read -r -a PKG_ARR <<< "${PACKAGES}"

# --- Validate asset-package selection --------------------------------------
# The commandlet locates <Name>.Package.json by recursive search under Content/,
# so the package needs no directory of its own. Checking here saves an hour:
# PrepareAssetsForCooking fails deep into the run.
for p in "${PKG_ARR[@]}"; do
  [ "${p}" = "Carla" ] && continue
  if ! find -L "${CONTENT_DIR}" -name "${p}.Package.json" -print -quit | grep -q .; then
    echo "[package] ERROR: no ${p}.Package.json under ${CONTENT_DIR}" >&2
    echo "[package] Create one: python3 scripts/package_json.py ${p} --map <MapName>" >&2
    echo "[package] Existing package configs:" >&2
    find -L "${CONTENT_DIR}" -name '*.Package.json' -printf '  %p\n' 2>/dev/null >&2
    exit 2
  fi
done

# A stale .tar from an interrupted run is APPENDED to by `tar -rf`, silently
# producing a corrupt, oversized archive.
for p in "${PKG_ARR[@]}"; do
  [ "${p}" = "Carla" ] && continue
  for stale in "${DIST}/${p}"_*.tar; do
    [ -e "${stale}" ] || continue
    echo "[package] ERROR: stale ${stale} would be appended to (tar -rf)." >&2
    echo "[package] Remove it first: rm '${stale}'" >&2
    exit 2
  done
done

# --- Assemble Package.sh args ----------------------------------------------
ARGS=("--packages=${PACKAGES}" "--config=${PACKAGE_CONFIG}")
[ "${PACKAGE_ZIP}" = "0" ]        && ARGS+=("--no-zip")
[ "${CLEAN_INTERMEDIATE}" = "1" ] && ARGS+=("--clean-intermediate")
[ -n "${ARCHIVE_SUFIX}" ]         && ARGS+=("--archive-sufix=${ARCHIVE_SUFIX}")
[ -n "${TARGET_ARCHIVE}" ]        && ARGS+=("--target-archive=${TARGET_ARCHIVE}")

# PythonAPI.wheel runs `python3 -m build` (or `python<pin> -m build` when a
# version is pinned); that interpreter must exist and carry `build`. It runs
# after the editor compile and before the cook, so a miss costs the compile
# (references/packaging.md P1). Verify it now and, ONLY when pinned, forward the
# version to `make` — otherwise the active env's `python3` is used as-is.
carla_require_wheel_python "${_PYV_PIN}" || exit 1
[ -n "${CARLA_PY_ARG}" ] && ARGS+=("${CARLA_PY_ARG}")

# ROS 2: forwarded through `make package` so the CarlaUE4Editor dependency
# re-runs BuildCarlaUE4.sh WITH --ros2 and keeps `Ros2 ON` in
# Config/OptionalModules.ini. Package.sh and BuildPythonAPI.sh don't declare the
# option and drop it with a harmless "unrecognized option" line on stderr; only
# the editor/LibCarla/Setup stages act on it (references/ros2.md).
if [ "${ROS2}" = "1" ]; then
  ARGS+=("--ros2")
  echo "[package] ROS2=1 — cooking with the native ROS 2 interface."
  echo "[package] NOTE: 'parse-options: unrecognized option --ros2' from Package.sh /"
  echo "[package]       BuildPythonAPI.sh is expected; they drop unknown options."
elif [ "$(carla_ros2_ini_state)" = "on" ]; then
  # The checkout was last built with ROS 2, but this cook would rewrite the ini
  # to `Ros2 OFF` and produce a package without it. Cheaper to say so now than
  # after a 30-90 min cook.
  echo "[package] WARN: this checkout is built with Ros2 ON, but ROS2=1 was not set."
  echo "[package]       The CarlaUE4Editor dependency will rewrite OptionalModules.ini"
  echo "[package]       to 'Ros2 OFF' and the package will have NO ROS 2 support."
  echo "[package]       Re-run with ROS2=1 to keep it. Continuing in 5s..."
  sleep 5
fi

# Mirrors get_git_repository_version (Util/BuildTools/Environment.sh) so artifact
# names are predictable. A dirty tree yields a '-dirty' tag.
cd "${CARLA_UE4_ROOT}"
BRANCH="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)"
if [[ "${BRANCH}" == ue4/* ]]; then
  TAG="${BRANCH#ue4/}"
else
  TAG="$(git rev-parse --short HEAD 2>/dev/null || echo nogit)"
  git diff-index --quiet HEAD -- 2>/dev/null || TAG="${TAG}-dirty"
fi

SUF=""; [ -n "${ARCHIVE_SUFIX}" ] && SUF="_${ARCHIVE_SUFIX}"
echo "[package] tag=${TAG}  packages=${PACKAGES}  config=${PACKAGE_CONFIG}  zip=${PACKAGE_ZIP}"
echo "[package] make package — expect 30-90 min on a cold shader cache..."

# The cook can SUCCEED and still fail the build: UE4Editor sometimes deadlocks on
# shutdown after "Success - 0 error(s)" (2 threads left in futex_wait, no I/O, log
# frozen), UAT waits for the child forever, and killing it makes UAT report
# Error_UnknownCookFailure. Verified 2026-08 — the cook sat idle for 51 minutes.
# The cooked data in Saved/Cooked survives, so the fix is to re-run this script:
# the second cook is incremental. Catch the failure to say so instead of leaving
# a bare `make` error.
if ! make package ARGS="${ARGS[*]}"; then
  echo "[package] ERROR: make package failed." >&2
  if grep -q "Success - 0 error" "${HOME}/Library/Logs/Unreal Engine/LocalBuildLogs/Log.txt" 2>/dev/null; then
    echo "[package] The cook itself reported success — this looks like the UE4Editor" >&2
    echo "[package] shutdown deadlock (see references/packaging.md). Saved/Cooked is" >&2
    echo "[package] intact, so simply RE-RUN this script; the next cook is incremental." >&2
  fi
  exit 1
fi

# The ini is what decided WITH_ROS2 for the modules this cook staged, so check it
# before trusting the package's ROS 2 support.
if [ "${ROS2}" = "1" ] && [ "$(carla_ros2_ini_state)" != "on" ]; then
  echo "[package] ERROR: ROS2=1 but Config/OptionalModules.ini says Ros2 $(carla_ros2_ini_state)." >&2
  echo "[package] The staged binaries have NO ROS 2 support — do not ship this package." >&2
  exit 1
fi

# --- Verify artifacts, not the exit code -----------------------------------
# PRODUCED collects the top-level Dist/ entry for each verified package (the
# .tar.gz, or the staged dir for --no-zip) so a later --dest relocation moves
# exactly what was built and passed verification, nothing stale.
MISSING=0
PRODUCED=()
for p in "${PKG_ARR[@]}"; do
  if [ "${p}" = "Carla" ]; then
    if [ "${PACKAGE_ZIP}" = "0" ]; then
      STAGE_DIR="${DIST}/CARLA_${PACKAGE_CONFIG}_${TAG}${SUF}"
      STAGED="${STAGE_DIR}/LinuxNoEditor"
      if [ -d "${STAGED}" ]; then
        echo "[package] OK  staged tree ${STAGED} ($(du -sh "${STAGED}" | cut -f1))"
        PRODUCED+=("${STAGE_DIR}")
      else
        echo "[package] FAILED: no staged tree at ${STAGED}"; MISSING=1
      fi
      continue
    fi
    if [ "${PACKAGE_CONFIG}" = "Shipping" ]; then
      ART="${DIST}/CARLA_${TAG}${SUF}.tar.gz"
    else
      ART="${DIST}/CARLA_${PACKAGE_CONFIG}_${TAG}${SUF}.tar.gz"
    fi
    if [ ! -f "${ART}" ]; then
      echo "[package] FAILED: expected ${ART}, not found."
      echo "[package] Read the FIRST error in the build log, not the last."
      MISSING=1; continue
    fi
    SIZE_MB=$(( $(stat -c%s "${ART}") / 1024 / 1024 ))
    echo "[package] OK  ${ART} (${SIZE_MB} MB)"
    if [ "${SIZE_MB}" -lt 500 ]; then
      echo "[package] WARNING: ${SIZE_MB} MB is far below a normal release (several GB)."
      echo "[package] Likely cooked without Content/Carla (build the carla content first)."
      MISSING=1
    else
      PRODUCED+=("${ART}")
    fi
  else
    # Content packages are tarred, then gzipped after the loop -> .tar.gz,
    # which is what ImportAssets.sh globs for.
    if [ -n "${TARGET_ARCHIVE}" ]; then
      ART="${DIST}/${TARGET_ARCHIVE}_${TAG}${SUF}.tar.gz"
    else
      ART="${DIST}/${p}_${TAG}${SUF}.tar.gz"
    fi
    if [ -f "${ART}" ]; then
      echo "[package] OK  ${ART} ($(( $(stat -c%s "${ART}") / 1024 / 1024 )) MB)"
      echo "[package]     install: cp '${ART}' RELEASE/Import/ && (cd RELEASE && ./ImportAssets.sh)"
      PRODUCED+=("${ART}")
    else
      echo "[package] FAILED: expected ${ART}, not found."; MISSING=1
    fi
  fi
done

if [ "${MISSING}" -ne 0 ]; then
  echo "[package] Contents of Dist/:"
  ls -lah "${DIST}" 2>/dev/null | sed 's/^/    /' || echo "    (no Dist/)"
  exit 1
fi

# --- Optional relocation to PACKAGE_DEST -----------------------------------
# Only reached when every package verified. move (default) leaves nothing behind
# in Dist/ — no multi-GB duplicate; copy keeps Dist/ as the canonical build tree.
# Uses TARGET_ARCHIVE folding note: with TARGET_ARCHIVE set, PRODUCED holds the
# single folded archive once, so it relocates once.
if [ -n "${PACKAGE_DEST}" ]; then
  DEST="$(mkdir -p "${PACKAGE_DEST}" && cd "${PACKAGE_DEST}" && pwd)" \
    || { echo "[package] ERROR: cannot create/enter PACKAGE_DEST '${PACKAGE_DEST}'." >&2; exit 1; }
  if [ "${DEST}" = "${DIST}" ]; then
    echo "[package] PACKAGE_DEST is Dist/ itself — nothing to relocate."
  else
    echo "[package] ${PACKAGE_DEST_MODE} ${#PRODUCED[@]} artifact(s) -> ${DEST}"
    # De-duplicate (TARGET_ARCHIVE makes several packages share one archive).
    declare -A SEEN=()
    for src in "${PRODUCED[@]}"; do
      [ -n "${SEEN[$src]:-}" ] && continue
      SEEN[$src]=1
      base="$(basename "${src}")"
      if [ "${PACKAGE_DEST_MODE}" = "move" ]; then
        rm -rf "${DEST}/${base}"           # replace any stale target atomically-ish
        mv -f "${src}" "${DEST}/"
      else
        cp -a "${src}" "${DEST}/"
      fi
      echo "[package]   ${PACKAGE_DEST_MODE}d -> ${DEST}/${base}"
    done
  fi
fi

echo "[package] DONE."
