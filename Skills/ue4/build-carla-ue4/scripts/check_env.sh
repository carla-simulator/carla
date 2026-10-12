#!/usr/bin/env bash
# Preflight — read-only checks that the host can build CARLA ue4-dev.
# Prints a PASS/WARN/FAIL report; exits non-zero only on hard blockers.
# Run by the MCP check_prerequisites(name) tool.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== OS =="
. /etc/os-release
case "${VERSION_ID:-}" in
  20.04|22.04) ok "Ubuntu ${VERSION_ID} (officially supported)";;
  24.04)       warn "Ubuntu 24.04 — supported via Ubuntu24Compat.sh (needs lld; PEP668 wheel install).";;
  *)           warn "Ubuntu ${VERSION_ID:-?} — untested.";;
esac

echo "== CARLA source =="
if [ -z "${CARLA_UE4_ROOT}" ]; then
  bad "CARLA_UE4_ROOT is unset — export it, or run from inside a carla ue4-dev checkout"
elif [ -f "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/CarlaUE4.uproject" ]; then
  if [ -e "${CARLA_UE4_ROOT}/.git" ]; then   # -e: .git is a FILE in a git worktree
    BR=$(git -C "${CARLA_UE4_ROOT}" branch --show-current 2>/dev/null)
    [ "${BR}" = "ue4-dev" ] && ok "carla on ue4-dev at ${CARLA_UE4_ROOT}" \
      || warn "carla on '${BR:-?}' (expected ue4-dev) at ${CARLA_UE4_ROOT}"
  else
    ok "carla checkout at ${CARLA_UE4_ROOT}"
  fi
else
  bad "no Unreal/CarlaUE4/CarlaUE4.uproject under ${CARLA_UE4_ROOT} — CARLA_UE4_ROOT is wrong"
fi

echo "== Disk (need ~120GB free: UE ~80 + content ~31 + builds, L2) =="
DISK_ROOT="${CARLA_UE4_ROOT:-$PWD}"
FREE_G=$(df -BG --output=avail "${DISK_ROOT}" 2>/dev/null | tail -1 | tr -dc '0-9')
[ "${FREE_G:-0}" -ge 120 ] && ok "${FREE_G}G free" || warn "${FREE_G:-?}G free (<120G)"

echo "== Tools =="
for t in git git-lfs cmake ninja make; do command -v "$t" >/dev/null && ok "$t" || bad "$t missing"; done
command -v ld.lld >/dev/null && ok "ld.lld (24.04 linker fix)" \
  || { [ "${VERSION_ID:-}" = "24.04" ] && bad "ld.lld missing (REQUIRED on 24.04: sudo apt install lld)" || warn "ld.lld missing"; }

echo "== UE4 fork =="
# Distinguish "unset" from "set but not built" so the failure names the real problem.
if [ -z "${UE4_ROOT}" ]; then
  bad "UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork"
elif [ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ]; then
  ok "UE4 built at ${UE4_ROOT}"
elif [ -d "${UE4_ROOT}/Engine" ]; then
  warn "UE4 cloned but NOT built at ${UE4_ROOT} (run step 03)"
else
  warn "UE4 fork not cloned at ${UE4_ROOT} (step 03 will instruct)"
fi

echo "== Content =="
# A bare Content/Carla dir is created by `git clone` immediately, so its mere
# existence does not mean the ~31GB checkout finished (L14). Require the .git
# repo AND at least one checked-out asset entry.
# find -L: Content/Carla may be a symlink to a shared content checkout.
CONTENT="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla"
if [ -d "${CONTENT}/.git" ] && [ -n "$(find -L "${CONTENT}" -mindepth 1 -maxdepth 1 ! -name '.git' -print -quit 2>/dev/null)" ]; then
  ok "Content present"
elif [ -d "${CONTENT}/.git" ]; then
  warn "Content clone in progress / incomplete (step 05 not finished)"
else
  warn "Content missing (step 05)"
fi

echo "== ROS 2 native interface (ROS2=${ROS2}) =="
# Build-time state only: this reports what the checkout is configured/built for.
# Runtime enabling is a separate flag ([[run-carla-server]] ROS2=1).
INI_STATE="$(carla_ros2_ini_state)"
case "${INI_STATE}" in
  absent) warn "Config/OptionalModules.ini not written yet (step 06 creates it)";;
  on)     ok   "checkout configured with Ros2 ON";;
  off)    if [ "${ROS2}" = "1" ]; then
            warn "checkout has Ros2 OFF — step 06 with ROS2=1 rebuilds it ON"
          else
            ok "checkout configured with Ros2 OFF"
          fi;;
esac
if [ "${ROS2}" = "1" ]; then
  # Setup.sh --ros2 builds these from source; presence means the long dependency
  # stage is already done. Names come from Setup.sh's *_INSTALL_DIR variables.
  for d in fast-dds-install cyclone-dds-install zenoh-install; do
    [ -d "${CARLA_UE4_ROOT}/Build/${d}" ] && ok "middleware dep built: Build/${d}" \
      || warn "Build/${d} missing — step 06 (ARGS=--ros2) builds it from source (long)"
  done
  # These are cloned from GitHub during the dependency build.
  command -v git >/dev/null && ok "git present (middleware sources are cloned)" || bad "git missing"
fi

echo "== Python client env (no manager assumed) =="
# Manager-agnostic: whatever `python3` (or python<pin>) the active env provides.
# See scripts/env.sh. Missing deps are WARNs — 02_client_env.sh adds
# them; boost.python builds on 3.10-3.12 (L5's ">3.10 breaks" caveat is stale).
PY="${CARLA_PY_VERSION:+python${CARLA_PY_VERSION}}"; PY="${PY:-python3}"
PY_BIN="$(command -v "${PY}" 2>/dev/null || true)"
if [ -z "${PY_BIN}" ]; then
  warn "no '${PY}' on PATH — activate a CARLA client env before step 02"
else
  PY_VER="$("${PY_BIN}" -c 'import sys;print("%d.%d"%sys.version_info[:2])' 2>/dev/null)"
  ok "python: ${PY_BIN} (${PY_VER:-?})"
  "${PY_BIN}" -c 'import numpy,sys;sys.exit(0 if numpy.__version__.split(".")[0]=="1" else 1)' 2>/dev/null \
    && ok "numpy 1.x present" \
    || { "${PY_BIN}" -c 'import numpy' 2>/dev/null \
           && warn "numpy>=2 — pin numpy<2 (step 02); 'import carla' crashes otherwise (L6)" \
           || warn "numpy not installed (step 02 adds it)"; }
fi

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
