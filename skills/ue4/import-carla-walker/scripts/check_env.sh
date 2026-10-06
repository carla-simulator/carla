#!/usr/bin/env bash
# Prerequisite checks for import-carla-walker. Read-only, no sudo.
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

# --- The GEN3 asset contract ------------------------------------------------
# Without these four assets there is nothing to bind the mesh to, nothing to
# animate it, nothing to duplicate and nowhere to register it. Each is fatal
# because no step of this skill can create them.

SKEL_FILE="$(game_to_disk "${CARLA_WALKER_SKELETON}")"
if [ -f "${SKEL_FILE}" ]; then
  pass "GEN3 skeleton present (${CARLA_WALKER_SKELETON##*/})"
else
  fail "no GEN3 skeleton at ${SKEL_FILE} — content is missing or out of date (build-carla-ue4 fetches it)"
fi

ABP_FILE="$(game_to_disk "${CARLA_WALKER_ANIM_BP}")"
if [ -f "${ABP_FILE}" ]; then
  pass "GEN3 anim blueprint present (${CARLA_WALKER_ANIM_BP##*/})"
else
  fail "no GEN3 anim blueprint at ${ABP_FILE} — an imported walker would never animate"
fi

DONOR_FILE="$(game_to_disk "${CARLA_WALKER_DONOR_BP}")"
if [ -f "${DONOR_FILE}" ]; then
  pass "donor blueprint present (${CARLA_WALKER_DONOR_BP##*/})"
else
  fail "no donor blueprint at ${DONOR_FILE} — pass --donor with a GEN3 walker BP that exists"
fi

FACTORY_FILE="$(game_to_disk "${CARLA_WALKER_FACTORY}")"
if [ -f "${FACTORY_FILE}" ]; then
  # Ids are readable straight off the asset, which gives the user a count and lets an
  # explicit --id be validated without booting the editor.
  # Two encodings: a text blob (list still a function local) or serialised FStrings
  # (list promoted to a member variable). See the references, C1.
  IDS="$(python3 - "${FACTORY_FILE}" <<'PYEOF' 2>/dev/null
import re, struct, sys
data = open(sys.argv[1], "rb").read()
literal = set(m.decode() for m in re.findall(rb'\(Id="(\d{1,8})"', data))
serial = set()
for m in re.finditer(rb'(?=(.{4})(\d{4}\x00))', data, re.S):
    if struct.unpack("<i", m.group(1))[0] == 5:
        serial.add(m.group(2)[:-1].decode())
print("\n".join(sorted(literal if len(literal) >= len(serial) else serial)))
PYEOF
)"
  COUNT="$(echo "${IDS}" | grep -c . || true)"
  if [ "${COUNT}" -gt 0 ]; then
    # 10# forces base 10: ids are zero-padded, and $((0052)) is OCTAL 42 in bash.
    MAX_ID="$(echo "${IDS}" | sort -n | tail -1)"
    NEXT="$(printf '%04d' "$(( 10#${MAX_ID} + 1 ))")"
    pass "WalkerFactory readable — ${COUNT} walker(s) registered, next free id ${NEXT}"
  else
    warn "no ids readable from ${FACTORY_FILE##*/} — normal once the array is a member variable (the encoding changes); the editor allocates the id instead"
  fi
  # Registration appends to a blueprint MEMBER variable. Stock content keeps the list
  # in a function LOCAL, which reflection cannot reach (C1) — the member name has to
  # appear in the package for the automatic path to work.
  if grep -aq "${CARLA_WALKER_FACTORY_ARRAY}" "${FACTORY_FILE}"; then
    pass "factory array '${CARLA_WALKER_FACTORY_ARRAY}' present — registration can be automatic"
  else
    warn "no member variable '${CARLA_WALKER_FACTORY_ARRAY}' in WalkerFactory — the list is probably still a function local (C1); the import will skip registration and print a paste-ready entry"
  fi
else
  fail "no WalkerFactory at ${FACTORY_FILE} — nothing to register a walker in"
fi

# --- Warnings: recoverable, but cheaper to know now -------------------------

if [ -d "${CONTENT_DIR}/Carla" ]; then
  pass "Content/Carla present"
  # Commonly a symlink to one content clone shared by every worktree. This skill
  # writes the mesh, the blueprint AND the factory entry into it, so an import
  # becomes visible from every checkout that links it — and there is no
  # --package escape hatch here: WalkerFactory is shared content by nature.
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

# The GEN3 walkers ship with a reference to an animation under a folder that is
# absent from some content drops. Harmless (ABP_GEN3 drives the walker) but it
# fills the editor log with linker warnings, and a duplicated donor inherits it —
# which is why build_walker.py clears that slot.
NOS_DIR="${CONTENT_DIR}/Carla/Static/Pedestrian/Animations/GEN3/Nos_"
if [ -d "${NOS_DIR}" ]; then
  pass "GEN3 Nos_/ animation folder present"
else
  warn "no ${NOS_DIR#"${CONTENT_DIR}"/} — stock GEN3 walkers log 'Failed to load AS_walkingG3'; the import clears that slot in the copy"
fi

# Only verification needs the client wheel; importing does not.
if [ -n "${CARLA_PY_VERSION:-}" ]; then PYBIN="python${CARLA_PY_VERSION}"; else PYBIN="python3"; fi
PYPATH="$(command -v "${PYBIN}" 2>/dev/null || true)"
if [ -z "${PYPATH}" ]; then
  warn "no '${PYBIN}' on PATH — importing still works; verify_walker.py will not"
elif "${PYPATH}" -c 'import carla' 2>/dev/null; then
  pass "client python: ${PYPATH} (imports carla) — verification available"
else
  warn "${PYPATH} cannot 'import carla' — importing still works, but verify_walker.py needs the wheel"
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
