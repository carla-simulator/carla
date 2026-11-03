#!/usr/bin/env bash
# Prerequisite checks for the Scenic skills. Read-only, no sudo, fast — the MCP
# check_prerequisites(name) tool must never hang.
# Exits non-zero on hard blockers: no scenic CLI, scenic/carla not importable from
# the same interpreter, client/server version mismatch, no server.
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
# preflight must report every problem, not stop at the first one.
set +e

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== Scenic =="
if [ -z "${SCENIC_BIN}" ]; then
  bad "no scenic CLI on PATH — pip install scenic into the interpreter that imports carla"
else
  ok "CLI at ${SCENIC_BIN}"
  # The CLI and the carla client must live in ONE interpreter. A pyenv shim can
  # resolve to a different env than $PYTHON, and the mismatch only surfaces later
  # as a model-import failure deep in a run.
  _sv="$(carla_scenic_version)"
  if [ "${_sv}" = "unknown" ]; then
    bad "scenic is not importable from ${PYTHON} even though the CLI exists"
    bad "  the CLI belongs to a different environment — set PYTHON to that interpreter"
  else
    ok "scenic ${_sv} importable from ${PYTHON}"
  fi
fi
[ -n "${SCENIC_EXAMPLES}" ] && ok "examples at ${SCENIC_EXAMPLES}" \
  || warn "no Scenic checkout found — the wheel ships world models but no example scenarios"

echo "== Scenario sources =="
if [ -z "${SCENARIO_RUNNER_ROOT}" ]; then
  warn "SCENARIO_RUNNER_ROOT unset — scenarios using 'model srunner.scenic.models.model' cannot import"
elif [ ! -d "${SCENARIO_RUNNER_ROOT}/srunner/scenic" ]; then
  warn "${SCENARIO_RUNNER_ROOT} has no srunner/scenic/ — that branch carries no Scenic model"
else
  _n="$(find "${SCENARIO_RUNNER_ROOT}/srunner/scenic" -maxdepth 1 -name '*.scenic' 2>/dev/null | wc -l)"
  ok "srunner.scenic model + ${_n} scenarios at ${SCENARIO_RUNNER_ROOT}/srunner/scenic"
fi

echo "== Python client =="
"${PYTHON}" - <<'PY'
import importlib.util as u, sys
def ok(m): print(f"  PASS {m}")
def bad(m): print(f"  FAIL {m}"); sys.exit(3)
def warn(m): print(f"  WARN {m}")
def have(mod):
    try: return u.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError): return False
if not have("scenic"):
    bad("cannot import scenic — pip install scenic")
if not have("carla"):
    bad("cannot import carla — install the client wheel matching the server")
import carla
if getattr(carla, "__file__", None) is None:
    # A DIRECTORY named `carla` on sys.path imports as an empty namespace package:
    # no error, __file__ is None, every later attribute access fails.
    bad(f"`carla` resolved to a directory, not the client: {getattr(carla,'__path__','?')}")
ok("scenic and carla importable from one interpreter")
# Scenic's CARLA simulator interface needs these; the driving domain needs shapely.
missing = [m for m in ("shapely", "trimesh", "scipy", "numpy", "pygame") if not have(m)]
if missing:
    bad(f"missing Scenic runtime deps: {', '.join(missing)} — pip install scenic")
ok("Scenic runtime deps present")
PY
[ $? -ne 0 ] && rc=1

echo "== Blueprint table =="
# Scenic picks blueprints from a table keyed by CLIENT version. An unknown key
# means every category resolves empty and scenarios fail at sample time with
# "no 'X' blueprints recorded", which reads like a scenario bug but is not.
"${PYTHON}" - <<'PY'
import sys
try:
    from importlib.metadata import version
    cv = version("carla")
except Exception:
    import carla; cv = getattr(carla, "__version__", "unknown")
try:
    from scenic.simulators.carla import _blueprintData as bd
except Exception as e:
    print(f"  WARN cannot read Scenic's blueprint table ({e})"); sys.exit(0)
keys = sorted(bd._IDS)
if cv in bd._IDS:
    tab = bd._IDS[cv]
    empty = sorted(k for k, v in tab.items() if not v)
    print(f"  PASS Scenic has a table for client {cv} ({sum(len(v) for v in tab.values())} ids)")
    if empty:
        print(f"  WARN empty categories for {cv}: {', '.join(empty)}")
        print("  WARN   scenarios asking for those types fail at sample time, not at runtime")
else:
    print(f"  FAIL Scenic has no blueprint table for client {cv} (has: {', '.join(keys)})")
    sys.exit(3)
PY
[ $? -ne 0 ] && rc=1

echo "== Simulator =="
"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(10.0)
    sv, cv = c.get_server_version(), c.get_client_version()
    print(f"  PASS server reachable at {sys.argv[1]}:{sys.argv[2]} (server {sv}, client {cv})")
    w = c.get_world()
    print(f"  PASS current map: {w.get_map().name.split('/')[-1]}")
    s = w.get_settings()
    if s.synchronous_mode:
        print("  WARN server is in synchronous mode — a previous run left it there;"
              " Scenic sets sync itself, but any other client will appear frozen")
    if sv != cv:
        print(f"  FAIL client/server version MISMATCH: client {cv} vs server {sv}")
        print("  FAIL   Scenic's blueprint table is keyed on the CLIENT version, so a"
              " mismatch silently offers ids the server does not have")
        sys.exit(3)
except SystemExit: raise
except Exception as e:
    print(f"  FAIL no usable CARLA server at {sys.argv[1]}:{sys.argv[2]} ({e})")
    sys.exit(3)
PY
[ $? -ne 0 ] && rc=1

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
