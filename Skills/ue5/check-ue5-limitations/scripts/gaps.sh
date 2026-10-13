#!/usr/bin/env bash
# What CARLA on UE 5.5 (ue5-dev) lacks relative to UE 5.8 (ue58-dev).
#
#   gaps.sh list            the gaps, and which ue58 skill each one breaks
#   gaps.sh check           test THIS tree for each gap marker (needs CARLA_UE5_ROOT)
#   gaps.sh diff            measured diff against a ue58 tree (needs CARLA_UE58_ROOT too)
#
# 5.5 and 5.8 are the same CARLA line, not parallel products: the Python API is
# nearly identical, and 5.8 is the later revision — the one released as CARLA
# 1.0. So the ue58 skills ARE the procedures for 5.5 as well — except for the
# features below, which do not exist there at all.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

say(){ echo "$*"; }

cmd_list() {
  cat <<'EOF'
Missing on UE 5.5 (ue5-dev) — verified by diffing the trees, not assumed.

1. ROS 2 middleware selection
   5.8 has LibCarla/source/carla/ros2/middleware/ (RMW abstraction, QosProfile.h,
   ActiveMiddleware) and the CMake options CARLA_CYCLONEDDS_* / CARLA_ZENOH_C_*.
   5.5 has NO middleware directory and no such options: FastDDS only.
   -> --rmw cyclonedds / --rmw zenoh do not exist. run-carla-ue58-server's ROS 2
      section applies except for the RMW choice.
   -> add-ros-publisher: 5.5 has BasicPublisher and the helper headers, but QoS
      lives in middleware/PublisherQos.h-era code, NOT QosProfile.h.

2. Autoware integration — absent entirely
   No AutowareGNSSPublisher, AutowareVehicleStatusPublisher,
   AutowareControlSubscriber, AutowareSteeringCompensation.h; no
   sensor.other.autoware_gnss, no sensor.other.vehicle_status.
   -> run-autoware-ue58 has NO 5.5 counterpart. Do not attempt it.

3. DLSS and the ray-traced lens camera
   No enable_dlss / dlss_screen_percentage attributes, no
   SceneCaptureCamera_RayTracedLens.cpp, no CARLA_DLSS_SDK_PATH option.
   -> sensor.camera.rt_lens does not exist; create-sensor's DLSS notes are 5.8-only.

4. OFPA large-map mount patch
   Carla.cpp on 5.8 calls MountExternalPackageRoots (2 references); 5.5 has none.
   -> Town12 / Town13 / Town15 load with an EMPTY World Partition and a black
      screen on 5.5. This was the manual patch step that 5.8 absorbed upstream.

5. A few Python API additions
   5.8 only: World.get_ego_spawn_points(), World.set_publish_tf() /
   get_publish_tf(), World.spawn_custom_mesh(), Actor.enable_constant_acceleration()
   / disable_constant_acceleration().
   -> everything else in the python-api group applies to both.

Identical on both, so every finding transfers unchanged:
   CMake-only build (no Makefile, Util/Tools not Util/BuildTools); all six package
   targets; the MapsToCook default INCLUDING uncookable Town15, so an
   out-of-the-box package build fails on 5.5 too; the single-dash -ros2 flag; the
   actor factories and their Prop/Walker/VehicleParameters.json registration; the
   whole CarlaTools header set; RecastBuilder missing from Util/DockerUtils/dist,
   so map-import navmesh generation silently produces nothing; the
   `#if 0 // @CARLAUE5` wheel block that stops GenerateNewVehicleBlueprint
   producing a drivable car; and the Import.sh CARLA_BUILD_TOOLS_FOLDER defect.

Version note: 5.8 is the line that continues, released as CARLA 1.0; 5.5 still
declares 0.10.0, as did every ue58 build before that release. Treat 5.5 as an
earlier point release of the same line, not a fork.
EOF
}

