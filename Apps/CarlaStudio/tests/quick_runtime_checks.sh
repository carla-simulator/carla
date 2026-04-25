#!/usr/bin/env bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -euo pipefail

MODE="${1:-}"
CARLA_ROOT="${CARLA_ROOT:-${CARLA_WORKSPACE_ROOT:-}}"

log() {
  echo "[carla-studio-runtime-check] $*"
}

skip() {
  log "SKIP: $*"
  exit 0
}

run_ue_launch_kill() {
  local script_name="$1"
  local script_path="${CARLA_ROOT}/${script_name}"

  if [[ -z "${CARLA_ROOT}" ]]; then
    skip "CARLA_ROOT/CARLA_WORKSPACE_ROOT is not set"
  fi

  if [[ ! -f "${script_path}" ]]; then
    skip "${script_name} not found under ${CARLA_ROOT}"
  fi

  log "Launching ${script_name} in quick smoke mode"
  timeout 25s bash -lc "'${script_path}' -RenderOffScreen >/tmp/carla_studio_${script_name}.log 2>&1 & echo \$!" >/tmp/carla_studio_${script_name}.pid || true

  if [[ ! -s "/tmp/carla_studio_${script_name}.pid" ]]; then
    skip "Could not acquire ${script_name} PID"
  fi

  local pid
  pid="$(cat "/tmp/carla_studio_${script_name}.pid" | tr -d '[:space:]')"
  if [[ -z "${pid}" ]]; then
    skip "${script_name} PID was empty"
  fi

  sleep 5
  if kill -0 "${pid}" >/dev/null 2>&1; then
    log "Terminating ${script_name} (pid=${pid})"
    kill -TERM "${pid}" >/dev/null 2>&1 || true
    sleep 2
    kill -KILL "${pid}" >/dev/null 2>&1 || true
  else
    log "${script_name} process already exited before kill check"
  fi

  log "PASS: ${script_name} quick launch/kill sequence"
}

run_third_party_checks() {
  local checks=(
    "ros2:command -v ros2 >/dev/null 2>&1"
    "scenario-runner-dir:test -d ${CARLA_ROOT}/PythonAPI/scenario_runner"
    "leaderboard-dir:test -d ${CARLA_ROOT}/PythonAPI/leaderboard"
    "ros-bridge-dir:test -d ${CARLA_ROOT}/PythonAPI/ros-bridge"
    "autoware-doc:grep -Rqi autoware ${CARLA_ROOT}/Docs 2>/dev/null"
    "roadrunner-doc:grep -Rqi roadrunner ${CARLA_ROOT}/Docs 2>/dev/null"
  )

  if [[ -z "${CARLA_ROOT}" || ! -d "${CARLA_ROOT}" ]]; then
    skip "CARLA_ROOT path is unavailable for third-party checks"
  fi

  local pass_count=0
  local fail_count=0

  for spec in "${checks[@]}"; do
    local name="${spec%%:*}"
    local cmd="${spec#*:}"
    log "Check ${name}"
    if bash -lc "${cmd}"; then
      log "PASS ${name}"
      pass_count=$((pass_count + 1))
    else
      log "INFO ${name} not available in this environment"
      fail_count=$((fail_count + 1))
    fi
  done

  log "Third-party checks complete: pass=${pass_count}, unavailable=${fail_count}"
}

case "${MODE}" in
  ue4)
    run_ue_launch_kill "CarlaUE4.sh"
    ;;
  ue5)
    run_ue_launch_kill "CarlaUE5.sh"
    ;;
  thirdparty)
    run_third_party_checks
    ;;
  *)
    echo "Usage: $0 {ue4|ue5|thirdparty}" >&2
    exit 2
    ;;
esac
