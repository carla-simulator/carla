#!/usr/bin/env bash
# Wrappers around the leaderboard's route authoring scripts, plus an offline
# structural check they do not provide.
#
#   bash route_tools.sh check    --file routes.xml            (offline, no server)
#   bash route_tools.sh summary  --file routes.xml
#   bash route_tools.sh display  --file routes.xml [--route 0] [--keypoints] [--scenarios] [--all]
#   bash route_tools.sh create   --file routes.xml
#   bash route_tools.sh scenarios --file routes.xml [--show-only]
#   bash route_tools.sh order    --file routes.xml
#   bash route_tools.sh weather  [--route 0]
#   bash route_tools.sh bridge   --routes old.xml --scenarios scen.json --endpoint new.xml
#
# Every mode except `check` needs a RUNNING server with the route's town loaded:
# these tools read positions off the spectator camera and draw debug shapes.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

MODE="${1:-}"; shift || true
[ -n "${LEADERBOARD_ROOT}" ] || { echo "LEADERBOARD_ROOT is not set — run check_env.sh" >&2; exit 2; }
S="${LEADERBOARD_ROOT}/scripts"

FILE=""; ROUTE=""; EXTRA=()
while [ $# -gt 0 ]; do
  case "$1" in
    --file|-f)     FILE="$2"; shift 2 ;;
    --route|-r)    ROUTE="$2"; shift 2 ;;
    --keypoints)   EXTRA+=(-sk); shift ;;
    --scenarios)   EXTRA+=(-ss); shift ;;
    --all)         EXTRA+=(-sa); shift ;;
    --show-only)   EXTRA+=(-s);  shift ;;
    *)             EXTRA+=("$1"); shift ;;
  esac
done

need_server() {
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" <<'PY'
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    print(f"[rt] server up, map {c.get_world().get_map().name.split('/')[-1]}")
except Exception as e:
    sys.exit(f"[rt] ERROR no CARLA server at {sys.argv[1]}:{sys.argv[2]} — "
             f"these tools drive the spectator and need one ({e})")
PY
}

case "${MODE}" in
check)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh check --file routes.xml" >&2; exit 2; }
  "${PYTHON}" - "${FILE}" "${SCENARIO_RUNNER_ROOT}" "${CARLA_HOST}" "${CARLA_PORT}" <<'PY'
import glob, math, os, re, sys, xml.etree.ElementTree as ET
path, sr, host, port = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
rc = 0
def bad(m):
    global rc
    print(f"FAIL  {m}"); rc = 1
def seg_dist(p, a, b):
    """Distance from point p to segment ab."""
    ax, ay = a; bx, by = b; px, py = p
    dx, dy = bx - ax, by - ay
    if dx == 0 and dy == 0:
        return math.dist(p, a)
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
    return math.dist(p, (ax + t * dx, ay + t * dy))
def warn(m): print(f"WARN  {m}")
def ok(m):   print(f"PASS  {m}")

if not os.path.isfile(path):
    sys.exit(f"FAIL  {path} does not exist")
try:
    root = ET.parse(path).getroot()
except ET.ParseError as e:
    sys.exit(f"FAIL  not well-formed XML: {e}")
ok("well-formed XML")
if root.tag != "routes":
    bad(f"root element is <{root.tag}>, expected <routes>")
routes = root.findall("route")
if not routes:
    sys.exit("FAIL  no <route> elements")

fmt = "2.x" if routes[0].find("waypoints") is not None else "1.0"
ok(f"{len(routes)} route(s), format {fmt}")
if fmt == "1.0":
    warn("1.0 format: scenarios live in a separate JSON. Convert with `bridge` before"
         " using it with a 2.x leaderboard.")

ids = [r.attrib.get("id") for r in routes]
dupes = {i for i in ids if ids.count(i) > 1}
if dupes:
    bad(f"duplicate route id(s): {sorted(dupes)}")
else:
    ok("route ids unique")
if ids != [str(i) for i in range(len(ids))]:
    warn("route ids are not 0..N-1: --routes-subset indexes POSITION, not id, so they "
         "will not line up")

known = set()
for f in glob.glob(f"{sr}/srunner/scenarios/*.py"):
    known |= set(re.findall(r"^class (\w+)\(", open(f, errors="replace").read(), re.M))
if not known:
    warn(f"no scenario classes found under {sr} — cannot check scenario types")

avail = None
try:
    import carla
    c = carla.Client(host, port); c.set_timeout(4.0)
    avail = {m.split('/')[-1] for m in c.get_available_maps()}
except Exception:
    warn("no server reachable — town availability not checked")

