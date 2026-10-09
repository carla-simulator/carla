#!/usr/bin/env bash
# Get a map into CARLA on UE 5.8. Four routes; see SKILL.md for which to pick.
#
#   bash import_map.sh routes                        explain the four routes for this tree
#   bash import_map.sh opendrive <file.xodr>         Route C: runtime OpenDRIVE world, no build
#   bash import_map.sh plan   --package <Name>       Route B: what the import would do
#   bash import_map.sh import --package <Name>       Route B: run it
#   bash import_map.sh verify --map <MapName>        is the map complete and loadable?
#
# Env: VERTEX_DISTANCE, MAX_ROAD_LENGTH, WALL_HEIGHT, EXTRA_WIDTH  (Route C tuning)
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
set +e

MODE="${1:-routes}"; shift 2>/dev/null || true
PKG=""; MAPNAME=""; XODR=""
while [ $# -gt 0 ]; do
  case "$1" in
    --package) PKG="$2"; shift 2 ;;
    --map)     MAPNAME="$2"; shift 2 ;;
    *)         [ -z "${XODR}" ] && XODR="$1"; shift ;;
  esac
done
[ -n "${CARLA_UE58_ROOT}" ] || { echo "CARLA_UE58_ROOT is not set — run check_env.sh" >&2; exit 2; }
CONTENT="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Content"

case "${MODE}" in

routes)
  cat <<'TXT'
Four routes, and they are not variations of one procedure:

A. Cook a map into a package        needs: map already in Content
   MAPS="Town10HD_Opt,MyTown" bash ../build-carla-ue58/scripts/build.sh configure
   bash ../package-carla-ue58/scripts/package.sh build            (1-2 h)

B. Import FBX + OpenDRIVE           needs: .fbx + .xodr in Import/
   bash import_map.sh import --package MyPackage                  (minutes)
   -> lands at /Game/<Package>/Maps/<Map>, editor-loadable

C. OpenDRIVE only, at runtime       needs: only a .xodr and a running server
   bash import_map.sh opendrive MyTown.xodr                       (seconds)
   -> procedural road mesh, no scenery, not persisted

D. Standalone distributable asset package
   DOES NOT EXIST on ue58-dev. The UE4 flow was
       make package ARGS="--packages=Name"
   and the CMake build has no equivalent target. Its docs
   (tuto_A_create_standalone.md, tuto_M_add_map_package.md) are absent too.
TXT
  echo
  echo "Checked in this tree:"
  grep -q '\-\-packages' "${CARLA_UE58_ROOT}/Unreal/CMakeLists.txt" 2>/dev/null \
    && echo "  --packages: present (unexpected!)" \
    || echo "  --packages target: absent (Route D unavailable)"
  for d in tuto_A_create_standalone.md tuto_M_add_map_package.md tuto_M_add_map_source.md; do
    [ -f "${CARLA_UE58_ROOT}/Docs/${d}" ] && echo "  Docs/${d}: present" || echo "  Docs/${d}: MISSING"
  done
  ;;

opendrive)
  [ -n "${XODR}" ] || { echo "usage: bash import_map.sh opendrive <file.xodr>" >&2; exit 2; }
  [ -f "${XODR}" ] || { echo "ERROR ${XODR} not found" >&2; exit 3; }
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${XODR}" \
      "${VERTEX_DISTANCE:-2.0}" "${MAX_ROAD_LENGTH:-50.0}" \
      "${WALL_HEIGHT:-1.0}" "${EXTRA_WIDTH:-0.6}" <<'PY'
import sys, time
host, port, path = sys.argv[1], int(sys.argv[2]), sys.argv[3]
vd, mrl, wh, ew = (float(x) for x in sys.argv[4:8])
try:
    import carla
except Exception as e:
    sys.exit(f"FAIL cannot import carla ({e})")
xodr = open(path, encoding="utf-8").read()
print(f"xodr: {path} ({len(xodr)} bytes)")
c = carla.Client(host, port); c.set_timeout(120.0)
try:
    print(f"server {c.get_server_version()}")
except Exception as e:
    sys.exit(f"FAIL no server at {host}:{port} ({e}) — Route C needs a running server")
params = carla.OpendriveGenerationParameters(
    vertex_distance=vd, max_road_length=mrl, wall_height=wh,
    additional_width=ew, smooth_junctions=True, enable_mesh_visibility=True)
print(f"generating: vertex_distance={vd} max_road_length={mrl} wall_height={wh} additional_width={ew}")
t0 = time.time()
try:
    w = c.generate_opendrive_world(xodr, params)
except Exception as e:
    sys.exit(f"FAIL generate_opendrive_world: {e}")
m = w.get_map()
print(f"PASS generated in {time.time()-t0:.1f}s")
print(f"PASS map name       : {m.name}")
print(f"PASS spawn points   : {len(m.get_spawn_points())}")
print(f"PASS topology edges : {len(m.get_topology())}")
print(f"PASS actors present : {len(w.get_actors())}")
print("NOTE this world is procedural road only — no buildings, props or scenery,")
print("NOTE and it is not persisted: it exists for this session only.")
PY
  exit $?
  ;;

