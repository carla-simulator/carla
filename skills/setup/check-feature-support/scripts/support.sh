#!/usr/bin/env bash
# Answer "is feature X usable here, and is there a vetted procedure for it".
#
#   support.sh matrix              the support table (no CARLA needed)
#   support.sh broken              only what is broken or removed — read before promising
#   support.sh probe               inspect THIS machine: trees, build flags, live server
#   support.sh version             the identity tuple, and the mismatches that lie
#
# This script never changes anything. Its whole job is to stop an agent inventing
# a procedure for something that has none, or walking into a known crash.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

say(){ echo "$*"; }

cmd_matrix() {
  cat <<'EOF'
LEGEND  [skill]  a vetted procedure exists in this collection — use it
        [works]  verified working, but no skill: you are on your own for the steps
        [untested] present in the build, never exercised here: DO NOT invent steps
        [broken] verified broken or removed on the UE5 line (ue58-dev)

== Engine lines ==
  UE 5.8 (ue58-dev) is the line that continues, and the one released as CARLA
  1.0. UE 5.5 (ue5-dev) is an earlier revision of the SAME line, not a fork: the
  Python API is nearly identical. So the ue58 skills are the procedures for both,
  minus five gaps (no Autoware, FastDDS only, no DLSS/rt_lens, no OFPA large-map
  mount, a few missing World/Actor methods).
  Full list: skills/ue5/check-ue5-limitations, `gaps.sh list`.
  Read "0.10.0" below as "the UE5 line": it is what pre-1.0 ue58 builds report,
  what every measurement here was taken on, and what 5.5 reports today. 1.0
  renamed the line — the facts hold.

== Covered by a skill ==
  [skill] build / package / run a server            build-carla-ue58, package-carla-ue58,
                                                    run-carla-ue58-server (ue4 equivalents too)
  [skill] import props, walkers, maps               import-carla-ue58-{prop,walker,map}
  [skill] Autoware over native ROS 2                run-autoware-ue58
  [skill] native ROS 2 topics, publishers, types    ros2 group (3 skills)
  [skill] sensors, traffic, weather, recording, …   python-api group (23 skills)

== Works, no skill ==
  [works] texture streaming at runtime              World.apply_color_texture_to_object(s),
          apply_float_color_texture_to_object(s), apply_textures_to_object(s), with
          carla.TextureColor / FloatColor / MaterialParameter. All present on 0.10.0.
  [works] Chaos / CarSim vehicle physics hooks      Vehicle.enable_chrono_physics(),
          enable_carsim(), use_carsim_road() exist on 0.10.0 — but see [broken] below
          for the tooling that used to drive them.

== Present, untested here ==
  [untested] RSS (Responsibility-Sensitive Safety)  sensor.other.rss is in the blueprint
          library, but ENABLE_RSS is OFF in this build's cache: the sensor cannot work
          without it. Rebuild with -DENABLE_RSS=ON first.
  [untested] V2X messaging                          sensor.other.v2x, sensor.other.v2x_custom,
          plus ServerSideSensor.send() for custom messages. Never exercised.
  [untested] multi-GPU / distributed servers        LibCarla/source/carla/multigpu exists and
          the server accepts -carla-primary-host, -carla-primary-port,
          -carla-secondary-port. No procedure, no measurement.
  [untested] digital twins / OSM map generation     CarlaTools has OpenDriveToMap.h,
          DigitalTwinsBaseWidget.h, MapGeneratorWidget.h, ProceduralBuildingUtilities.h,
          ProceduralWaterManager.h — editor widgets, not a CLI. ENABLE_OSM2ODR is OFF in
          this build, so the OSM->OpenDRIVE half is not even compiled in.
  [untested] vehicle import on ue58                 TO BE DONE — no skill in this version.
          The pipeline is understood and was partly proved: import the skeletal mesh bound
          to a donor's skeleton, duplicate the donor's 4 UChaosVehicleWheel blueprints, set
          wheel_setups + the mesh on the duplicated vehicle BP's CDO, register in
          VehicleParameters.json. That much works: the vehicle registers, appears in the
          blueprint library, spawns, and get_physics_control() reports 4 wheels. It then
          does NOT move, because the physics asset does not persist on the skeletal mesh
          across an editor restart. Deferred to a later version rather than shipped
          half-working. Report it as to-be-done, and do not hand-roll the procedure from
          the walker skill's shape -- the two differ in exactly the step that is unsolved.

== Broken or removed on the UE5 line (measured on 0.10.0 / ue58-dev) ==
  [broken] GBuffer capture                          CRASHES THE SERVER. See `support.sh broken`.
  [broken] map layers (load_map_layer/unload)       accepted, silently do nothing.
  [broken] Landmark.waypoint                        always None.
  [broken] VehiclePhysicsControl gear ratios        forward_gear_ratios / reverse_gear_ratios
           have no Python converter; reading raises TypeError.
  [broken] standalone asset packages                producer and installer both gone.
  [broken] pedestrian navmesh on map import         silently skipped.
  [broken] semantic tagging of imported assets      GenerateTaggedMaterialsRegistry gone.
  [broken] SUMO / PTV-Vissim / Chrono co-simulation the whole Co-Simulation/ directory is
           absent from ue58-dev (UE4 ships Sumo, PTV-Vissim, Chrono, Carsim).
  [broken] rt/carla/map ROS topic                   CarlaMapPublisher removed.

If a feature is [untested] or [broken], say so plainly and offer to investigate.
Do NOT synthesise a procedure from UE4 documentation: the tree it describes is
not this tree, which is what every [broken] row above used to be.
EOF
}

