#!/usr/bin/env bash
# Prerequisite checks for import-carla-map. Read-only, no sudo.
# Exits non-zero ONLY on hard blockers; WARN means a later step handles it.
# Run by the MCP check_prerequisites(name) tool.
#
# Usage: check_env.sh [map-dir]
#   map-dir  optional: the directory holding the map, checked for a usable
#            .fbx/.xodr pair.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAP_DIR="${1:-}"
# Capture an EXPLICIT caller pin; env.sh sets no CARLA_PY_VERSION default, so the
# active `python3` is what we check unless the caller pinned one.
_PYV_PIN="${CARLA_PY_VERSION:-}"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
CARLA_PY_VERSION="${_PYV_PIN}"

FAIL=0
pass() { echo "PASS  $*"; }
warn() { echo "WARN  $*"; }
fail() { echo "FAIL  $*"; FAIL=1; }

CONTENT_DIR="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content"
UPROJECT="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/CarlaUE4.uproject"
IMPORT_DIR="${CARLA_UE4_ROOT}/Import"
FBX2OBJ="${CARLA_UE4_ROOT}/Util/DockerUtils/dist/FBX2OBJ"

# --- Hard blockers ----------------------------------------------------------
# What must exist is a runnable editor for THIS project plus a carla-capable
# python. Building any of it belongs to the build-carla-ue4 skill: this one
# checks and defers, it never builds.

if [ -z "${UE4_ROOT}" ]; then
  fail "UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork"
elif [ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ]; then
  pass "UE4 built at ${UE4_ROOT}"
else
  fail "UE4 not built at ${UE4_ROOT} (no Engine/Binaries/Linux/UE4Editor) — run the build-carla-ue4 skill"
fi

if [ -z "${CARLA_UE4_ROOT}" ]; then
  fail "CARLA_UE4_ROOT is unset — export it, or run from inside a carla checkout"
elif [ -f "${UPROJECT}" ]; then
  pass "carla checkout at ${CARLA_UE4_ROOT}"
else
  fail "no ${UPROJECT} — CARLA_UE4_ROOT is wrong"
fi

# The editor commandlets (ImportAssets, PrepareAssetsForCooking, ...) load the
# project's editor modules. This skill has no build step to produce them, so a
# missing editor binary is fatal rather than merely slow.
if [ -f "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Binaries/Linux/libUE4Editor-CarlaUE4.so" ]; then
  pass "CarlaUE4Editor built"
else
  fail "CarlaUE4Editor not built — run the build-carla-ue4 skill against ${CARLA_UE4_ROOT}"
fi

# NOTE: no PythonScriptPlugin check here, unlike import-carla-prop. That skill
# drives the editor through `-run=pythonscript` and genuinely needs the plugin.
# Map import does not: Import.py's invoke_commandlet only ever runs the C++
# commandlets (ImportAssets, MoveAssets, PrepareAssetsForCooking,
# LoadAssetMaterials). Checking for it here failed checkouts that import fine.

# HARD, unlike prop import: Import.py `import carla` at module scope, then calls
# carla.Map(...).cook_in_memory_map(...) for the TM binary. No wheel -> the
# import aborts on Import.py's first line before the editor even boots.
if [ -n "${CARLA_PY_VERSION:-}" ]; then PYBIN="python${CARLA_PY_VERSION}"; else PYBIN="python3"; fi
PYPATH="$(command -v "${PYBIN}" 2>/dev/null || true)"
if [ -z "${PYPATH}" ]; then
  fail "no '${PYBIN}' on PATH — nothing can run Import.py; activate your CARLA client env"
elif "${PYPATH}" -c 'import carla' 2>/dev/null; then
  pass "client python: ${PYPATH} (imports carla)"
else
  fail "${PYPATH} cannot 'import carla' — Import.py aborts on its first line; activate the env with the wheel (build-carla-ue4 step 04)"
fi

# --- Warnings: recoverable, but cheaper to know now -------------------------

# Where the map will actually land. Two levels can be symlinked to a content
# clone shared by every worktree, and they mean different things:
#   Content       -> shared : EVERY package, including yours, lands in the clone.
#   Content/Carla -> shared : only the stock package is shared; a --package NAME
#                             import lands locally, which is the point of naming it.
if [ ! -d "${CONTENT_DIR}" ] && [ ! -L "${CONTENT_DIR}" ]; then
  warn "no ${CONTENT_DIR} — build-carla-ue4 fetches content; the import has nowhere to land"
