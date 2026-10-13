#!/usr/bin/env bash
# Prerequisite checks for install-scenario-runner. Read-only, no sudo.
# Hard blockers: no git, no pip, no importable carla (nothing to match against).
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
# env.sh runs `set -euo pipefail`, and sourcing it applies -e to THIS shell. A
# preflight must report every problem, not stop at the first one, so turn it back
# off — the probes below all set rc explicitly.
set +e

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== Tools =="
command -v git >/dev/null && ok "git $(git --version | awk '{print $3}')" || bad "git missing — needed to clone scenario_runner"
"${PYTHON}" -c 'import sys; sys.exit(0)' 2>/dev/null && ok "${PYTHON} runs" || bad "${PYTHON} not usable"
"${PYTHON}" -m pip --version >/dev/null 2>&1 && ok "pip available for ${PYTHON}" || bad "no pip for ${PYTHON} — cannot install requirements"

echo "== Python version window =="
PV="$("${PYTHON}" -c 'import sys; print("%d.%d" % sys.version_info[:2])')"
case "${PV}" in
  3.7|3.8|3.9|3.10) ok "Python ${PV} is inside the supported 3.7-3.10 window" ;;
  3.11|3.12) warn "Python ${PV}: the carla wheel exists but py_trees 0.8.3 is untested here" ;;
  *) warn "Python ${PV} is outside the tested range (3.7-3.10)" ;;
esac

echo "== What we are matching against =="
"${PYTHON}" - <<'PY'
import importlib.util as u
def have(mod):
    """Is `mod` importable?

    importlib.util.find_spec() IMPORTS the parent packages of a dotted name, so
    find_spec("agents.navigation.x") raises ModuleNotFoundError when `agents`
    itself is absent — which is exactly the case being tested for. Catch it.
    """
    try:
        return u.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError):
        return False
if not have("carla"):
    print("  WARN no importable carla — the branch choice will fall back to master")
    print("  WARN   run install-python-api first for a version-matched decision")
else:
    try:
        from importlib.metadata import version
        print(f"  PASS carla client {version('carla')}")
    except Exception:
        print("  WARN carla imports but has no dist metadata (raw egg?) — version unknown")
if not have("agents.navigation.global_route_planner"):
    print("  WARN `agents` not importable yet — set CARLA_ROOT; scenario_runner needs it at import time")
else:
    print("  PASS agents.navigation importable")
PY

echo "== Target directory =="
if [ -n "${SCENARIO_RUNNER_ROOT}" ] && [ -f "${SCENARIO_RUNNER_ROOT}/scenario_runner.py" ]; then
  ok "existing checkout at ${SCENARIO_RUNNER_ROOT} (branch $(git -C "${SCENARIO_RUNNER_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown))"
  D="$(git -C "${SCENARIO_RUNNER_ROOT}" status --porcelain 2>/dev/null | wc -l)"
  [ "${D}" -gt 0 ] && warn "${D} uncommitted change(s) — a branch switch needs --force" || ok "working tree clean"
elif [ -e "${SR_INSTALL_DIR}" ]; then
  warn "${SR_INSTALL_DIR} already exists — install refuses to clone into a non-empty directory"
else
  ok "will clone into ${SR_INSTALL_DIR}"
  AV="$(df -Pm "$(dirname "${SR_INSTALL_DIR}")" | awk 'NR==2{print $4}')"
  [ "${AV:-0}" -ge 500 ] && ok "${AV} MB free (the checkout is ~150-400 MB depending on branch)" \
    || warn "only ${AV} MB free at $(dirname "${SR_INSTALL_DIR}")"
fi

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
