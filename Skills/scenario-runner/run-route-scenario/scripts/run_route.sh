#!/usr/bin/env bash
# Inspect and run ScenarioRunner routes.
#
#   bash run_route.sh list  routes.xml            # ids, towns, scenario histogram, warnings
#   bash run_route.sh show  routes.xml 0          # one route's scenarios in trigger order
#   bash run_route.sh run   routes.xml [id]       # drive it (AGENT required)
#
# Knobs (env): AGENT (required for run), AGENT_CONFIG, REPETITIONS,
#              OUTPUT/FILE_OUT/JSON/JUNIT=1, OUTPUT_DIR, RECORD, DEBUG=1, TIMEOUT,
#              FRAME_RATE (route mode is always synchronous when an agent is used)
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
# env.sh runs `set -euo pipefail`, and sourcing it applies -e to THIS shell. This
# wrapper must survive a non-zero exit from the tool it runs, or its verification
# and cleanup are skipped precisely when a run failed.
set +e


CMD="${1:-}"; ROUTES="${2:-}"; RID="${3:-}"
[ -n "${SCENARIO_RUNNER_ROOT}" ] || { echo "SCENARIO_RUNNER_ROOT is not set — run check_env.sh" >&2; exit 2; }
case "${CMD}" in list|show|run) ;; *)
  echo "usage: bash run_route.sh {list|show|run} <routes.xml> [route-id]" >&2; exit 2 ;; esac
[ -f "${ROUTES:-}" ] || { echo "ERROR: route file '${ROUTES}' not found" >&2; exit 3; }

if [ "${CMD}" != "run" ]; then
  "${PYTHON}" - "${ROUTES}" "${SCENARIO_RUNNER_ROOT}" "${CMD}" "${RID}" "${CARLA_HOST}" "${CARLA_PORT}" <<'PY'
import collections, glob, re, sys, xml.etree.ElementTree as ET
path, root, cmd, rid, host, port = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], int(sys.argv[6])
tree = ET.parse(path)
routes = tree.getroot().findall("route")
if not routes:
    sys.exit("no <route> elements — is this a route file?")

# Format detection matters: a 1.0 file keeps scenarios in a separate JSON and
# uses flat <waypoint> children, and scenario_runner on master cannot read it.
fmt = "2.x" if routes[0].find("waypoints") is not None else "1.0"
print(f"# {path}: {len(routes)} route(s), format {fmt}")
if fmt == "1.0":
    print("# WARNING 1.0 format: geometry only. Scenarios live in a separate --scenarios JSON,")
    print("#         which scenario_runner.py on master/ue5-master does not accept.")

known = set()
for f in glob.glob(f"{root}/srunner/scenarios/*.py"):
    known |= set(re.findall(r"^class (\w+)\(", open(f, errors="replace").read(), re.M))

avail = None
try:
    import carla
    c = carla.Client(host, port); c.set_timeout(4.0)
    avail = {m.split('/')[-1] for m in c.get_available_maps()}
except Exception:
    pass

if cmd == "show":
    sel = [r for r in routes if r.attrib.get("id") == rid]
    if not sel:
        sys.exit(f"no route with id {rid!r}; ids are: {', '.join(r.attrib.get('id','?') for r in routes)}")
    r = sel[0]
    wps = r.findall("waypoints/position") or r.findall("waypoint")
    print(f"route {r.attrib.get('id')}  town={r.attrib.get('town')}  waypoints={len(wps)}")
    for w in r.findall("weathers/weather"):
        print(f"  weather @{w.attrib.get('route_percentage')}%  "
              f"sun_alt={w.attrib.get('sun_altitude_angle')} rain={w.attrib.get('precipitation')} "
              f"fog={w.attrib.get('fog_density')}")
    scs = r.findall("scenarios/scenario")
    print(f"  {len(scs)} scenario(s):")
    for s in scs:
        tp = s.find("trigger_point")
        loc = f"({tp.attrib.get('x')}, {tp.attrib.get('y')}, {tp.attrib.get('z')}) yaw={tp.attrib.get('yaw')}" if tp is not None else "?"
        flag = "" if s.attrib.get("type") in known else "   <-- NO CLASS: will be skipped"
        print(f"    {s.attrib.get('name','?'):34} {s.attrib.get('type','?'):32} {loc}{flag}")
    sys.exit(0)

missing_types, towns = set(), set()
print(f"{'ID':>5}  {'TOWN':14} {'WPS':>4} {'SCEN':>5}  SCENARIOS")
for r in routes:
    rid_ = r.attrib.get("id", "?")
    town = r.attrib.get("town", "?")
    towns.add(town)
    wps = r.findall("waypoints/position") or r.findall("waypoint")
    scs = [s.attrib.get("type") for s in r.findall("scenarios/scenario")]
    missing_types |= {t for t in scs if t not in known}
    hist = collections.Counter(scs)
    top = ", ".join(f"{k}x{v}" for k, v in hist.most_common(4))
    more = f" (+{len(hist)-4} more types)" if len(hist) > 4 else ""
    print(f"{rid_:>5}  {town:14} {len(wps):>4} {len(scs):>5}  {top}{more}")