cmd_broken() {
  cat <<'EOF'
Verified broken or removed on 0.10.0 / ue58-dev. Each line is a measurement or a
source reading, not a guess.

GBuffer capture — CRASHES THE SERVER
  listen_to_gbuffer() raises RuntimeError std::exception client-side and takes the
  simulator down with:
      Assertion failed: Stream.has_value() [Carla/Sensor/DataStream.h] [Line: 55]
      SIGSEGV ... FCameraGBufferUint8::GetToken() const
  The API surface still exists on carla.ServerSideSensor and all seven
  GBufferTextureID members are present, which is what makes it dangerous. A plain
  camera on the same sensor works. Treat gbuffers as nonexistent.

Map layers do nothing
  unload_map_layer(MapLayer.Buildings) then a 40-ray sweep: Buildings hit 9 times
  before, 9 after, 9 after reload; no streaming activity in the server log. Cause:
  Town10HD_Opt.umap is 32.9 MB with ZERO LevelStreaming references on 0.10.0
  (158 KB with two on 0.9.x) — the layers were baked into the persistent level, so
  the mask matches nothing. Use enable_environment_objects instead.

Landmark.waypoint is always None
  Town10HD_Opt: 68 landmarks, 68 with waypoint is None. Every other field
  populates. Use map.get_waypoint_xodr(road_id, lane_id, s) instead.

Gear ratios unreadable
  forward_gear_ratios / reverse_gear_ratios are declared with no
  std::vector<float> converter: TypeError: No to_python (by-value) converter
  found. Everything else on VehiclePhysicsControl reads, and writes still apply.

Standalone asset packages — gone both ways
  No Package.sh --packages= producer; no ImportAssets.sh installer in a built
  package. Only whole-server package targets exist, scoped by CARLA_MAPS_TO_COOK.

Pedestrian navmesh on map import — silent skip
  Import.py shells out to Util/DockerUtils/dist/build.sh, which is guarded by
  `if [ -f "RecastBuilder" ]` / `if [ -f "FBX2OBJ" ]`. UE4 ships RecastBuilder in
  that folder; ue58 does not, and no CMake target populates it. Import exits 0 and
  writes no .bin.

Semantic tags on imported assets
  GenerateTaggedMaterialsRegistry: 2 files in UE4, 0 in ue58. semantic_tags comes
  back empty on imported props.

Co-simulation tooling
  ue58-dev has no Co-Simulation/ directory at all. UE4 ships Sumo, PTV-Vissim,
  Chrono and Carsim bridges. The Vehicle.enable_chrono_physics() /
  enable_carsim() API calls survive, with nothing shipped to drive them.

rt/carla/map
  CarlaMapPublisher is absent from LibCarla/source/carla/ros2/publishers on
  0.10.0. Read the map with map.to_opendrive() over RPC.
EOF
}

