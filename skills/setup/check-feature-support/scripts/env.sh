#!/usr/bin/env bash
# Self-contained environment for the check-feature-support skill.
# Source before the other scripts:  source env.sh
#
# This skill answers "is feature X usable here, and does this collection have a
# vetted procedure for it". It builds nothing, launches nothing and changes
# nothing, so it needs no CARLA — but it reports much more when a checkout or a
# running server is visible.
#
#   CARLA_UE58_ROOT / CARLA_UE5_ROOT / CARLA_UE4_ROOT / CARLA_TARGET
#                       whichever of these are set get inspected; all optional
#   CARLA_HOST / CARLA_PORT
#                       a running server to query (default 127.0.0.1:2000)
#   CARLA_PRESET        CMake preset whose cache is read (default Release)
#
# Sets no shell options: this file is sourced.

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export PYTHON="${PYTHON:-python3}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_PRESET="${CARLA_PRESET:-Release}"

# Every root is optional; empty means "not offered for inspection".
export CARLA_UE58_ROOT="${CARLA_UE58_ROOT:-}"
export CARLA_UE5_ROOT="${CARLA_UE5_ROOT:-}"
export CARLA_UE4_ROOT="${CARLA_UE4_ROOT:-}"
export CARLA_TARGET="${CARLA_TARGET:-}"

# Read one option out of a CMake cache without configuring anything. The cache is
# the only honest answer to "was this built with X": every -D option has to be
# repeated on each re-configure, so the source tree cannot tell you.
carla_cmake_opt() {
  local root="$1" name="$2"
  local cache="${root}/Build/${CARLA_PRESET}/CMakeCache.txt"
  [ -f "${cache}" ] || return 1
  local line
  line="$(grep -m1 "^${name}:" "${cache}" 2>/dev/null)" || return 1
  printf '%s' "${line#*=}"
}

# First root that looks like a UE5-era CARLA checkout, so callers can degrade
# gracefully instead of demanding a specific variable.
carla_any_root() {
  local r
  for r in "${CARLA_UE58_ROOT}" "${CARLA_UE5_ROOT}" "${CARLA_UE4_ROOT}"; do
    [ -n "${r}" ] && [ -d "${r}" ] && { printf '%s' "${r}"; return 0; }
  done
  return 1
}
