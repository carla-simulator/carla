#!/usr/bin/env bash
# Prerequisite checks for package-carla-ue4. Read-only, no sudo.
# Exits non-zero ONLY on hard blockers; WARN means a later step handles it.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Capture an EXPLICIT caller pin; the vendored env.sh sets no CARLA_PY_VERSION
# default, so the active `python3` is what we check unless the caller pinned one.
_PYV_PIN="${CARLA_PY_VERSION:-}"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
CARLA_PY_VERSION="${_PYV_PIN}"

FAIL=0
pass() { echo "PASS  $*"; }
warn() { echo "WARN  $*"; }
fail() { echo "FAIL  $*"; FAIL=1; }

CONTENT_DIR="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content"

# --- Hard blockers ----------------------------------------------------------
# Building UE4 + carla content is out of scope here; we confirm only the
# outputs this skill consumes.

if [ -z "${UE4_ROOT}" ]; then
  fail "UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork"
elif [ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ]; then
  pass "UE4 built at ${UE4_ROOT}"
else
  fail "UE4 not built at ${UE4_ROOT} — build UE4 first (looked for Engine/Binaries/Linux/UE4Editor)"
fi

if [ -z "${CARLA_UE4_ROOT}" ]; then
  fail "CARLA_UE4_ROOT is unset — export it, or run from inside a carla checkout"
elif [ -f "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/CarlaUE4.uproject" ]; then
  pass "carla checkout at ${CARLA_UE4_ROOT}"
else
  fail "no Unreal/CarlaUE4/CarlaUE4.uproject under ${CARLA_UE4_ROOT} — CARLA_UE4_ROOT is wrong"
fi

# --- Content: not a blocker, but packaging without it wastes the whole run ---
if [ -d "${CONTENT_DIR}/Carla" ]; then
  pass "Content/Carla present"
else
  warn "Content/Carla missing — the cook succeeds but produces empty maps (build the carla content first)"
fi

# --- Active python: the wheel stage runs `python3 -m build` -----------------
# Manager-agnostic: whatever env is active must provide a `python3` that imports
# `build`. A pinned CARLA_PY_VERSION means a version-suffixed interpreter is used
# instead, so check that one.
if [ -n "${CARLA_PY_VERSION:-}" ]; then PYBIN="python${CARLA_PY_VERSION}"; else PYBIN="python3"; fi
PYPATH="$(command -v "${PYBIN}" 2>/dev/null || true)"
if [ -z "${PYPATH}" ]; then
  warn "no '${PYBIN}' on PATH — activate your CARLA client env before packaging (references/packaging.md P1)"
elif "${PYPATH}" -c 'import build' 2>/dev/null; then
  if "${PYPATH}" -c 'import carla' 2>/dev/null; then
    pass "wheel python: ${PYPATH} (has carla + build)"
  else
    pass "wheel python: ${PYPATH} (has build; carla not importable here — wheel installs into it)"
  fi
else
  warn "${PYPATH} cannot 'import build' — wheel stage fails after the editor compile"
  echo "        install it there: ${PYPATH} -m pip install build   (references/packaging.md P1)"
fi

# --- Disk -------------------------------------------------------------------
AVAIL_GB=$(df -BG --output=avail "${CARLA_UE4_ROOT}" 2>/dev/null | tail -1 | tr -dc '0-9')
if [ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -ge 30 ]; then
  pass "disk: ${AVAIL_GB} GB free"
else
  warn "disk: only ${AVAIL_GB:-?} GB free — a release needs ~30 GB and is not resumable"
fi

# --- ROS 2 native interface (ROS2=${ROS2}) ----------------------------------
# Build-time state, inherited from the editor build. `make package` re-runs
# BuildCarlaUE4.sh, so the flag must be repeated here or it flips OFF.
ROS2_INI="$(carla_ros2_ini_state)"
if [ "${ROS2}" = "1" ]; then
  case "${ROS2_INI}" in
    on)  pass "checkout built with Ros2 ON — the cook keeps it (ARGS gains --ros2)";;
    off) warn "checkout has Ros2 OFF — this cook will rebuild the editor modules WITH ROS 2 (longer: middleware deps may build too)";;
    absent) warn "OptionalModules.ini absent — the cook writes it (Ros2 ON)";;
  esac
  for d in fast-dds-install cyclone-dds-install zenoh-install; do
    [ -d "${CARLA_UE4_ROOT}/Build/${d}" ] && pass "middleware dep present: Build/${d}" \
      || warn "Build/${d} missing — Setup.sh --ros2 builds it from source (long, one-off)"
  done
elif [ "${ROS2_INI}" = "on" ]; then
  warn "checkout is built with Ros2 ON but ROS2=1 is NOT set — this cook produces a package WITHOUT ROS 2 (set ROS2=1 to keep it)"
fi

# --- Asset packages available to cook --------------------------------------
PKG_JSONS="$(find -L "${CONTENT_DIR}" -name '*.Package.json' 2>/dev/null | sort)"
if [ -n "${PKG_JSONS}" ]; then
  pass "asset packages defined:"
  printf '%s\n' "${PKG_JSONS}" | while read -r j; do
    echo "        $(basename "${j}" .Package.json)  (${j#"${CONTENT_DIR}"/})"
  done
else
  warn "no *.Package.json under Content/ — only the full release can be cooked"
  echo "        define one: python3 scripts/package_json.py MyMaps --map Town02"
fi

# --- Dist/ checks: only when it exists (absent before the first package) -----
if [ -d "${CARLA_UE4_ROOT}/Dist" ]; then
  STALE="$(find "${CARLA_UE4_ROOT}/Dist" -maxdepth 1 -name '*.tar' 2>/dev/null || true)"
  if [ -n "${STALE}" ]; then
    warn "stale uncompressed .tar in Dist/ — tar -rf APPENDS to these; remove before re-running:"
    printf '%s\n' "${STALE}" | sed 's/^/        /'
  fi
  if compgen -G "${CARLA_UE4_ROOT}/Dist/*.tar.gz" >/dev/null; then
    warn "Dist/ already holds packages:"
    ls -lah "${CARLA_UE4_ROOT}"/Dist/*.tar.gz | sed 's/^/        /'
  fi
fi

echo
[ "${FAIL}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                    || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${FAIL}"