cmd_probe() {
  say "== Trees offered for inspection =="
  local found=0 r
  for pair in "CARLA_UE58_ROOT:${CARLA_UE58_ROOT}" "CARLA_UE5_ROOT:${CARLA_UE5_ROOT}" \
              "CARLA_UE4_ROOT:${CARLA_UE4_ROOT}" "CARLA_TARGET:${CARLA_TARGET}"; do
    r="${pair#*:}"
    [ -n "${r}" ] || continue
    found=1
    if [ -d "${r}" ]; then say "  ${pair%%:*} = ${r}"; else say "  ${pair%%:*} = ${r}  (MISSING)"; fi
  done
  [ "${found}" -eq 1 ] || say "  none set — matrix and broken still work without a checkout"

  local root
  if root="$(carla_any_root)"; then
    say ""
    say "== Build flags in ${root}/Build/${CARLA_PRESET}/CMakeCache.txt =="
    local any=0 v
    for opt in ENABLE_ROS2 ENABLE_RSS ENABLE_OSM2ODR CARLA_UNREAL_RHI CMAKE_BUILD_TYPE; do
      if v="$(carla_cmake_opt "${root}" "${opt}")"; then
        say "  ${opt} = ${v}"
        any=1
        case "${opt}:${v}" in
          ENABLE_RSS:OFF)     say "      -> sensor.other.rss exists but cannot work; rebuild with -DENABLE_RSS=ON" ;;
          ENABLE_OSM2ODR:OFF) say "      -> OSM -> OpenDRIVE (digital twins) is not compiled in" ;;
          ENABLE_ROS2:OFF)    say "      -> no native ROS 2 interface at all; not switchable at runtime" ;;
        esac
      fi
    done
    [ "${any}" -eq 1 ] || say "  no cache for preset ${CARLA_PRESET} (tree not configured)"

    say ""
    say "== Features whose presence is visible in the tree =="
    [ -d "${root}/LibCarla/source/carla/multigpu" ] \
      && say "  multi-GPU sources present (untested)" || say "  multi-GPU sources absent"
    [ -d "${root}/Co-Simulation" ] \
      && say "  Co-Simulation/ present: $(ls -1 "${root}/Co-Simulation" | tr '\n' ' ')" \
      || say "  Co-Simulation/ ABSENT (no SUMO/Vissim/Chrono bridges shipped)"
    [ -f "${root}/Util/DockerUtils/dist/RecastBuilder" ] \
      && say "  RecastBuilder in DockerUtils/dist: map-import navmesh CAN be built" \
      || say "  RecastBuilder NOT in DockerUtils/dist: map-import navmesh silently skipped"
    [ -d "${root}/PythonAPI/examples/av_stacks/autoware" ] \
      && say "  Autoware integration shipped (see run-autoware-ue58)" \
      || say "  no Autoware integration in this tree"
  fi

  say ""
  say "== Live server at ${CARLA_HOST}:${CARLA_PORT} =="
  "${PYTHON}" - <<'PY'
import os, sys
try:
    import carla
except Exception:
    print("  carla module not importable — see install-python-api")
    sys.exit(0)