plan|import)
  [ -n "${PKG}" ] || { echo "usage: bash import_map.sh ${MODE} --package <Name>" >&2; exit 2; }
  IMPORT_DIR="${CARLA_UE58_ROOT}/Import"
  echo "== Import/ contents =="
  if [ -d "${IMPORT_DIR}" ]; then
    find "${IMPORT_DIR}" -maxdepth 2 -type f ! -name 'README*' -printf '  %P (%s bytes)\n' 2>/dev/null | head -20
    N_FBX=$(find "${IMPORT_DIR}" -maxdepth 2 -name '*.fbx' 2>/dev/null | wc -l)
    N_XODR=$(find "${IMPORT_DIR}" -maxdepth 2 -name '*.xodr' 2>/dev/null | wc -l)
    N_JSON=$(find "${IMPORT_DIR}" -maxdepth 2 -name '*.json' 2>/dev/null | wc -l)
    echo "  -> ${N_FBX} .fbx, ${N_XODR} .xodr, ${N_JSON} .json"
    [ "${N_FBX}" -eq 0 ] && echo "  WARNING no .fbx: nothing to import (Route C needs no import at all)"
    # build_binary_for_navigation() skips any map with no "source", and
    # build_binary_for_tm() skips any with no "xodr" — silently, via `continue`.
    [ "${N_XODR}" -eq 0 ] && echo "  WARNING no .xodr: no OpenDRIVE, so no nav mesh and no TM cache"
  else
    echo "  ${IMPORT_DIR} does not exist"
  fi

  echo "== Where it will land =="
  echo "  .umap        Content/${PKG}/Maps/<MapName>       -> /Game/${PKG}/Maps/<MapName>"
  echo "  TM cache     Content/${PKG}/Maps/<MapName>/TM/<MapName>.bin"
  echo "  descriptor   Content/${PKG}/Config/${PKG}.Package.json"
  echo "  NOTE from a PACKAGE, load_world() resolves only /Game/Carla/Maps/<Name>."
  echo "  NOTE  /Game/${PKG}/Maps/... is editor-loadable but NOT package-loadable."

  echo "== Prerequisites =="
  [ -n "${CARLA_UNREAL_ENGINE_PATH}" ] && echo "  PASS engine ${CARLA_UNREAL_ENGINE_PATH}" \
    || echo "  FAIL CARLA_UNREAL_ENGINE_PATH unset — the commandlets need it"
  "${PYTHON}" -c 'import carla' 2>/dev/null \
    && echo "  PASS carla importable (needed for the TM cache)" \
    || echo "  FAIL carla not importable — Import.py imports it at module scope"
  R="${CARLA_UE58_ROOT}/Util/DockerUtils/dist"
  # Check the BINARY, not the directory: the directory always exists (it holds
  # build.sh and the helper scripts) while the binary is NOT shipped on ue58, and
  # build.sh skips its navmesh step silently when it is absent.
  if [ -x "${R}/RecastBuilder" ] && [ -x "${R}/FBX2OBJ" ]; then
    echo "  PASS RecastBuilder + FBX2OBJ in ${R} (nav mesh will be built)"
  else
    echo "  WARN no pedestrian nav mesh will be generated — build.sh needs BOTH"
    echo "       ${R}/FBX2OBJ and ${R}/RecastBuilder, and ue58 ships neither."
    echo "       The import still succeeds and exits 0; walkers just cannot navigate."
    echo "       Fix: cp \"\$(find \"\${CARLA_UE58_ROOT}/Build\" -name RecastBuilder -type f | head -1)\" \"${R}/\""
    echo "            cmake -S Util/DockerUtils/fbx -B /tmp/fbx2obj && cmake --build /tmp/fbx2obj"
  fi

  echo "== The command =="
  echo "  ${PYTHON} ${CARLA_UE58_ROOT}/Util/Tools/Import.py --package ${PKG}"
  echo "  NOT Util/Tools/Import.sh — it is broken on this branch:"
  echo "    Environment.sh never sets CARLA_BUILD_TOOLS_FOLDER, so it runs"
  echo "    'python3 /Import.py' and exits 2 (Environment.sh has set -e)."

  if [ "${MODE}" = "plan" ]; then
    echo
    echo "(plan only — nothing changed. Re-run with 'import' to execute.)"
    exit 0
  fi

  echo "== Importing =="
  cd "${CARLA_UE58_ROOT}" || exit 3
  "${PYTHON}" Util/Tools/Import.py --package "${PKG}"
  RC=$?
  echo "[import] Import.py exited ${RC}"
  # Import.py uses subprocess.call for the POSIX commandlet path and never checks
  # the result, so a failed commandlet does not surface here.
  echo "[import] NOTE Import.py does not check commandlet exit codes — verify the result:"
  echo "[import]   bash import_map.sh verify --map <MapName>"
  exit ${RC}
  ;;