print(f"\ntowns used: {', '.join(sorted(towns))}")
if avail is not None:
    for t in sorted(towns):
        if t not in avail:
            print(f"  MISSING  {t} is not on the server — routes in it cannot load"
                  " (AdditionalMaps? leaderboard build?)")
        else:
            print(f"  ok       {t}")
else:
    print("  (no server reachable — town availability not checked)")
if missing_types:
    print(f"\n{len(missing_types)} scenario type(s) in this file have NO class in {root}:")
    for t in sorted(missing_types):
        print(f"  {t}")
    print("  RouteScenario skips unknown types silently — this is the usual cause of")
    print("  'the ego just drives and nothing ever happens'. Check the branch.")
else:
    print("\nall scenario types in this file exist in the checkout")
PY
  exit $?
fi

# --- run --------------------------------------------------------------------
[ -n "${AGENT:-}" ] || {
  echo "ERROR: route mode needs an agent. Set AGENT=<path to agent .py>." >&2
  echo "       reference agent: ${SCENARIO_RUNNER_ROOT}/srunner/autoagents/npc_agent.py" >&2
  exit 2; }
[ -f "${AGENT}" ] || { echo "ERROR: AGENT=${AGENT} does not exist" >&2; exit 3; }

# ScenarioRunner derives the class name from the file name; a mismatch is a bare
# AttributeError after the world has already been reloaded, so check up front.
EXPECT="$("${PYTHON}" -c "import os,sys;print(os.path.basename(sys.argv[1]).split('.')[0].title().replace('_',''))" "${AGENT}")"
if ! grep -qE "^class[[:space:]]+${EXPECT}\b" "${AGENT}"; then
  echo "ERROR: ${AGENT} must define 'class ${EXPECT}' (the name is derived from the file name)" >&2
  grep -nE '^class [A-Za-z_]+' "${AGENT}" | sed 's/^/       found: /' >&2
  exit 6
fi
echo "[run] agent class ${EXPECT} found in ${AGENT}"

ARGS=(--route "${ROUTES}" --agent "${AGENT}"
      --host "${CARLA_HOST}" --port "${CARLA_PORT}" --trafficManagerPort "${CARLA_TM_PORT}"
      --timeout "${TIMEOUT:-60}" --repetitions "${REPETITIONS:-1}"
      --frameRate "${FRAME_RATE:-20}")
[ -n "${RID}" ]                 && ARGS+=(--route-id "${RID}")
[ -n "${AGENT_CONFIG:-}" ]      && ARGS+=(--agentConfig "${AGENT_CONFIG}")
[ "${DEBUG:-0}" = "1" ]         && ARGS+=(--debug)
[ "${OUTPUT:-0}" = "1" ]        && ARGS+=(--output)
[ "${FILE_OUT:-0}" = "1" ]      && ARGS+=(--file)
[ "${JSON:-0}" = "1" ]          && ARGS+=(--json)
[ "${JUNIT:-0}" = "1" ]         && ARGS+=(--junit)
[ -n "${OUTPUT_DIR:-}" ]        && { mkdir -p "${OUTPUT_DIR}"; ARGS+=(--outputDir "${OUTPUT_DIR}"); }
[ -n "${RECORD:-}" ]            && { mkdir -p "${SCENARIO_RUNNER_ROOT}/${RECORD}"; ARGS+=(--record "${RECORD}"); }

# Route mode forces --sync when an agent is given, so the reset is unconditional.
reset_async() {
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${CARLA_TM_PORT}" <<'PY' || true
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    w = c.get_world(); s = w.get_settings()
    if s.synchronous_mode:
        s.synchronous_mode = False; s.fixed_delta_seconds = None
        w.apply_settings(s)
        c.get_trafficmanager(int(sys.argv[3])).set_synchronous_mode(False)
        print("[run] world reset to asynchronous mode")
except Exception as e:
    print(f"[run] WARNING could not reset async mode: {e}")
PY
}
trap reset_async EXIT INT TERM

echo "[run] ${PYTHON} scenario_runner.py ${ARGS[*]}"
echo "[run] route mode forces --reloadWorld and --sync; the world WILL be reloaded"
cd "${SCENARIO_RUNNER_ROOT}"
# -u: Python block-buffers stdout when it is not a tty, so redirecting a long
# run to a log otherwise shows nothing until it finishes.
"${PYTHON}" -u scenario_runner.py "${ARGS[@]}"
RC=$?
echo "[run] scenario_runner exited ${RC}"
exit ${RC}
