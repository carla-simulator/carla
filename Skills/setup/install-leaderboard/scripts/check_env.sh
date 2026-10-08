#!/usr/bin/env bash
# Prerequisite checks for install-leaderboard. Read-only, no sudo.
# Hard blockers: no git, no pip. Everything else is advisory — this skill's job
# is to create the environment the other leaderboard skills demand.
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
command -v git >/dev/null && ok "git $(git --version | awk '{print $3}')" || bad "git missing — needed to clone"
"${PYTHON}" -m pip --version >/dev/null 2>&1 && ok "pip available for ${PYTHON}" || bad "no pip for ${PYTHON}"
PV="$("${PYTHON}" -c 'import sys; print("%d.%d" % sys.version_info[:2])' 2>/dev/null || echo '?')"
case "${PV}" in
  3.7|3.8) ok "Python ${PV} — matches the official leaderboard docker (3.7)" ;;
  3.9|3.10) ok "Python ${PV} — works; note opencv-python==4.2.0.32 has no wheel here" ;;
  *) warn "Python ${PV} is outside the tested 3.7-3.10 range" ;;
esac

echo "== Existing checkouts =="
"${PYTHON}" "${HERE}/install_leaderboard.py" detect 2>&1 | sed 's/^/  /' || warn "detect failed"

echo "== CARLA client =="
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
    print("  WARN no importable carla yet — install it after choosing the version (install-python-api)")
else:
    try:
        from importlib.metadata import version
        v = version("carla")
        print(f"  PASS carla client {v}")
        if v == "leaderboard":
            print("  PASS   this is the leaderboard CARLA build — required for LB 2.x evaluation")
        else:
            print("  WARN   not the leaderboard build; LB 2.x routes will run but scores are indicative only")
    except Exception:
        print("  WARN carla imports but has no dist metadata (raw egg) — version unknown")
PY

echo "== Disk =="
AV="$(df -Pm "${LB_INSTALL_DIR}" 2>/dev/null | awk 'NR==2{print $4}')"
[ "${AV:-0}" -ge 1000 ] && ok "${AV} MB free under ${LB_INSTALL_DIR} (both checkouts are ~400 MB)" \
  || warn "only ${AV:-?} MB free under ${LB_INSTALL_DIR}"

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
