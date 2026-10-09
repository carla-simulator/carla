#!/usr/bin/env bash
# Step 06 — build the CarlaUE4 SERVER (editor C++ modules), and OPTIONALLY open
# the editor UI.
#
# This skill's job is to compile CARLA's source artifacts — engine (03), server
# (this step) and PythonAPI (04). COOKING/PACKAGING into a distributable Dist/
# tarball is NOT built here: that is [[package-carla-ue4]] (`make package`).
# RUNNING the result is [[run-carla-server]]. See SKILL.md "Scope".
#
# Targets (TARGET=, default `server`):
#   server  -> `make CarlaUE4Editor`. Editor C++ modules incl the Carla server
#              plugin (libUE4Editor-Carla*.so) and the CarlaTools plugin
#              (libUE4Editor-CarlaTools*.so, VehicleAuthoringLibrary). Binaries
#              only — no cook, no UI. This is what an uncooked -nullrhi server
#              ([[run-carla-server]]) and the verify step (07) need, and the
#              cheap incremental recompile after touching Unreal/CarlaUE4/Plugins/
#              — the target [[add-carla-vehicle]] points at when it reports
#              STATUS=REBUILD_CARLATOOLS_REQUIRED.
#   launch  -> `make launch`. Builds CarlaUE4Editor (as above) AND opens the UE4
#              editor UI. Interactive; needs a display. Opt-in — for editor work,
#              not headless/agent runs.
#
# No client Python env is required here: unlike `make package` (L15), neither
# CarlaUE4Editor nor launch build the Python wheel.
#
# ROS2=1 builds the native ROS 2 interface in (`ARGS="--ros2"`, see env.sh and
# references/ros2.md). The flag is BUILD-TIME state: BuildCarlaUE4.sh rewrites
# Config/OptionalModules.ini on every run, so a plain (ROS2=0) run of this step
# turns it back OFF. This step therefore treats a flag flip as a reason to
# rebuild even when the .so exists — otherwise the skip below would leave a
# binary whose ROS 2 support does not match what was asked for.
#
# Idempotent (TARGET=server): skips when the server plugin .so is already present
# AND its ROS 2 state matches ROS2=, unless FORCE=1. `make` is incremental
# regardless, so a re-run is cheap.
#
# NOTE (L16): the top-level make MUST see UE4_ROOT exported, or the generated
# Build/clang{,++}.sh wrappers bake a broken '/Engine/...' compiler path and
# every compile fails. env.sh (sourced below) guarantees it.
#
# Prereqs: step 03 (UE4 built). Content (05) only matters at run time, not here.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

TARGET="${TARGET:-server}"

[ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ] \
  || { echo "[server] ERROR: UE4 not built (step 03)."; exit 1; }

PLUGIN_SRC="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/CarlaTools/Source/CarlaTools"
[ -d "${PLUGIN_SRC}" ] \
  || { echo "[server] ERROR: CarlaTools plugin source missing at ${PLUGIN_SRC} — update the ue4-dev checkout."; exit 1; }
[ -n "$(find "${PLUGIN_SRC}" -name 'VehicleAuthoringLibrary.h' -print -quit)" ] \
  || echo "[server] WARN: VehicleAuthoringLibrary.h not found — branch may predate PR #9805; add-carla-vehicle will not work."

CARLATOOLS_SO="$(find "${CARLA_UE4_ROOT}/Unreal/CarlaUE4" -name 'libUE4Editor-CarlaTools*.so' -print -quit 2>/dev/null || true)"

# ROS 2 build-time state: what was asked for vs what the checkout currently has.
CARLA_SO="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/Carla/Binaries/Linux/libUE4Editor-Carla.so"
WANT_ROS2="off"; [ "${ROS2}" = "1" ] && WANT_ROS2="on"
# Prefer the BINARY over the ini: the ini records what was asked for, the symbols
# record what was built, and those two disagree whenever UBT relinked without
# recompiling (see the object-cache note below). Reading the binary is what makes
# this guard self-healing instead of trusting a file that can lie.
HAVE_ROS2="$(carla_ros2_ini_state)"
if [ -f "${CARLA_SO}" ] && command -v nm >/dev/null 2>&1; then
  # grep -c, never grep -q: -q exits on the first match, nm then dies of SIGPIPE,
  # and `set -o pipefail` turns that into a FAILED pipeline — so a binary that DOES
  # contain the symbols reads as "off" (verified: exit 141 with 1112 matches).
  # -c drains the stream, so the exit status means what it says.
  if [ "$(nm -DC "${CARLA_SO}" 2>/dev/null | grep -c 'carla::ros2' || true)" -gt 0 ]; then
    HAVE_ROS2="on"
  else
    HAVE_ROS2="off"
  fi
fi
if [ "${ROS2}" = "1" ]; then
  echo "[server] ROS2=1 — building the native ROS 2 interface in (ARGS=\"--ros2\")."
  echo "[server] NOTE: 'parse-options: unrecognized option --ros2' from BuildUE4Plugins.sh"
  echo "[server]       is expected and harmless — that script drops unknown options."
fi

