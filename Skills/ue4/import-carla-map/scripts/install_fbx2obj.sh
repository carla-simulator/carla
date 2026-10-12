#!/usr/bin/env bash
# Install a Blender-backed FBX2OBJ into <carla>/Util/DockerUtils/dist/.
#
# Replaces `make build.utils`, whose Autodesk FBX SDK download now 403s (see
# references/maps.md). Writes two files into dist/ -- gitignored, exactly as the
# compiled FBX2OBJ used to be:
#
#   FBX2OBJ            shim with the original CLI: FBX2OBJ <in.fbx> <out.obj>
#   fbx2obj_blender.py the converter it drives
#
# Keeping the name and CLI means build.sh and Import.py pick it up unchanged, so
# importing a map produces the navmesh again.
#
# Usage:  bash install_fbx2obj.sh            # uses CARLA_UE4_ROOT from env.sh
#         BLENDER=/path/to/blender bash install_fbx2obj.sh
#         bash install_fbx2obj.sh --force    # replace an existing FBX2OBJ.orig backup

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${HERE}/env.sh"

DIST="${CARLA_UE4_ROOT}/Util/DockerUtils/dist"
BLENDER_BIN="${BLENDER:-blender}"
FORCE=0
[ "${1:-}" = "--force" ] && FORCE=1

# Marker identifying a shim we installed, so a real FBX2OBJ is never silently
# replaced (and so re-running this is idempotent).
SHIM_MARKER="carla-agentic-tools skills/import-carla-map"

if [ -z "${CARLA_UE4_ROOT}" ] || [ ! -d "${DIST}" ]; then
  echo "[install-fbx2obj] ERROR: dist folder not found: ${DIST}" >&2
  echo "[install-fbx2obj]        export CARLA_UE4_ROOT to a CARLA checkout." >&2
  exit 1
fi

if ! command -v "${BLENDER_BIN}" >/dev/null 2>&1; then
  echo "[install-fbx2obj] ERROR: blender not found (tried '${BLENDER_BIN}')." >&2
  echo "[install-fbx2obj]        Install Blender or set BLENDER=/path/to/blender." >&2
  exit 1
fi
# Resolve it now and bake the absolute path into the shim: the import runs
# FBX2OBJ without BLENDER exported, so a shim that looked it up at run time
# would work at install time and silently produce no navmesh later.
BLENDER_ABS="$(command -v "${BLENDER_BIN}")"

# Never clobber a real, SDK-compiled FBX2OBJ without keeping it.
if [ -e "${DIST}/FBX2OBJ" ] && ! grep -q "${SHIM_MARKER}" "${DIST}/FBX2OBJ" 2>/dev/null; then
  if [ -e "${DIST}/FBX2OBJ.orig" ] && [ "${FORCE}" -eq 0 ]; then
    echo "[install-fbx2obj] ERROR: ${DIST}/FBX2OBJ is not our shim, and" >&2
    echo "[install-fbx2obj]        FBX2OBJ.orig already exists — refusing to" >&2
    echo "[install-fbx2obj]        overwrite the backup. Move one aside, or --force." >&2
    exit 1
  fi
  mv "${DIST}/FBX2OBJ" "${DIST}/FBX2OBJ.orig" || exit 1
  echo "[install-fbx2obj] existing FBX2OBJ was not ours — kept it as FBX2OBJ.orig"
fi

cp "${HERE}/fbx2obj_blender.py" "${DIST}/fbx2obj_blender.py" || exit 1

cat > "${DIST}/FBX2OBJ" <<SHIM
#!/usr/bin/env bash
# ${SHIM_MARKER}: Blender-backed stand-in for the FBX SDK build of FBX2OBJ.
BLENDER_DEFAULT="${BLENDER_ABS}"
SHIM
cat >> "${DIST}/FBX2OBJ" <<'SHIM'
# CLI is unchanged:  FBX2OBJ <in.fbx> <out.obj>
set -uo pipefail

# build.sh exports LD_LIBRARY_PATH=./ so the real binary finds libfbxsdk.so in
# dist/. That poisons Blender's own library resolution, so drop it.
unset LD_LIBRARY_PATH

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# BLENDER_DEFAULT is the absolute path resolved when this shim was installed;
# the import does not export BLENDER, so the lookup cannot be deferred.
BLENDER_BIN="${BLENDER:-${BLENDER_DEFAULT}}"

if [ $# -lt 2 ]; then
  echo "FBX2OBJ: usage: FBX2OBJ <in.fbx> <out.obj>" >&2
  exit 2
fi
OUT="$2"

if ! command -v "${BLENDER_BIN}" >/dev/null 2>&1; then
  echo "FBX2OBJ: ERROR blender not found (tried '${BLENDER_BIN}')" >&2
  exit 1
fi

"${BLENDER_BIN}" --background --python "${HERE}/fbx2obj_blender.py" -- "$@"

# Blender does not reliably propagate a script's sys.exit code, so verify the
# artifact instead. An empty OBJ is the exact failure mode this replaces.
if [ ! -s "${OUT}" ]; then
  echo "FBX2OBJ: ERROR no OBJ produced (or it is empty): ${OUT}" >&2
  exit 1
fi
exit 0
SHIM

chmod +x "${DIST}/FBX2OBJ" || exit 1

echo "[install-fbx2obj] installed:"
echo "  ${DIST}/FBX2OBJ"
echo "  ${DIST}/fbx2obj_blender.py"
echo "[install-fbx2obj] blender: ${BLENDER_ABS} ($(${BLENDER_ABS} --version 2>/dev/null | head -1))"

# Smoke-test the whole chain now, on a cube Blender makes itself. Every failure
# mode this script exists to prevent -- a Blender too old for wm.obj_export, a
# path that does not resolve at import time, an empty OBJ -- is silent at
# navmesh time and obvious here.
SMOKE="$(mktemp -d)"
trap 'rm -rf "${SMOKE}"' EXIT
cat > "${SMOKE}/make_cube.py" <<'PY'
import bpy, sys
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.mesh.primitive_cube_add()
bpy.context.active_object.name = "Roads_Sidewalk_smoke"
bpy.ops.export_scene.fbx(filepath=sys.argv[sys.argv.index("--") + 1])
PY
if "${BLENDER_ABS}" --background --python "${SMOKE}/make_cube.py" -- "${SMOKE}/smoke.fbx" \
     >"${SMOKE}/log" 2>&1 && [ -s "${SMOKE}/smoke.fbx" ]; then
  if "${DIST}/FBX2OBJ" "${SMOKE}/smoke.fbx" "${SMOKE}/smoke.obj" >>"${SMOKE}/log" 2>&1 \
       && [ -s "${SMOKE}/smoke.obj" ]; then
    echo "[install-fbx2obj] smoke test: OK (cube FBX -> $(wc -l <"${SMOKE}/smoke.obj") line OBJ)"
  else
    echo "[install-fbx2obj] ERROR: the installed FBX2OBJ failed on a test cube." >&2
    tail -20 "${SMOKE}/log" >&2
    exit 1
  fi
else
  echo "[install-fbx2obj] WARNING: could not build a test FBX with this Blender;" >&2
  echo "[install-fbx2obj]          skipping the smoke test. Verify the first" >&2
  echo "[install-fbx2obj]          navmesh build carefully." >&2
fi

echo "[install-fbx2obj] map import will now build Nav/<map>.bin again."