missing_types, towns = set(), set()
for r in routes:
    rid = r.attrib.get("id", "?")
    town = r.attrib.get("town")
    if not town:
        bad(f"route {rid}: no town attribute")
    else:
        towns.add(town)
    wps = r.findall("waypoints/position") or r.findall("waypoint")
    if len(wps) < 2:
        bad(f"route {rid}: {len(wps)} waypoint(s) — a route needs at least a start and an end")
    pts = []
    for w in wps:
        try:
            pts.append((float(w.attrib["x"]), float(w.attrib["y"]), float(w.attrib.get("z", 0))))
        except (KeyError, ValueError):
            bad(f"route {rid}: waypoint with missing/invalid x,y")
    # Rough polyline length: a route whose keypoints are metres apart is usually a
    # copy-paste error, and one 50 km long is usually a units mistake.
    length = sum(math.dist(a[:2], b[:2]) for a, b in zip(pts, pts[1:]))
    if pts and length < 20:
        warn(f"route {rid}: keypoints span only {length:.1f} m")
    for w in r.findall("weathers/weather"):
        p = w.attrib.get("route_percentage")
        if p is None:
            bad(f"route {rid}: <weather> without route_percentage")
        elif not 0 <= float(p) <= 100:
            bad(f"route {rid}: weather route_percentage={p} outside 0..100")
    scs = r.findall("scenarios/scenario")
    for s in scs:
        t, n = s.attrib.get("type"), s.attrib.get("name")
        if not t:
            bad(f"route {rid}: <scenario> without a type")
        elif known and t not in known:
            missing_types.add(t)
        tp = s.find("trigger_point")
        if tp is None:
            bad(f"route {rid}/{n}: no <trigger_point>")
            continue
        try:
            tx, ty = float(tp.attrib["x"]), float(tp.attrib["y"])
        except (KeyError, ValueError):
            bad(f"route {rid}/{n}: trigger_point with missing/invalid x,y")
            continue
        # A trigger point off the driven path never fires — but the keypoint
        # polyline is only a planner hint, and the real route follows roads between
        # keypoints. Calibrated against the 6534 scenarios in the shipped Town12/13
        # route files: median offset 24 m, p95 224 m, p99 510 m, max 1242 m. So only
        # a gross outlier is worth reporting; anything tighter cries wolf on the
        # official routes.
        if len(pts) >= 2:
            d = min(seg_dist((tx, ty), a[:2], b[:2]) for a, b in zip(pts, pts[1:]))
            if d > 1300:
                warn(f"route {rid}/{n}: trigger point is {d:.0f} m from the keypoint polyline"
                     " — further than any scenario in the shipped routes; verify it is on"
                     " the driven path with `display --scenarios`")
    if fmt == "2.x" and not scs:
        warn(f"route {rid}: no scenarios — the ego just drives the route")

if missing_types:
    bad(f"{len(missing_types)} scenario type(s) have no class in {sr}: {', '.join(sorted(missing_types))}")
    bad("  RouteScenario SKIPS unknown types silently — this is the usual cause of"
        " 'nothing ever happens'")
elif known and any(r.findall("scenarios/scenario") for r in routes):
    ok("every scenario type resolves to a class in the paired scenario_runner")
if avail is not None:
    for t in sorted(towns):
        (ok if t in avail else bad)(f"town {t}" + ("" if t in avail else " is NOT available on this server"))

print("\ncheck " + ("OK" if rc == 0 else "FAILED"))
sys.exit(rc)
PY
  exit $?
  ;;
summary)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh summary --file routes.xml" >&2; exit 2; }
  need_server || exit 4
  exec "${PYTHON}" "${S}/route_summarizer.py" -f "${FILE}" --show \
       --host "${CARLA_HOST}" --port "${CARLA_PORT}" "${EXTRA[@]}"
  ;;
display)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh display --file routes.xml [--route 0]" >&2; exit 2; }
  need_server || exit 4
  A=(-f "${FILE}" --host "${CARLA_HOST}" --port "${CARLA_PORT}")
  [ -n "${ROUTE}" ] && A+=(-sr "${ROUTE}")
  exec "${PYTHON}" "${S}/route_displayer.py" "${A[@]}" "${EXTRA[@]}"
  ;;
create)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh create --file routes.xml" >&2; exit 2; }
  need_server || exit 4
  echo "[rt] interactive: move the SPECTATOR in the CARLA window, type commands here"
  exec "${PYTHON}" "${S}/route_creator.py" -f "${FILE}" \
       --host "${CARLA_HOST}" --port "${CARLA_PORT}" "${EXTRA[@]}"
  ;;
scenarios)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh scenarios --file routes.xml" >&2; exit 2; }
  need_server || exit 4
  echo "[rt] interactive: position the spectator at each trigger point"
  exec "${PYTHON}" "${S}/scenario_creator.py" -f "${FILE}" \
       --host "${CARLA_HOST}" --port "${CARLA_PORT}" "${EXTRA[@]}"
  ;;
order)
  [ -n "${FILE}" ] || { echo "usage: bash route_tools.sh order --file routes.xml" >&2; exit 2; }
  need_server || exit 4
  exec "${PYTHON}" "${S}/scenario_orderer.py" -f "${FILE}" \
       --host "${CARLA_HOST}" --port "${CARLA_PORT}" "${EXTRA[@]}"
  ;;
weather)
  need_server || exit 4
  A=(--host "${CARLA_HOST}" --port "${CARLA_PORT}")
  [ -n "${ROUTE}" ] && A+=(-r "${ROUTE}")
  exec "${PYTHON}" "${S}/weather_creator.py" "${A[@]}" "${EXTRA[@]}"
  ;;
bridge)
  # route_bridge.py exists on `master` only; it was dropped from the 2.0/2.1 branches.
  if [ ! -f "${S}/route_bridge.py" ]; then
    echo "ERROR: ${S}/route_bridge.py does not exist on this branch ($(carla_lb_branch))." >&2
    echo "       It ships on leaderboard 'master' only. Clone a master checkout, run it" >&2
    echo "       there, and use the resulting 2.x route file here." >&2
    exit 5
  fi
  exec "${PYTHON}" "${S}/route_bridge.py" "${EXTRA[@]}"
  ;;
*)
  echo "usage: bash route_tools.sh {check|summary|display|create|scenarios|order|weather|bridge} [options]" >&2
  exit 2
  ;;
esac