try:
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", 2000)))
    c.set_timeout(8.0)
    world = c.get_world()
    ver = c.get_server_version()
except Exception:
    print("  no server reachable — tree checks above still apply")
    sys.exit(0)
print(f"  server {ver}, map {world.get_map().name}")
lib = world.get_blueprint_library()
def have(bid):
    try:
        lib.find(bid); return True
    except Exception:
        return False
for bid, note in (("sensor.other.rss", "needs ENABLE_RSS=ON to function"),
                  ("sensor.other.v2x", "untested"),
                  ("sensor.other.v2x_custom", "untested; pairs with ServerSideSensor.send()"),
                  ("sensor.other.autoware_gnss", "see run-autoware-ue58"),
                  ("sensor.other.vehicle_status", "see run-autoware-ue58")):
    print(f"  {'present' if have(bid) else 'absent '} {bid:32} {note}")
w = carla.World
for name, note in (("apply_color_texture_to_object", "texture streaming: works, no skill"),
                   ("apply_textures_to_objects", "texture streaming: works, no skill"),
                   ("set_publish_tf", "UE5 line only: global rt/tf switch")):
    print(f"  {'present' if hasattr(w, name) else 'absent '} World.{name:30} {note}")
# The UE5 line reports 0.10.0 before the 1.0 release and 1.x after it; both are
# the same line, so gate on the line, not on one label.
if ver.startswith("0.10") or ver.split(".")[0].isdigit() and int(ver.split(".")[0]) >= 1:
    print(f"  UE5 line (server {ver}): gbuffers CRASH the server, map layers are a no-op"
          " — `support.sh broken`")
    if not ver.startswith("0.10"):
        print("  (those two were measured on 0.10.0; re-check them on this build)")
PY
}