cmd_check() {
  if [ -z "${CARLA_UE5_ROOT}" ]; then
    say "no ue5-dev checkout — set CARLA_UE5_ROOT (list still works without one)"
    exit 2
  fi
  say "tree     ${CARLA_UE5_ROOT}"
  say "branch   $(carla_ue5_branch)"
  local flavor; flavor="$(carla_ue5_flavor)"
  say "flavor   ${flavor}"
  if [ "${flavor}" = "ue58" ]; then
    say ""
    say "This tree has 5.8 markers (ros2/middleware or the Autoware publishers)."
    say "It is a ue58-dev tree: use the ue58 skills directly, none of the gaps below apply."
    exit 0
  fi
  local eng; eng="$(carla_ue5_expected_engine)"
  say "engine   expects ${eng:-<unstated>}  (5.5 trees name ue5-dev-carla)"
  say ""
  say "== Gap markers in this tree =="
  local L="${CARLA_UE5_ROOT}/LibCarla/source/carla"
  local P="${CARLA_UE5_ROOT}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla"

  [ -d "${L}/ros2/middleware" ] \
    && say "  ok      ros2/middleware present (RMW selection available)" \
    || say "  MISSING ros2/middleware -> FastDDS only; no --rmw cyclonedds/zenoh"

  [ -f "${L}/ros2/publishers/AutowareGNSSPublisher.cpp" ] \
    && say "  ok      Autoware publishers present" \
    || say "  MISSING Autoware publishers/subscriber -> run-autoware-ue58 does not apply"

  if [ -f "${P}/Carla.cpp" ]; then
    grep -q "MountExternalPackageRoots" "${P}/Carla.cpp" \
      && say "  ok      OFPA mount patch present (large maps can load)" \
      || say "  MISSING MountExternalPackageRoots -> Town12/13/15 load empty (black screen)"
  fi

  local S="${P}/Actor/ActorBlueprintFunctionLibrary.cpp"
  if [ -f "${S}" ]; then
    grep -q "enable_dlss" "${S}" \
      && say "  ok      DLSS camera attributes present" \
      || say "  MISSING enable_dlss / dlss_screen_percentage -> no DLSS attributes"
  fi
  [ -f "${P}/Sensor/SceneCaptureCamera_RayTracedLens.cpp" ] \
    && say "  ok      ray-traced lens camera present" \
    || say "  MISSING SceneCaptureCamera_RayTracedLens -> no sensor.camera.rt_lens"

  local W="${CARLA_UE5_ROOT}/PythonAPI/carla/src/World.cpp"
  if [ -f "${W}" ]; then
    grep -q "get_ego_spawn_points" "${W}" \
      && say "  ok      World.get_ego_spawn_points present" \
      || say "  MISSING World.get_ego_spawn_points / set_publish_tf / spawn_custom_mesh"
  fi

  say ""
  say "== Shared defects: present here too, so the ue58 notes apply verbatim =="
  [ -f "${CARLA_UE5_ROOT}/Util/DockerUtils/dist/RecastBuilder" ] \
    && say "  RecastBuilder IS in DockerUtils/dist (navmesh can be built)" \
    || say "  RecastBuilder absent from DockerUtils/dist -> map-import navmesh silently skipped"
  if [ -f "${CARLA_UE5_ROOT}/Util/Tools/Environment.sh" ]; then
    grep -q "CARLA_BUILD_TOOLS_FOLDER=" "${CARLA_UE5_ROOT}/Util/Tools/Environment.sh" \
      && say "  Environment.sh sets CARLA_BUILD_TOOLS_FOLDER (Import.sh works)" \
      || say "  Environment.sh does NOT set CARLA_BUILD_TOOLS_FOLDER -> Import.sh calls /Import.py, exit 2"
  fi
  grep -q "@CARLAUE5" "${CARLA_UE5_ROOT}/Unreal/CarlaUnreal/Plugins/CarlaTools/Source/CarlaTools/Private/USDImporterWidget.cpp" 2>/dev/null \
    && say "  USDImporterWidget still has the #if 0 wheel block -> vehicle import cannot drive"
  say ""
  say "Procedures: use the ue58 skills, minus the MISSING lines above."
}

cmd_diff() {
  if [ -z "${CARLA_UE5_ROOT}" ] || [ -z "${CARLA_UE58_ROOT}" ]; then
    say "needs BOTH CARLA_UE5_ROOT and CARLA_UE58_ROOT to measure a diff"
    exit 2
  fi
  local A="${CARLA_UE5_ROOT}" B="${CARLA_UE58_ROOT}"
  say "== ros2 layer (< only in 5.5, > only in 5.8) =="
  diff <(ls -1 "${A}/LibCarla/source/carla/ros2" 2>/dev/null) \
       <(ls -1 "${B}/LibCarla/source/carla/ros2" 2>/dev/null)
  say "== ros2 publishers =="
  diff <(ls -1 "${A}/LibCarla/source/carla/ros2/publishers" 2>/dev/null | sed 's/\..*//' | sort -u) \
       <(ls -1 "${B}/LibCarla/source/carla/ros2/publishers" 2>/dev/null | sed 's/\..*//' | sort -u)
  say "== CMake options only in 5.8 =="
  comm -13 <(grep -hoE "^  (CARLA|ENABLE|BUILD)_[A-Z0-9_]+" "${A}/CMake/Options.cmake" | tr -d ' ' | sort -u) \
           <(grep -hoE "^  (CARLA|ENABLE|BUILD)_[A-Z0-9_]+" "${B}/CMake/Options.cmake" | tr -d ' ' | sort -u) \
    | sed 's/^/  /'
  say "== Python API binding line changes =="
  local f n
  for f in Actor.cpp Sensor.cpp World.cpp Client.cpp Map.cpp Blueprint.cpp; do
    [ -f "${A}/PythonAPI/carla/src/${f}" ] && [ -f "${B}/PythonAPI/carla/src/${f}" ] || continue
    n="$(diff "${A}/PythonAPI/carla/src/${f}" "${B}/PythonAPI/carla/src/${f}" | grep -c '^[<>]')"
    printf "  %-14s %s changed lines\n" "${f}" "${n}"
  done
  say "== declared version =="
  printf "  5.5: %s\n" "$(grep -m1 -E '^## CARLA' "${A}/CHANGELOG.md" 2>/dev/null)"
  printf "  5.8: %s\n" "$(grep -m1 -E '^## CARLA' "${B}/CHANGELOG.md" 2>/dev/null)"
}

MODE="${1:-list}"; shift 2>/dev/null
case "${MODE}" in
  list)  cmd_list "$@" ;;
  check) cmd_check "$@" ;;
  diff)  cmd_diff "$@" ;;
  *) sed -n '2,7p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
