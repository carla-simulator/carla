#!/usr/bin/env bash
# Runs CARLA's full Inverted AI example, PythonAPI/examples/invertedai_traffic.py,
# with the key from .env and the IAI venv, and cleans up after it.
#
#   run_example.sh [OUT_DIR] [extra args for invertedai_traffic.py]
#   N=30 W=10 SEED=7 LEN=60 run_example.sh ./iai_out --record --iai-log
#   CLEAN=1 run_example.sh ...      also destroy every vehicle/walker left (old examples)
#
# OUT_DIR (default ./iai_out) is the working directory: the example writes
# logs/carla_record.log and <timestamp>/iai_log/*.{json,gif} relative to it.
# The example ALWAYS reloads the map (carla:Town10HD by default). It destroys its
# actors and restores asynchronous mode on exit; this wrapper checks that it did,
# because a crash or an older example leaves a synchronous world nobody ticks.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
[ -n "${IAI_API_KEY:-}" ] || { echo "FAIL IAI_API_KEY not set — add it to ${IAI_ENV_FILE}"; exit 2; }
EX="${CARLA_ROOT:-}/PythonAPI/examples/invertedai_traffic.py"
[ -f "${EX}" ] || { echo "FAIL ${EX} not found — set CARLA_ROOT"; exit 2; }

OUT="${1:-./iai_out}"; [ $# -gt 0 ] && shift
mkdir -p "${OUT}"; OUT="$(cd "${OUT}" && pwd)"

restore() {
  "${IAI_PY}" "${HERE}/restore_world.py" "${CARLA_HOST}" "${CARLA_PORT}" "${CLEAN:-0}" 2>&1 \
    | grep -v "^Commercial access" || true
}
trap restore EXIT

export TQDM_DISABLE=1   # the example's progress bar floods run.txt
cd "${OUT}"
set +e
"${IAI_PY}" "${EX}" --host "${CARLA_HOST}" --port "${CARLA_PORT}" \
  -n "${N:-30}" -w "${W:-0}" ${SEED:+--seed "${SEED}"} --sim-length "${LEN:-30}" "$@" 2>&1 \
  | grep --line-buffered -v "^Commercial access" | tee "${OUT}/run.txt"
rc=${PIPESTATUS[0]}
set -e
# The example catches its own exceptions and prints them, so rc is 0 even on
# failure. "KeyError: '4378'" (older examples print a bare '4378') comes from the
# SDK's GIF renderer and is harmless when the logs below exist.
if grep -qE "Error|Exception|Traceback|^'[0-9]+'$" "${OUT}/run.txt"; then
  echo "[run_example] WARN the example printed an error (see ${OUT}/run.txt)"
fi
[ -f "${OUT}/logs/carla_record.log" ] && echo "[run_example] CARLA recording: ${OUT}/logs/carla_record.log"
for d in "${OUT}"/*/iai_log; do [ -d "${d}" ] && echo "[run_example] Inverted AI log: ${d}"; done
exit "${rc}"
