#!/usr/bin/env bash
# Prerequisite checks for import-carla-prop. Read-only, no sudo.
# Exits non-zero ONLY on hard blockers; WARN means a later step handles it.
# Run by the MCP check_prerequisites(name) tool.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

# --- Hard blockers ----------------------------------------------------------
# The importer drives UE4Editor directly, so what must exist is a runnable
# editor for THIS project — not a Python environment.
#
# Building any of it belongs to the build-carla-ue4 skill: this one checks and
# defers, it never builds. Every FAIL below names that skill rather than a
# command to run here.

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

# The editor commandlet loads the project's own editor modules. There is no
# build step in this skill to produce them, so a missing editor binary is fatal
# rather than merely slow.
if [ -f "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Binaries/Linux/libUE4Editor-CarlaUE4.so" ]; then
  pass "CarlaUE4Editor built"
else
  fail "CarlaUE4Editor not built — run the build-carla-ue4 skill against ${CARLA_UE4_ROOT}"
fi

# Registration runs as editor Python (-run=pythonscript). Without the plugin the
# commandlet reports '-Script argument not specified' or runs as a stub.
if [ -f "${UPROJECT}" ] && grep -q "PythonScriptPlugin" "${UPROJECT}"; then
  pass "PythonScriptPlugin enabled in CarlaUE4.uproject"
else
  fail "PythonScriptPlugin not enabled in CarlaUE4.uproject — the importer cannot register the prop"
fi

# --- Warnings: recoverable, but cheaper to know now -------------------------

# The default (stock) route imports into Content/Carla. --package NAME does not
# need it, which is why this warns rather than blocks.
if [ -d "${CONTENT_DIR}/Carla" ]; then
  pass "Content/Carla present"

  # Very commonly a symlink to one content clone shared by every worktree. The
  # stock route writes the mesh and Default.Package.json into it, so the prop
  # becomes visible from all of them at once.
  if [ -L "${CONTENT_DIR}/Carla" ]; then
    TARGET="$(readlink -f "${CONTENT_DIR}/Carla")"
    case "${TARGET}" in
      "${CARLA_UE4_ROOT}"/*) : ;;
      *) warn "Content/Carla -> ${TARGET} (shared with other checkouts — imports land there too)" ;;
    esac
  fi
else
  warn "Content/Carla missing — the default route needs it (build-carla-ue4 fetches content); --package NAME does not"
fi

# The registry file the default route appends to.
DEFAULT_PKG="${CONTENT_DIR}/Carla/Config/Default.Package.json"
if [ -f "${DEFAULT_PKG}" ]; then
  COUNT="$(python3 -c "import json,sys;print(len(json.load(open(sys.argv[1])).get('props',[])))" "${DEFAULT_PKG}" 2>/dev/null)"
  if [ -n "${COUNT}" ]; then
    pass "Default.Package.json parses — ${COUNT} prop(s) registered"
  else
    fail "${DEFAULT_PKG} is not valid JSON — the importer would rewrite a broken file"
  fi
elif [ -d "${CONTENT_DIR}/Carla" ]; then
  warn "no Default.Package.json yet — the importer creates it"
fi

# Every *.Package.json under Content/ is read at map load. One that does not
# parse, or has no "props" array, only logs a JSON warning and contributes
# nothing — but it is noise in exactly the log you would be reading on a failure.
if [ -d "${CONTENT_DIR}" ]; then
  while IFS= read -r j; do
    [ -n "${j}" ] || continue
    if ! python3 -c "import json,sys;d=json.load(open(sys.argv[1]));sys.exit(0 if 'props' in d else 3)" "${j}" 2>/dev/null; then
      warn "stray registry file: ${j#"${CONTENT_DIR}"/} (unparseable or no 'props' key — ignored at load, but noisy)"
    fi
  done < <(find -L "${CONTENT_DIR}" -name '*.Package.json' 2>/dev/null | sort)
fi

# Only verification needs the client wheel; importing does not. So this is a
# warning, unlike the old `make import` route where it was fatal on line one.
if [ -n "${CARLA_PY_VERSION:-}" ]; then PYBIN="python${CARLA_PY_VERSION}"; else PYBIN="python3"; fi
PYPATH="$(command -v "${PYBIN}" 2>/dev/null || true)"
if [ -z "${PYPATH}" ]; then
  warn "no '${PYBIN}' on PATH — importing still works; verify_prop.py will not"
elif "${PYPATH}" -c 'import carla' 2>/dev/null; then
  pass "client python: ${PYPATH} (imports carla) — verification available"
else
  warn "${PYPATH} cannot 'import carla' — importing still works, but verify_prop.py needs the wheel"
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
