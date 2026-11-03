#!/usr/bin/env bash
# Prerequisite checks for install-scenic. Read-only, no sudo, fast.
# Exits non-zero only on hard blockers: no interpreter, no pip.
# A missing carla client is a WARN — install order is the skill's job to fix.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail

# This skill has no env.sh (nothing to resolve before it runs), so it loads the
# recorded paths itself — otherwise a PYTHON set through set_config would be
# invisible to the one skill whose whole job is picking that interpreter.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

PYTHON="${PYTHON:-python3}"
rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== Interpreter =="
if ! command -v "${PYTHON}" >/dev/null 2>&1; then
  bad "no interpreter '${PYTHON}' — set PYTHON to the one that has (or will have) the carla client"
else
  V="$("${PYTHON}" -c 'import sys;print("%d.%d.%d"%sys.version_info[:3])' 2>/dev/null)"
  ok "${PYTHON} is Python ${V}"
  # Scenic needs 3.8+. The practical ceiling is not Scenic but the CARLA client:
  # its wheel is built per Python version, so only the matching one exists.
  "${PYTHON}" -c 'import sys;sys.exit(0 if sys.version_info[:2] >= (3,8) else 1)' \
    || bad "Scenic needs Python 3.8 or newer"
  "${PYTHON}" -m pip --version >/dev/null 2>&1 && ok "pip available" \
    || bad "no pip in ${PYTHON} — python -m ensurepip, or use the distro package"
fi

echo "== CARLA client =="
CV="$("${PYTHON}" -c 'from importlib.metadata import version;print(version("carla"))' 2>/dev/null)"
if [ -n "${CV}" ]; then
  ok "carla client ${CV} in the same interpreter"
else
  warn "no carla client here — install it FIRST (install-python-api skill)"
  warn "  Scenic's blueprint tables are keyed on the CLIENT version, and its CLI must"
  warn "  share the interpreter, so installing Scenic first can look fine and fail later"
fi

echo "== Already installed? =="
SV="$("${PYTHON}" -c 'from importlib.metadata import version;print(version("scenic"))' 2>/dev/null)"
if [ -n "${SV}" ]; then
  ok "scenic ${SV} present — 'verify' will say whether it matches the client"
else
  ok "scenic not installed yet (expected for a first run)"
fi

echo "== PyPI reachable =="
if "${PYTHON}" -m pip download --no-deps --dest /tmp/.scenic-probe scenic==0 >/dev/null 2>&1; then
  ok "index reachable"
else
  # pip exits non-zero for "no such version" too, which is the expected answer for
  # ==0; distinguish a resolver answer from no network at all.
  if "${PYTHON}" -m pip index versions scenic >/dev/null 2>&1; then
    ok "index reachable"
  else
    warn "cannot reach the package index — an offline install needs a local wheel"
  fi
fi
rm -rf /tmp/.scenic-probe 2>/dev/null

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
