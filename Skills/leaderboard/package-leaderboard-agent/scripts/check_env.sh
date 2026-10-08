#!/usr/bin/env bash
# Prerequisite checks for package-leaderboard-agent. Read-only, no sudo, fast — the MCP
# check_prerequisites(name) tool must never hang.
# Hard blockers: missing checkout, mismatched scenario_runner, no `carla`/`agents`.
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

echo "== Leaderboard checkout =="
if [ -z "${LEADERBOARD_ROOT}" ] || ! carla_lb_is_root "${LEADERBOARD_ROOT}"; then
  bad "no leaderboard checkout — export LEADERBOARD_ROOT, or run the install-leaderboard skill"
  LBV="unknown"
else
  LBV="$(carla_lb_version)"
  ok "checkout at ${LEADERBOARD_ROOT} (branch $(carla_lb_branch), detected version ${LBV})"
  case "${LBV}" in
    1.0) ok "LB 1.0 — CARLA 0.9.10.1, Town01-06, tracks SENSORS/MAP, 4 cameras / 1 lidar / 2 radar" ;;
    2.0) ok "LB 2.0 — leaderboard CARLA build (0.9.14+large maps), Town12/13, multiplicative penalties" ;;
    2.1) ok "LB 2.1 — same routes/CARLA as 2.0; penalties are ADDITIVE: score_penalty = 1/(1+sum)" ;;
    *)   warn "cannot tell which leaderboard version this is" ;;
  esac
  if [ "$(carla_lb_branch)" = "master" ]; then
    warn "branch 'master' is the 2.0 line, NOT 2.1 — the online leaderboard scores with 2.1"
    warn "  checkout leaderboard-2.1 to reproduce submitted scores"
  fi
fi

echo "== ScenarioRunner pairing =="
if [ -z "${SCENARIO_RUNNER_ROOT}" ] || ! carla_sr_is_root "${SCENARIO_RUNNER_ROOT}"; then
  bad "no scenario_runner checkout — the leaderboard imports srunner and cannot run without it"
  bad "  run the install-scenario-runner skill (branch $(carla_lb_required_sr_branch))"
else
  SRB="$(git -C "${SCENARIO_RUNNER_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)"
  WANT="$(carla_lb_required_sr_branch)"
  ok "scenario_runner at ${SCENARIO_RUNNER_ROOT} (branch ${SRB})"
  if [ "${WANT}" != "unknown" ] && [ "${SRB}" != "${WANT}" ]; then
    # SR leaderboard-2.0 and leaderboard-2.1 are literally the same commit, so
    # that particular "mismatch" is harmless and reported as a note, not a fail.
    if { [ "${WANT}" = "leaderboard-2.1" ] && [ "${SRB}" = "leaderboard-2.0" ]; } \
    || { [ "${WANT}" = "leaderboard-2.0" ] && [ "${SRB}" = "leaderboard-2.1" ]; }; then
      ok "branch ${SRB} is byte-identical to ${WANT} in scenario_runner — fine"
    elif [ "${SRB}" = "$(carla_lb_branch)" ]; then
      # Both repos on the same branch name is a coordinated fork line (a port
      # advancing the pair together, e.g. ue58-dev on both). Matched by
      # construction, so requiring the upstream branch NAME would reject a valid
      # setup. What matters is that the leaderboard's srunner imports resolve.
      ok "both repos on '${SRB}' — a coordinated fork line, treated as matched"
    else
      bad "scenario_runner is on '${SRB}' but this leaderboard needs '${WANT}'"
      bad "  master/ue5-master do NOT work: the route scenario classes differ"
    fi
  fi
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
    cv = "unknown"
ok(f"carla importable (client {cv})")
# The leaderboard CARLA package reports the literal version 'leaderboard'; the
# evaluator special-cases it and skips its own minimum-version check.
if cv == "leaderboard":
    ok("client is the leaderboard CARLA build ('leaderboard' version string)")
for mod, why in [("agents.navigation.global_route_planner", "add ${CARLA_ROOT}/PythonAPI/carla to PYTHONPATH"),
                 ("srunner.scenariomanager.carla_data_provider", "add ${SCENARIO_RUNNER_ROOT} to PYTHONPATH"),
                 ("leaderboard.utils.statistics_manager", "add ${LEADERBOARD_ROOT} to PYTHONPATH")]:
    if not have(mod):
        bad(f"cannot import {mod} — {why}")
ok("agents / srunner / leaderboard all importable")
missing = [m for m in ("py_trees", "dictor", "requests", "tabulate", "numpy", "shapely", "networkx")
           if not have(m)]
if missing:
    bad(f"missing requirements: {', '.join(missing)} — pip install -r $LEADERBOARD_ROOT/requirements.txt"
        " -r $SCENARIO_RUNNER_ROOT/requirements.txt")
ok("leaderboard + scenario_runner requirements importable")
PY
[ $? -ne 0 ] && rc=1

echo "== Agent and track =="
if [ -z "${TEAM_AGENT:-}" ]; then
  warn "TEAM_AGENT unset — export it, or use \$LEADERBOARD_ROOT/leaderboard/autoagents/npc_agent.py to smoke-test"
elif [ ! -f "${TEAM_AGENT}" ]; then
  bad "TEAM_AGENT=${TEAM_AGENT} does not exist"
else
  ok "agent ${TEAM_AGENT}"
fi
case "${CHALLENGE_TRACK_CODENAME}" in
  SENSORS|MAP) ok "track ${CHALLENGE_TRACK_CODENAME}" ;;
  SENSORS_QUALIFIER|MAP_QUALIFIER)
    if [ "${LBV}" = "1.0" ]; then bad "track ${CHALLENGE_TRACK_CODENAME} does not exist in LB 1.0 (only SENSORS/MAP)"
    else ok "track ${CHALLENGE_TRACK_CODENAME} (halved sensor budget)"; fi ;;
  *) bad "CHALLENGE_TRACK_CODENAME='${CHALLENGE_TRACK_CODENAME}' is not a valid track" ;;
esac

echo "== Simulator =="
"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(4.0)
    print(f"  PASS server reachable (server {c.get_server_version()})")
    maps = [m.split('/')[-1] for m in c.get_available_maps()]
    for need in ("Town12", "Town13"):
        if need in maps: print(f"  PASS {need} available")
        else: print(f"  WARN {need} NOT available — LB 2.x routes need it (install AdditionalMaps"
                    " or use the leaderboard CARLA build)")
except Exception as e:
    print(f"  FAIL no CARLA server at {sys.argv[1]}:{sys.argv[2]} — start one (run-carla-server skill) ({e})")
    sys.exit(3)
PY
[ $? -ne 0 ] && rc=1

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
