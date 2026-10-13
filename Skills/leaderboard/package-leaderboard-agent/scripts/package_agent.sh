#!/usr/bin/env bash
# Build and verify the leaderboard submission docker image.
#
#   bash package_agent.sh build [--ros melodic|noetic|foxy]
#   bash package_agent.sh verify                 imports + agent class INSIDE the image
#   bash package_agent.sh shell                  interactive shell in the image
#   bash package_agent.sh run [evaluator args]   full dry run against your own server
#
# Env: TEAM_CODE_ROOT (required), TEAM_AGENT, CHALLENGE_TRACK_CODENAME, IMAGE
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

MODE="${1:-}"; shift || true
IMAGE="${IMAGE:-leaderboard-user}"
ROS_DISTRO_ARG=""

while [ $# -gt 0 ]; do
  case "$1" in
    --ros) ROS_DISTRO_ARG="$2"; IMAGE="leaderboard-user:ros-$2"; shift 2 ;;
    *) break ;;
  esac
done

command -v docker >/dev/null || { echo "ERROR: docker not found" >&2; exit 2; }

case "${MODE}" in
build)
  for v in CARLA_ROOT SCENARIO_RUNNER_ROOT LEADERBOARD_ROOT TEAM_CODE_ROOT; do
    eval "val=\${$v:-}"
    [ -n "${val}" ] || { echo "ERROR: $v is not set — make_docker.sh checks all four" >&2; exit 3; }
    [ -d "${val}" ] || { echo "ERROR: $v=${val} is not a directory" >&2; exit 3; }
  done
  if [ -n "${ROS_DISTRO_ARG}" ]; then
    case "${ROS_DISTRO_ARG}" in melodic|noetic|foxy) ;; *)
      echo "ERROR: make_docker.sh accepts only melodic, noetic or foxy" >&2; exit 3 ;; esac
    [ -n "${CARLA_ROS_BRIDGE_ROOT:-}" ] || {
      echo "ERROR: CARLA_ROS_BRIDGE_ROOT must be set for the ROS image" >&2; exit 3; }
  fi

  # make_docker.sh renames $CARLA_ROOT/PythonAPI/carla/dist/carla*-py3*.egg to
  # carla-leaderboard-py3x.egg. No egg -> the mv fails and the image ends up with a
  # PYTHONPATH pointing at a file that does not exist.
  if ! ls "${CARLA_ROOT}"/PythonAPI/carla/dist/carla*.egg >/dev/null 2>&1; then
    echo "ERROR: no .egg in ${CARLA_ROOT}/PythonAPI/carla/dist/" >&2
    echo "       make_docker.sh renames the egg to carla-leaderboard-py3x.egg and the" >&2
    echo "       image's PYTHONPATH depends on it. A wheel-only CARLA tree cannot be" >&2
    echo "       packaged as-is — use a release tarball or a source build for CARLA_ROOT." >&2
    exit 4
  fi
  echo "[pkg] eggs: $(ls "${CARLA_ROOT}"/PythonAPI/carla/dist/carla*.egg | xargs -n1 basename | tr '\n' ' ')"

  SZ="$(du -sm "${TEAM_CODE_ROOT}" 2>/dev/null | awk '{print $1}')"
  echo "[pkg] TEAM_CODE_ROOT is ${SZ} MB — it is copied WHOLESALE into the image"
  [ "${SZ:-0}" -gt 5000 ] && echo "[pkg] WARNING over 5 GB; strip datasets, checkpoints and .git"

  DF="${LEADERBOARD_ROOT}/scripts/Dockerfile.master"
  [ -n "${ROS_DISTRO_ARG}" ] && DF="${LEADERBOARD_ROOT}/scripts/Dockerfile.ros"
  if [ -f "${DF}" ]; then
    echo "[pkg] agent baked into ${DF##*/}:"
    grep -nE '^ENV (TEAM_AGENT|TEAM_CONFIG|CHALLENGE_TRACK_CODENAME|ROUTES)' "${DF}" | sed 's/^/[pkg]   /'
    echo "[pkg] edit those lines to point at YOUR agent before submitting"
  else
    echo "[pkg] WARNING ${DF} not found"
  fi

  cd "${LEADERBOARD_ROOT}"
  if [ -n "${ROS_DISTRO_ARG}" ]; then
    bash scripts/make_docker.sh -r "${ROS_DISTRO_ARG}"
  else
    bash scripts/make_docker.sh
  fi
  RC=$?
  [ ${RC} -eq 0 ] && echo "[pkg] built ${IMAGE} — now: bash package_agent.sh verify"
  exit ${RC}
  ;;