case "${TARGET}" in
  server)
    if [ -n "${CARLATOOLS_SO}" ] && [ "${FORCE:-0}" != "1" ] && [ "${HAVE_ROS2}" = "${WANT_ROS2}" ]; then
      echo "[server] already built (CarlaTools .so present, Ros2 ${HAVE_ROS2}) — skipping. FORCE=1 to rebuild."
      echo "[server] ${CARLATOOLS_SO}"
      exit 0
    fi
    if [ -n "${CARLATOOLS_SO}" ] && [ "${HAVE_ROS2}" != "${WANT_ROS2}" ]; then
      echo "[server] ROS 2 flag changed (built: Ros2 ${HAVE_ROS2}, requested: Ros2 ${WANT_ROS2}) — rebuilding."
      # Re-running make is NOT enough (verified 2026-08): Carla.Build.cs turns the
      # ini into the WITH_ROS2 *preprocessor definition*, but UBT tracks source
      # files, not Config/OptionalModules.ini. With no source change it reuses the
      # cached .o files and merely RELINKS — producing a plugin with zero
      # carla::ros2 symbols while every log line says success. Deleting the Carla
      # module's object cache is what forces the recompile that picks the
      # definition up. Scoped to that one module, so it costs minutes, not a full
      # plugin rebuild.
      # find, not a fixed-depth glob: the path carries a platform id, a project
      # hash and the build configuration (…/Build/Linux/<hash>/UE4Editor/Development/Carla),
      # none of which are stable. Generated headers under Inc/ are left alone —
      # only the object cache has to go.
      _INT="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/Carla/Intermediate/Build"
      if [ -d "${_INT}" ]; then
        find "${_INT}" -type d -name Carla -not -path "*/Inc/*" -print 2>/dev/null | while IFS= read -r objdir; do
          echo "[server] invalidating object cache: ${objdir}"
          rm -rf "${objdir}"
        done
      fi
    fi
    cd "${CARLA_UE4_ROOT}"
    echo "[server] make CarlaUE4Editor ${CARLA_ROS2_ARG:+ARGS=\"${CARLA_ROS2_ARG}\"} (UBT incremental; minutes)..."
    make CarlaUE4Editor ${CARLA_ROS2_ARG:+ARGS="${CARLA_ROS2_ARG}"}
    # Ground truth is the produced module, not make's exit code (L13).
    SO="$(find "${CARLA_UE4_ROOT}/Unreal/CarlaUE4" -name 'libUE4Editor-CarlaTools*.so' -print -quit)"
    [ -n "${SO}" ] \
      || { echo "[server] ERROR: build finished but libUE4Editor-CarlaTools.so was not produced."; exit 1; }
    # Confirm the ROS 2 state actually written, not the flag we passed (L13:
    # ground truth is the artifact). Config/OptionalModules.ini is what
    # Carla.Build.cs read to decide WITH_ROS2.
    GOT_ROS2="$(carla_ros2_ini_state)"
    [ "${GOT_ROS2}" = "${WANT_ROS2}" ] \
      || { echo "[server] ERROR: requested Ros2 ${WANT_ROS2} but OptionalModules.ini says Ros2 ${GOT_ROS2}."; exit 1; }
    # The ini only records what was ASKED for. Ground truth is whether the server
    # plugin really carries the ROS 2 code: with WITH_ROS2 undefined the module
    # compiles fine and links with zero carla::ros2 symbols, and nothing else in
    # the build says so (verified 2026-08). Check the binary itself.
    if [ "${WANT_ROS2}" = "on" ] && [ -f "${CARLA_SO}" ] && command -v nm >/dev/null 2>&1; then
      ROS2_SYMS="$(nm -DC "${CARLA_SO}" 2>/dev/null | grep -c 'carla::ros2' || true)"
      [ "${ROS2_SYMS:-0}" -gt 0 ] \
        || { echo "[server] ERROR: libUE4Editor-Carla.so has no carla::ros2 symbols — WITH_ROS2 did not reach the compile."; \
             echo "[server]        UBT reused cached objects; delete Plugins/Carla/Intermediate/Build/*/*/*/Carla and re-run."; exit 1; }
      echo "[server] verified: ${ROS2_SYMS} carla::ros2 symbols in libUE4Editor-Carla.so"
    fi
    echo "[server] DONE (server): ${SO} (Ros2 ${GOT_ROS2})"
    ;;
  launch)
    cd "${CARLA_UE4_ROOT}"
    echo "[server] make launch ${CARLA_ROS2_ARG:+ARGS=\"${CARLA_ROS2_ARG}\"} (build CarlaUE4Editor + open the editor UI; ~30-60min first run)..."
    echo "[server] NOTE: interactive — needs a display; opt-in for editor work."
    # With --ros2, BuildCarlaUE4.sh also appends --ros2 to the editor's own
    # command line, so the launched editor runs with ROS 2 active (no separate
    # --editor-flags needed). RMW=/ROS_DOMAIN_ID= go through the same ARGS.
    make launch ${CARLA_ROS2_ARG:+ARGS="${CARLA_ROS2_ARG}${RMW:+ --rmw=${RMW}}${ROS_DOMAIN_ID:+ --ros-domain-id=${ROS_DOMAIN_ID}}"}
    echo "[server] DONE (launch)."
    ;;
  *)
    echo "[server] ERROR: unknown TARGET='${TARGET}' (expected 'server' or 'launch')."; exit 1
    ;;
esac
