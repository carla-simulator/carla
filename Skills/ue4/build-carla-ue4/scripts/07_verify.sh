#!/usr/bin/env bash
# Step 07 — end-to-end verification against the FROM-SOURCE build.
#
# This skill builds source artifacts, not a Dist/ package, so verification runs
# the UNCOOKED server directly ([[run-carla-server]], `UE4Editor -game -nullrhi`)
# rather than a cooked CarlaUE4.sh. It boots that server, waits for the RPC port,
# runs a stock example from the active client env, then shuts it down — proof the
# whole toolchain (UE4 + Carla server plugin + LibCarla + bindings + content)
# works. For a cooked, sensor-capable server, package with [[package-carla-ue4]]
# and run it with PACKAGED=1.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
RPC_PORT="${RPC_PORT:-2000}"
MAP="${MAP:-/Game/Carla/Maps/Town02}"   # light map = fast first load (uncooked)
RUN_SERVER="${HERE}/../../run-carla-server/scripts/run_server.sh"

[ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ] \
  || { echo "[verify] ERROR: UE4Editor not built (step 03)."; exit 1; }
[ -n "$(find "${CARLA_UE4_ROOT}/Unreal/CarlaUE4" -name 'libUE4Editor-Carla*.so' -print -quit 2>/dev/null)" ] \
  || { echo "[verify] ERROR: Carla server plugin not built (run step 06 — make CarlaUE4Editor)."; exit 1; }
[ -d "${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla" ] \
  || echo "[verify] WARN: Content/Carla missing (step 05) — the map may be empty."
[ -f "${RUN_SERVER}" ] \
  || { echo "[verify] ERROR: run-carla-server skill not found at ${RUN_SERVER}."; exit 1; }

# Resolve the client interpreter (manager-agnostic) for the example client.
carla_require_build_python || exit 1   # sets CARLA_PY_BIN

echo "[verify] starting uncooked headless server via run-carla-server (map=${MAP})..."
bash "${RUN_SERVER}" "${MAP}" "${RPC_PORT}" >/tmp/carla_verify_server.log 2>&1 &
# Stop the SERVER by process name, not by our shell's PID: run_server.sh execs a
# new process. `pkill -x UE4Editor` is the documented shutdown (never
# `pkill -f CarlaUE4.uproject` — it would kill this script; run-carla-server P6).
trap 'echo "[verify] stopping server"; pkill -x UE4Editor 2>/dev/null || true' EXIT

echo "[verify] waiting for RPC port ${RPC_PORT} (uncooked boot ~15-20s)..."
for i in $(seq 1 90); do
  if (echo > "/dev/tcp/127.0.0.1/${RPC_PORT}") >/dev/null 2>&1; then
    echo "[verify] port up after ${i}s"; break
  fi
  sleep 1
  [ "$i" -eq 90 ] && { echo "[verify] ERROR: server never opened port ${RPC_PORT}. See /tmp/carla_verify_server.log"; exit 1; }
done

echo "[verify] running example: generate_traffic.py (10s)..."
cd "${CARLA_UE4_ROOT}/PythonAPI/examples"
timeout 30 "${CARLA_PY_BIN}" generate_traffic.py --host 127.0.0.1 --port "${RPC_PORT}" -n 10 -w 5 &
EX_PID=$!
sleep 12
kill "${EX_PID}" 2>/dev/null || true

echo "[verify] SUCCESS — from-source server accepted client + example ran."