elif [ -L "${CONTENT_DIR}" ]; then
  TARGET="$(readlink -f "${CONTENT_DIR}")"
  case "${TARGET}" in
    "${CARLA_UE4_ROOT}"/*) pass "Content -> ${TARGET} (inside this checkout)" ;;
    *) warn "Content -> ${TARGET} (shared with other checkouts — the imported map lands there and every one of them sees it)" ;;
  esac
elif [ -L "${CONTENT_DIR}/Carla" ]; then
  TARGET="$(readlink -f "${CONTENT_DIR}/Carla")"
  case "${TARGET}" in
    "${CARLA_UE4_ROOT}"/*) pass "Content/Carla -> ${TARGET} (inside this checkout)" ;;
    *) pass "Content/Carla -> ${TARGET} (shared clone; a --package NAME import stays local — do NOT import into 'Carla')" ;;
  esac
else
  pass "Content is a real directory in this checkout"
fi

# Import.py copies the .fbx/.xodr into dist/ for the navmesh stage and removes
# them afterwards. An interrupted run leaves them behind, and the NEXT navmesh
# build can pick up the stale geometry (the dist/ analogue of M2).
LEFTOVERS="$(find "${CARLA_UE4_ROOT}/Util/DockerUtils/dist" -maxdepth 1 \
  \( -name '*.fbx' -o -name '*.xodr' -o -name '*.obj' -o -name '*.bin' \) 2>/dev/null | head -3)"
if [ -n "${LEFTOVERS}" ]; then
  warn "leftover geometry in Util/DockerUtils/dist from an interrupted import: $(echo "${LEFTOVERS}" | tr '\n' ' ')— remove it before rebuilding a navmesh"
fi

# Nav/<map>.bin needs both halves of the chain: FBX2OBJ (not shipped; the stock
# 'make build.utils' can no longer install it) and RecastBuilder (ships in
# dist/). Missing either means the map imports and drives but walkers cannot
# navigate it. See references/maps.md.
RECAST="${CARLA_UE4_ROOT}/Util/DockerUtils/dist/RecastBuilder"
if [ -x "${FBX2OBJ}" ]; then
  pass "FBX2OBJ present — pedestrian navmesh can be generated"
elif command -v "${BLENDER:-blender}" >/dev/null 2>&1; then
  warn "FBX2OBJ missing (${FBX2OBJ}) — no pedestrian navmesh. Install it: bash ${HERE}/install_fbx2obj.sh"
else
  warn "FBX2OBJ missing (${FBX2OBJ}) and blender not found — no pedestrian navmesh. Install Blender (or set BLENDER), then run install_fbx2obj.sh"
fi

if [ ! -x "${RECAST}" ]; then
  warn "RecastBuilder missing or not executable (${RECAST}) — no pedestrian navmesh even with FBX2OBJ present"
fi

# --- The map the caller named ------------------------------------------------
# Checked only when a directory is passed.
if [ -n "${MAP_DIR}" ]; then
  if [ ! -d "${MAP_DIR}" ]; then
    fail "no such map directory: ${MAP_DIR}"
  else
    N_XODR="$(find "${MAP_DIR}" -maxdepth 1 -name '*.xodr' 2>/dev/null | wc -l)"
    N_FBX="$(find "${MAP_DIR}" -maxdepth 1 -name '*.fbx' 2>/dev/null | wc -l)"
    if [ "${N_XODR}" -eq 1 ] && [ "${N_FBX}" -ge 1 ]; then
      pass "map source: ${N_FBX} .fbx + 1 .xodr in ${MAP_DIR}"
    elif [ "${N_XODR}" -eq 0 ]; then
      fail "no .xodr in ${MAP_DIR} — a CARLA map needs its OpenDRIVE beside the mesh"
    elif [ "${N_XODR}" -gt 1 ]; then
      fail "${N_XODR} .xodr files in ${MAP_DIR} — import one map at a time"
    else
      fail "no .fbx in ${MAP_DIR}"
    fi
  fi
fi

# Import.py walks Import/ for package jsons and imports every one it finds.
# Only jsons are discovered there; loose .fbx/.xodr are not.
if [ -d "${IMPORT_DIR}" ]; then
  STRAY="$(find "${IMPORT_DIR}" -name '*.json' ! -name 'roadpainter_decals.json' 2>/dev/null | head -1)"
  if [ -n "${STRAY}" ]; then
    warn "package json already under Import/ (${STRAY#"${CARLA_UE4_ROOT}"/}) — Import.py imports that content package too; move it out to import one map alone"
  else
    pass "no other package json under Import/"
  fi
fi

# --- Disk -------------------------------------------------------------------
AVAIL_GB=$(df -BG --output=avail "${CARLA_UE4_ROOT}" 2>/dev/null | tail -1 | tr -dc '0-9')
if [ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -ge 10 ]; then
  pass "disk: ${AVAIL_GB} GB free"
else
  warn "disk: only ${AVAIL_GB:-?} GB free — a map import writes cooked tiles into Content/"
fi

echo
[ "${FAIL}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                    || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${FAIL}"