verify)
  [ -n "${MAPNAME}" ] || { echo "usage: bash import_map.sh verify --map <MapName>" >&2; exit 2; }
  rc=0
  ok(){ echo "  PASS $*"; }
  warn(){ echo "  WARN $*"; }
  bad(){ echo "  FAIL $*"; rc=1; }

  echo "== Assets in Content =="
  UMAP="$(find "${CONTENT}" -name "${MAPNAME}.umap" -print -quit 2>/dev/null)"
  if [ -n "${UMAP}" ]; then
    REL="${UMAP#${CONTENT}/}"
    ok "${MAPNAME}.umap at Content/${REL}"
    PKGPATH="/Game/$(dirname "${REL}")/${MAPNAME}"
    ok "package path ${PKGPATH}"
    # FindMapPath's pak fallback is hardcoded to /Game/Carla/Maps/<Name>.
    case "${PKGPATH}" in
      /Game/Carla/Maps/${MAPNAME}) ok "loadable from a PACKAGE (matches the /Game/Carla/Maps/<Name> fallback)" ;;
      *) warn "NOT loadable from a package: FindMapPath's pak fallback only checks"
         warn "  /Game/Carla/Maps/${MAPNAME}. Editor and -game mode are fine." ;;
    esac
  else
    bad "no ${MAPNAME}.umap anywhere under Content/"
  fi

  X="$(find "${CONTENT}" -name "${MAPNAME}.xodr" -print -quit 2>/dev/null)"
  [ -n "${X}" ] && ok "OpenDRIVE at Content/${X#${CONTENT}/}" \
    || warn "no ${MAPNAME}.xodr — no road network, so no waypoints or routing"
  T="$(find "${CONTENT}" -path "*TM*" -name "${MAPNAME}.bin" -print -quit 2>/dev/null)"
  [ -n "${T}" ] && ok "TM cache at Content/${T#${CONTENT}/}" \
    || warn "no TM cache — Traffic Manager will have no precomputed map"
  N="$(find "${CONTENT}" -path "*Nav*" -name "${MAPNAME}*.bin" -print -quit 2>/dev/null)"
  if [ -n "${N}" ]; then
    ok "nav mesh at Content/${N#${CONTENT}/}"
  elif [ -n "${UMAP}" ] && [ -d "${CONTENT}/Carla/__ExternalActors__/Carla/Maps/${MAPNAME}" ]; then
    # A World Partition map keeps its actors as one-file-per-actor packages under
    # Content/Carla/__ExternalActors__/ and streams its navmesh from the cells, so
    # it ships no Nav/*.bin — Town12 is like this and walkers DO spawn there.
    ok "no Nav/*.bin, but this is a World Partition map (OFPA packages present):"
    ok "  the navmesh streams from the cells. Needs the OFPA mount patch to load."
  else
    warn "no nav mesh — walkers cannot spawn (Town_C has this same gap)"
  fi
  D="$(find "${CONTENT}" -name '*.Package.json' 2>/dev/null | xargs -r grep -l "\"${MAPNAME}\"" 2>/dev/null | head -1)"
  [ -n "${D}" ] && ok "declared in Content/${D#${CONTENT}/}" \
    || warn "not named in any .Package.json descriptor"

  echo "== Staging (only matters for a package build) =="
  if [ -n "${UMAP}" ]; then
    SUB="$(dirname "${UMAP#${CONTENT}/}")"
    case "${SUB}" in
      Carla/Maps)
        ok "road data would come from the shared Carla/Maps/{OpenDrive,TM} dirs,"
        ok "  both of which ARE in DirectoriesToAlwaysStageAsUFS" ;;
      *)
        warn "this map's road data lives under ${SUB}/, a per-map subdirectory."
        warn "  DirectoriesToAlwaysStageAsUFS lists Carla/Maps/{OpenDrive,Nav,TM},"
        warn "  Carla/Maps/Town15/*, Carla/Config — not this path. Add an entry to"
        warn "  Unreal/CarlaUnreal/Config/DefaultGame.ini or the package ships without it." ;;
    esac
  fi

  echo "== Live server =="
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${MAPNAME}" <<'PY'
import sys
host, port, name = sys.argv[1], int(sys.argv[2]), sys.argv[3]
try:
    import carla
    c = carla.Client(host, port); c.set_timeout(10.0)
    c.get_server_version()
except Exception:
    print("  INFO no server running — skipped the load test")
    sys.exit(0)
maps = [m.split('/')[-1] for m in c.get_available_maps()]
print(f"  {'PASS' if name in maps else 'WARN'} get_available_maps(): "
      f"{name}{'' if name in maps else ' NOT'} listed ({len(maps)} maps)")
if not maps:
    print("  INFO an empty list means a packaged server: discovery cannot see inside a .pak")
PY
  echo "== Result =="
  [ "$rc" -eq 0 ] && echo "  map looks complete (warnings are non-blocking)" || echo "  PROBLEMS — see FAIL lines"
  exit $rc
  ;;

*)
  echo "usage: bash import_map.sh {routes|opendrive <file.xodr>|plan|import|verify}" >&2
  exit 2
  ;;
esac
