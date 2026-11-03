#!/usr/bin/env bash
# Prerequisite checks for import-carla-vehicle. Read-only, no sudo.
# Exits non-zero ONLY on hard blockers; WARN means a later step handles it.
# Run by the MCP check_prerequisites(name) tool.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

# /Game/X -> Content/X.uasset
game_to_disk() {
  echo "${CONTENT_DIR}/${1#/Game/}.uasset"
}

# --- Hard blockers ----------------------------------------------------------
# This skill drives UE4Editor twice. What must exist is a runnable editor for
# THIS project; building any of it is the build-carla-ue4 skill's job.

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

if [ -f "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Binaries/Linux/libUE4Editor-CarlaUE4.so" ]; then
  pass "CarlaUE4Editor built"
else
  fail "CarlaUE4Editor not built — run the build-carla-ue4 skill against ${CARLA_UE4_ROOT}"
fi

if [ -f "${UPROJECT}" ] && grep -q "PythonScriptPlugin" "${UPROJECT}"; then
  pass "PythonScriptPlugin enabled in CarlaUE4.uproject"
else
  fail "PythonScriptPlugin not enabled in CarlaUE4.uproject — neither editor boot can run"
fi

# --- The vehicle asset contract ---------------------------------------------
# Assembly is done by CarlaTools' VehicleAuthoringLibrary (CARLA PR #9805). Without it
# there is no scripted way to build wheel bodies or wheel setups, so its absence is
# fatal rather than a warning.
CARLATOOLS_SO="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/CarlaTools/Binaries/Linux/libUE4Editor-CarlaTools.so"
if [ -f "${CARLATOOLS_SO}" ]; then
  if grep -aq "VehicleAuthoringLibrary" "${CARLATOOLS_SO}"; then
    pass "CarlaTools exposes VehicleAuthoringLibrary"
  else
    fail "CarlaTools is built WITHOUT VehicleAuthoringLibrary (CARLA PR #9805) — vehicle assembly cannot run; rebuild against a checkout that has it"
  fi
else
  fail "CarlaTools not built (no ${CARLATOOLS_SO##*/}) — run the build-carla-ue4 skill"
fi

# Registration is CARLA-side tooling, next to the prop and walker equivalents.
REG_SCRIPT="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/add_vehicle_to_vehicle_factory.py"
if [ -f "${REG_SCRIPT}" ]; then
  pass "add_vehicle_to_vehicle_factory.py present"
else
  fail "no ${REG_SCRIPT##*/} in this checkout — registration has nothing to call"
fi

DONOR_FILE="$(game_to_disk "${CARLA_VEHICLE_DONOR_BP}")"
if [ -f "${DONOR_FILE}" ]; then
  pass "donor vehicle blueprint present (${CARLA_VEHICLE_DONOR_BP##*/})"
else
  fail "no donor vehicle blueprint at ${DONOR_FILE} — pass --donor with one that exists"
fi

ANIM_FILE="$(game_to_disk "${CARLA_VEHICLE_DONOR_ANIM_BP}")"
if [ -f "${ANIM_FILE}" ]; then
  pass "donor anim blueprint present (${CARLA_VEHICLE_DONOR_ANIM_BP##*/})"
else
  fail "no donor anim blueprint at ${ANIM_FILE} — the vehicle would have no wheel animation"
fi

MISSING_WHEELS=0
IFS=',' read -ra WHEELS <<< "${CARLA_VEHICLE_DONOR_WHEELS}"
for w in "${WHEELS[@]}"; do
  [ -f "$(game_to_disk "${w}")" ] || { MISSING_WHEELS=$((MISSING_WHEELS+1)); }
done
if [ "${#WHEELS[@]}" -eq 4 ] && [ "${MISSING_WHEELS}" -eq 0 ]; then
  pass "4 donor wheel blueprints present"
else
  fail "${MISSING_WHEELS} of ${#WHEELS[@]} donor wheel blueprints missing — set CARLA_VEHICLE_DONOR_WHEELS"
fi

FACTORY_FILE="$(game_to_disk "${CARLA_VEHICLE_FACTORY}")"
if [ -f "${FACTORY_FILE}" ]; then
  if grep -aq "${CARLA_VEHICLE_FACTORY_ARRAY}" "${FACTORY_FILE}"; then
    COUNT="$(grep -aoE 'BP_[A-Za-z0-9_]+_C' "${FACTORY_FILE}" | sort -u | grep -c . || true)"
    pass "VehicleFactory readable — '${CARLA_VEHICLE_FACTORY_ARRAY}' member present, ~${COUNT} vehicle blueprint(s) referenced"
  else
    fail "VehicleFactory has no '${CARLA_VEHICLE_FACTORY_ARRAY}' member variable — registration cannot append"
  fi
else
  fail "no VehicleFactory at ${FACTORY_FILE} — nothing to register a vehicle in"
fi

# --- Warnings: recoverable, but cheaper to know now -------------------------

if [ -d "${CONTENT_DIR}/Carla" ]; then
  pass "Content/Carla present"
  # Commonly a symlink to one content clone shared by every worktree. This skill
  # writes the mesh, the blueprints AND the factory entry into it, so an import
  # becomes visible from every checkout that links it: VehicleFactory is shared content.
  if [ -L "${CONTENT_DIR}/Carla" ]; then
    TARGET="$(readlink -f "${CONTENT_DIR}/Carla")"
    case "${TARGET}" in
      "${CARLA_UE4_ROOT}"/*) : ;;
      *) warn "Content/Carla -> ${TARGET} (shared — an imported walker appears in every checkout linking it)" ;;
    esac
  fi
else
  warn "Content/Carla missing — there is nothing to import into (build-carla-ue4 fetches content)"
fi

# Only verification needs the client wheel; importing does not.
if [ -n "${CARLA_PY_VERSION:-}" ]; then PYBIN="python${CARLA_PY_VERSION}"; else PYBIN="python3"; fi
PYPATH="$(command -v "${PYBIN}" 2>/dev/null || true)"
if [ -z "${PYPATH}" ]; then
  warn "no '${PYBIN}' on PATH — importing still works; verify_vehicle.py will not"
elif "${PYPATH}" -c 'import carla' 2>/dev/null; then
  pass "client python: ${PYPATH} (imports carla) — verification available"
else
  warn "${PYPATH} cannot 'import carla' — importing still works, but verify_vehicle.py needs the wheel"
fi

# --- Disk -------------------------------------------------------------------
AVAIL_GB=$(df -BG --output=avail "${CARLA_UE4_ROOT}" 2>/dev/null | tail -1 | tr -dc '0-9')
if [ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -ge 5 ]; then
  pass "disk: ${AVAIL_GB} GB free"
else
  warn "disk: only ${AVAIL_GB:-?} GB free"
fi

echo
[ "${FAIL}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                    || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${FAIL}"