verify)
  docker image inspect "${IMAGE}" >/dev/null 2>&1 || {
    echo "ERROR: image ${IMAGE} not found — build it first" >&2; exit 5; }
  echo "[pkg] verifying ${IMAGE}"
  docker run --rm "${IMAGE}" bash -lc '
set -u
echo "--- environment ---"
for v in CARLA_ROOT SCENARIO_RUNNER_ROOT LEADERBOARD_ROOT TEAM_CODE_ROOT TEAM_AGENT \
         TEAM_CONFIG CHALLENGE_TRACK_CODENAME ROUTES PYTHONPATH; do
  eval "printf \"%-26s %s\n\" $v \"\${$v:-<unset>}\""
done
echo "--- imports ---"
python3 - <<PY
import importlib.util as u, os, sys
def have(mod):
    # find_spec imports parent packages, so a dotted name raises when the parent
    # is missing — which is the condition being tested.
    try:
        return u.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError):
        return False
rc = 0
for m in ("carla", "agents.navigation.global_route_planner",
          "srunner.scenariomanager.carla_data_provider",
          "leaderboard.utils.statistics_manager",
          "leaderboard.autoagents.autonomous_agent"):
    if not have(m):
        print(f"FAIL  {m}"); rc = 1
    else:
        print(f"PASS  {m}")
try:
    from importlib.metadata import version
    print(f"INFO  carla client {version(\"carla\")}")
except Exception:
    print("INFO  carla client version unknown (raw egg)")

agent = os.environ.get("TEAM_AGENT", "")
if not agent or not os.path.isfile(agent):
    print(f"FAIL  TEAM_AGENT={agent!r} does not exist in the image"); rc = 1
else:
    expect = os.path.basename(agent).split(".")[0].title().replace("_", "")
    src = open(agent).read()
    if f"class {expect}" in src:
        print(f"PASS  {os.path.basename(agent)} defines class {expect}")
    else:
        print(f"FAIL  {os.path.basename(agent)} must define class {expect}"
              " (the evaluator derives it from the file name)"); rc = 1

track = os.environ.get("CHALLENGE_TRACK_CODENAME", "")
try:
    from leaderboard.autoagents.autonomous_agent import Track
    names = [t.value for t in Track]
    print(("PASS  " if track in names else "FAIL  ") + f"track {track!r} (valid: {\", \".join(names)})")
    rc |= 0 if track in names else 1
except Exception as e:
    print(f"WARN  could not check the track ({e})")

routes = os.environ.get("ROUTES", "")
print(("PASS  " if routes and os.path.isfile(routes) else "WARN  ") + f"ROUTES={routes!r}")
sys.exit(rc)
PY
'
  RC=$?
  [ ${RC} -eq 0 ] && echo "[pkg] image OK" || echo "[pkg] image has problems — fix before submitting"
  exit ${RC}
  ;;

shell)
  docker image inspect "${IMAGE}" >/dev/null 2>&1 || {
    echo "ERROR: image ${IMAGE} not found" >&2; exit 5; }
  exec docker run --rm -it "${IMAGE}" bash
  ;;

run)
  docker image inspect "${IMAGE}" >/dev/null 2>&1 || {
    echo "ERROR: image ${IMAGE} not found" >&2; exit 5; }
  echo "[pkg] dry run against ${CARLA_HOST}:${CARLA_PORT} (host network)"
  echo "[pkg] the server must already be running OUTSIDE the container"
  exec docker run --rm -it --network host \
    -e CARLA_HOST="${CARLA_HOST}" -e CARLA_PORT="${CARLA_PORT}" \
    "${IMAGE}" bash -lc "python3 \${LEADERBOARD_ROOT}/leaderboard/leaderboard_evaluator.py \
      --routes=\${ROUTES} --agent=\${TEAM_AGENT} --track=\${CHALLENGE_TRACK_CODENAME} \
      --checkpoint=/workspace/results/results.json --debug=1 \
      --host=${CARLA_HOST} --port=${CARLA_PORT} $*"
  ;;

*)
  echo "usage: bash package_agent.sh {build|verify|shell|run} [--ros DISTRO] [args]" >&2
  exit 2
  ;;
esac
