#!/usr/bin/env bash
# Prerequisite checks for run-route-scenario. Read-only, no sudo, fast — the MCP
# check_prerequisites(name) tool must never hang.
# Exits non-zero on hard blockers: no checkout, no `carla`, no `agents`, no server.
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

echo "== ScenarioRunner checkout =="
if [ -z "${SCENARIO_RUNNER_ROOT}" ]; then
  bad "SCENARIO_RUNNER_ROOT unset and no checkout found in \$PWD, ~/scenario_runner or /workspace/scenario_runner"
  bad "  run the install-scenario-runner skill, or export SCENARIO_RUNNER_ROOT=/path/to/scenario_runner"
elif ! carla_sr_is_root "${SCENARIO_RUNNER_ROOT}"; then
  bad "${SCENARIO_RUNNER_ROOT} holds no scenario_runner.py + srunner/ — wrong path"
else
  BR="$(carla_sr_branch)"; FL="$(carla_sr_flavor)"
  ok "checkout at ${SCENARIO_RUNNER_ROOT} (branch ${BR})"
  case "${FL}" in
    ue4) ok "flavor ue4 — CARLA 0.9.14+ (UE4). Scenarios in Town01..Town10HD_Opt" ;;
    ue5) ok "flavor ue5 — CARLA 0.10.0 (UE5). Town10HD_Opt only; just 11 of 101 configs were ported to it"
         ok "  blueprints are 'vehicle.lincoln.mkz' (no _2017); weather behaviours are disabled" ;;
    ue58) ok "flavor ue58 — CARLA 1.0 on UE 5.8 (pre-release builds report 0.10.0). Towns 1-5 + Town10HD_Opt, all as _Opt"
          ok "  96 configs, every one resolving to a scenario class (ue5-master had 11 runnable:"
          ok "  the rest named non-_Opt towns that do not exist in the UE5 content)"
          ok "  ego is 'vehicle.lincoln.mkz'; vision blockers are 'static.prop.dumpster'"
          ok "  verify after any XML change: python3 scripts/list_scenarios.py --check" ;;
    lb2) ok "flavor lb2 — pinned to Leaderboard 2.0/2.1; routes are Town12/Town13, needs the leaderboard CARLA build" ;;
    lb1) ok "flavor lb1 — pinned to Leaderboard 1.0; needs CARLA 0.9.10.1 exactly" ;;
    *)   warn "branch '${BR}' is not one of master / ue5-master / leaderboard-{1.0,2.0,2.1} — compatibility unknown" ;;
  esac
fi

echo "== Python client =="
"${PYTHON}" - <<'PY'
import importlib.util as u, sys
def ok(m): print(f"  PASS {m}")
def bad(m): print(f"  FAIL {m}"); sys.exit(3)
def warn(m): print(f"  WARN {m}")
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
    bad("cannot import carla — run the install-python-api skill for this interpreter")
import carla
if getattr(carla, "__file__", None) is None:
    # A DIRECTORY named `carla` on sys.path (the CWD counts, and ~/carla is the
    # usual place people clone it) imports as an empty namespace package: no
    # error, __file__ is None, and every later attribute access fails.
    bad(f"`carla` resolved to a directory, not the client: {getattr(carla, '__path__', '?')}")
    bad("  this is a namespace-package shadow, not an install. Run from elsewhere, or"
        " remove that path from PYTHONPATH/CWD")
try:
    from importlib.metadata import version
    cv = version("carla")
except Exception:
    cv = getattr(carla, "__version__", "unknown")
ok(f"carla importable (client {cv})")
# ScenarioRunner imports agents.navigation.global_route_planner at module load.
# That package lives in CARLA's PythonAPI/carla, never in the carla wheel, so a
# pip-only install fails here with ModuleNotFoundError: No module named 'agents'.
if not have("agents.navigation.global_route_planner"):
    bad("no `agents` package — add ${CARLA_ROOT}/PythonAPI/carla to PYTHONPATH (set CARLA_ROOT)")
ok("agents.navigation importable")
missing = [m for m in ("py_trees", "networkx", "shapely", "xmlschema", "tabulate", "ephem", "numpy")
           if not have(m)]
if missing:
    bad(f"missing requirements: {', '.join(missing)} — pip install -r $SCENARIO_RUNNER_ROOT/requirements.txt")
ok("scenario_runner requirements importable")
# py_trees 0.8.3 — the version ScenarioRunner pins — exposes no __version__
# attribute, so read the distribution metadata. Reading the attribute raised
# AttributeError and aborted the whole preflight on a *correct* install.
try:
    from importlib.metadata import version as _dist_version
    _pt = _dist_version("py_trees")
except Exception:
    import py_trees
    _pt = getattr(py_trees, "__version__", "")
if not _pt:
    warn("cannot determine the py_trees version")
elif not _pt.startswith("0.8"):
    warn(f"py_trees {_pt}: only 0.8.x is supported, behaviour trees break on 2.x")
PY
[ $? -ne 0 ] && rc=1

echo "== Simulator =="
"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(4.0)
    sv, cv = c.get_server_version(), c.get_client_version()
    print(f"  PASS server reachable at {sys.argv[1]}:{sys.argv[2]} (server {sv}, client {cv})")
    print(f"  PASS current map: {c.get_world().get_map().name.split('/')[-1]}")
    if sv != cv:
        print(f"  WARN client/server version MISMATCH: client {cv} vs server {sv}")
        print("  WARN   ScenarioRunner aborts mid-scenario on mismatched snapshots —"
              " install a matching client with the install-python-api skill")
except Exception as e:
    print(f"  FAIL no CARLA server at {sys.argv[1]}:{sys.argv[2]} — start one (run-carla-server skill) ({e})")
    sys.exit(3)
PY
[ $? -ne 0 ] && rc=1

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