cmd_version() {
  # "Which CARLA is this?" has five answers that can disagree, and a disagreement
  # is the failure this mode exists to catch: reasoning about a tree while talking
  # to a differently-built server is silently wrong, not loudly wrong.
  local root tree_date installed_mtime whl whl_mtime warn=0

  say "== Python client =="
  "${PYTHON}" - <<'PYEOF'
import datetime, os, sys
try:
    import carla
except Exception as exc:
    print(f"  not importable: {exc}")
    sys.exit(0)
path = getattr(carla, "__file__", "?")
try:
    stamp = datetime.date.fromtimestamp(os.path.getmtime(path)).isoformat()
except Exception:
    stamp = "?"
print(f"  module   {path}")
print(f"  built    {stamp}")
PYEOF

  say ""
  say "== Server at ${CARLA_HOST}:${CARLA_PORT} =="
  "${PYTHON}" - <<'PYEOF'
import os, sys
try:
    import carla
except Exception:
    print("  skipped (no carla module)")
    sys.exit(0)
try:
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", 2000)))
    c.set_timeout(8.0)
    client_v, server_v = c.get_client_version(), c.get_server_version()
    world = c.get_world()
except Exception:
    print("  not reachable")
    sys.exit(0)
print(f"  server   {server_v}")
print(f"  client   {client_v}")
print(f"  map      {world.get_map().name}")
# CARLA usually still CONNECTS across a skew, then misbehaves in ways that look
# like your own bug, which is why this is called out rather than left to fail.
if client_v != server_v:
    print(f"  MISMATCH client {client_v} != server {server_v}")
    print("           install the client that matches this server (install-python-api)")
PYEOF

  if root="$(carla_any_root)"; then
    say ""
    say "== Tree ${root} =="
    say "  branch   $(git -C "${root}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo '?')"
    say "  head     $(git -C "${root}" log -1 --format='%h %ad' --date=short 2>/dev/null || echo '?')"
    tree_date="$(git -C "${root}" log -1 --format=%at 2>/dev/null)"

    local content="${root}/Unreal/CarlaUnreal/Content/Carla"
    [ -d "${content}" ] || content="${root}/Unreal/CarlaUE4/Content/Carla"
    if git -C "${content}" rev-parse --git-dir >/dev/null 2>&1; then
      say "  content  $(git -C "${content}" rev-parse --abbrev-ref HEAD 2>/dev/null) @ $(git -C "${content}" log -1 --format='%h %ad' --date=short 2>/dev/null)"
    else
      say "  content  not a git checkout (or missing): ${content}"
    fi

    local eng
    eng="$(carla_cmake_opt "${root}" CARLA_UNREAL_ENGINE_PATH)"
    [ -n "${eng}" ] || eng="${CARLA_UNREAL_ENGINE_PATH:-}"
    if [ -n "${eng}" ] && [ -f "${eng}/Engine/Build/Build.version" ]; then
      say "  engine   $(tr -d ' \t",' < "${eng}/Engine/Build/Build.version" | awk -F: '
        /MajorVersion/{maj=$2} /MinorVersion/{min=$2} /PatchVersion/{pat=$2}
        END{printf "%s.%s.%s", maj, min, pat}')  (${eng})"
    fi

    say ""
    say "== Build flags (preset ${CARLA_PRESET}) =="
    local any=0 v
    for opt in CMAKE_BUILD_TYPE ENABLE_ROS2 ENABLE_RSS ENABLE_OSM2ODR CARLA_UNREAL_RHI CARLA_MAPS_TO_COOK; do
      if v="$(carla_cmake_opt "${root}" "${opt}")"; then say "  ${opt} = ${v}"; any=1; fi
    done
    [ "${any}" -eq 1 ] || say "  no cache for preset ${CARLA_PRESET} (tree unconfigured)"

    installed_mtime="$("${PYTHON}" -c 'import carla,os; print(int(os.path.getmtime(carla.__file__)))' 2>/dev/null)"
    whl="$(ls -t "${root}"/Build/*/PythonAPI/dist/*.whl 2>/dev/null | head -1)"
    if [ -n "${whl}" ]; then
      say ""
      say "== Wheel built from this tree =="
      say "  $(basename "${whl}")  ($(date -r "${whl}" +%Y-%m-%d))"
      whl_mtime="$(date -r "${whl}" +%s 2>/dev/null)"
      if [ -n "${installed_mtime}" ] && [ -n "${whl_mtime}" ] && [ "${whl_mtime}" -gt "${installed_mtime}" ]; then
        say "  MISMATCH this wheel is NEWER than the installed client:"
        say "           ${PYTHON} -m pip install --force-reinstall '${whl}'"
        warn=1
      fi
    fi
    # A client older than the tree is the skew that cost real debugging time here:
    # you read a source change that the client you are calling does not contain.
    if [ -n "${installed_mtime}" ] && [ -n "${tree_date}" ] && [ "${tree_date}" -gt "${installed_mtime}" ]; then
      say "  MISMATCH the installed client predates the tree HEAD — source you read may"
      say "           not be in the client you call. Rebuild:"
      say "           cmake --build Build/${CARLA_PRESET} --target carla-python-api-install"
      warn=1
    fi
  else
    say ""
    say "no CARLA checkout offered (set CARLA_UE58_ROOT / CARLA_UE4_ROOT for tree facts)"
  fi

  say ""
  if [ "${warn}" -eq 0 ]; then
    say "No skew detected. Agreement on VERSION is still not agreement on BEHAVIOUR:"
    say "the build flags and the content branch decide as much as the version does."
    say "Run 'support.sh matrix' before assuming a feature works."
  else
    say "Resolve the MISMATCH lines before trusting any measurement taken here."
  fi
}

MODE="${1:-matrix}"; shift 2>/dev/null
case "${MODE}" in
  matrix) cmd_matrix "$@" ;;
  broken) cmd_broken "$@" ;;
  probe)  cmd_probe "$@" ;;
  version) cmd_version "$@" ;;
  *) sed -n '2,10p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
