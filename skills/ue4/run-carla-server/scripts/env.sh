#!/usr/bin/env bash
# Self-contained environment for the run-carla-server skill.
# Source this before the skill's scripts:  source env.sh
#
# carla-agentic-tools is a STANDALONE repo — it does not live inside a CARLA
# checkout — so the target instance is chosen at runtime. Both roots below are
# overridable and must point at a real, built CARLA + UE4:
#
#   CARLA_UE4_ROOT  the carla source checkout (branch ue4-dev) to serve
#   UE4_ROOT        the built CarlaUnreal UE 4.26 fork (uncooked modes launch it)
#
# No environment-manager assumption is made: the verify client uses whatever
# `python3` your active CARLA client env provides.

# Sets no shell options on purpose: this file is sourced, and `set -e` here
# would change its callers' error semantics. Each script sets its own.

# --- Optional environment hook ----------------------------------------------
# CARLA_ENV_ACTIVATE, when set, names an activation script to source into this
# (possibly non-interactive) shell — the one hook for driving this skill without
# an already-active environment. Unset, it is a silent no-op. Nothing else is
# detected: no environment-manager binary is probed, no dotfile is searched for.
# Roots the caller exported explicitly outrank anything that script sets.
# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

_KEEP_CARLA_ROOT="${CARLA_UE4_ROOT:-}" _KEEP_UE4_ROOT="${UE4_ROOT:-}"
if [ -n "${CARLA_ENV_ACTIVATE:-}" ] && [ -f "${CARLA_ENV_ACTIVATE}" ]; then
  # shellcheck disable=SC1090
  source "${CARLA_ENV_ACTIVATE}"
fi
[ -n "${_KEEP_CARLA_ROOT}" ] && CARLA_UE4_ROOT="${_KEEP_CARLA_ROOT}"
[ -n "${_KEEP_UE4_ROOT}" ] && UE4_ROOT="${_KEEP_UE4_ROOT}"

# --- Resolve the target CARLA checkout --------------------------------------
# Precedence: explicit CARLA_UE4_ROOT  >  $PWD if it is a checkout  >  the
# path-derived guess (only meaningful when this repo was dropped INTO a checkout).
_SKILL_SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_DERIVED_ROOT="$(cd "${_SKILL_SCRIPTS_DIR}/../../../../.." && pwd)"
_UPROJECT_REL="Unreal/CarlaUE4/CarlaUE4.uproject"

if [ -z "${CARLA_UE4_ROOT:-}" ]; then
  if [ -f "${PWD}/${_UPROJECT_REL}" ]; then
    CARLA_UE4_ROOT="${PWD}"
  elif [ -f "${_DERIVED_ROOT}/${_UPROJECT_REL}" ]; then
    CARLA_UE4_ROOT="${_DERIVED_ROOT}"
  fi
fi
export CARLA_UE4_ROOT="${CARLA_UE4_ROOT:-}"

# UE4_ROOT has no derivable default from a standalone repo — export it, or let
# check_env.sh fail loudly with the path it looked for. Only the uncooked modes
# (default / WINDOW=1) need it; a package runs without it.
export UE4_ROOT="${UE4_ROOT:-}"

# --- What are we being asked to run? ----------------------------------------
# The user names ONE path and the skill works out what it is. Three shapes exist:
#
#   package   an extracted release: <path>/CarlaUE4.sh          (no UE4 needed)
#   dist      a checkout that cooked one: <path>/Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh
#   editor    a source checkout: <path>/Unreal/CarlaUE4/CarlaUE4.uproject (needs UE4_ROOT)
#
# CARLA_TARGET accepts any of the three; CARLA_PACKAGE_ROOT names a release
# explicitly; CARLA_UE4_ROOT keeps its old meaning (a checkout). Detection order
# below prefers the most specific thing the user pointed at.
export CARLA_TARGET="${CARLA_TARGET:-}"
export CARLA_PACKAGE_ROOT="${CARLA_PACKAGE_ROOT:-}"

# Echoes "<mode> <path-to-launch>" where mode is package|dist|editor, or
# "none ." when nothing runnable was found, or "invalid <path>" when a path the
# user named explicitly holds no CARLA. `path` is the CarlaUE4.sh for
# package/dist and the checkout root for editor.
#
# $1 restricts what counts as a hit: "package" ignores source checkouts, "editor"
# ignores cooked builds. Needed because a checkout that has cooked a package can
# be run either way, and detection alone cannot know which the user meant.
carla_detect_target() {
  local want="${1:-auto}"
  # Literal, not ${_UPROJECT_REL}: that variable is unset at the end of this file,
  # and this function runs later, from the caller.
  local uproject="Unreal/CarlaUE4/CarlaUE4.uproject"
  local cand explicit
  # An EXPLICIT target that turns out to hold no CARLA is an error, not a reason to
  # quietly run something else: silently serving a different build than the user
  # named is the worst outcome available here.
  for explicit in "${CARLA_TARGET}" "${CARLA_PACKAGE_ROOT}"; do
    [ -n "${explicit}" ] || continue
    if [ ! -d "${explicit}" ]; then echo "invalid ${explicit}"; return 0; fi
  done
  for cand in "${CARLA_TARGET}" "${CARLA_PACKAGE_ROOT}" "${CARLA_UE4_ROOT}" "${PWD}"; do
    [ -n "${cand}" ] || continue
    [ -d "${cand}" ] || continue
    # A standalone release: the launcher sits at the top level.
    if [ "${want}" != "editor" ] && [ -x "${cand}/CarlaUE4.sh" ]; then
      echo "package ${cand}/CarlaUE4.sh"; return 0
    fi
    # Some releases keep the LinuxNoEditor/ subdir when extracted.
    if [ "${want}" != "editor" ] && [ -x "${cand}/LinuxNoEditor/CarlaUE4.sh" ]; then
      echo "package ${cand}/LinuxNoEditor/CarlaUE4.sh"; return 0
    fi
    # A checkout that has cooked its own package (newest wins).
    local pkg
    pkg="$(ls -1dt "${cand}"/Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh 2>/dev/null | head -1 || true)"
    if [ "${want}" != "editor" ] && [ -n "${pkg}" ] && [ -x "${pkg}" ]; then
      echo "dist ${pkg}"; return 0
    fi
    # A source checkout: run the editor.
    if [ "${want}" != "package" ] && [ -f "${cand}/${uproject}" ]; then
      echo "editor ${cand}"; return 0
    fi
    # Reached only when this candidate held nothing runnable. If the user named it
    # explicitly, stop here instead of falling through to another install.
    for explicit in "${CARLA_TARGET}" "${CARLA_PACKAGE_ROOT}"; do
      if [ -n "${explicit}" ] && [ "${cand}" = "${explicit}" ]; then
        echo "invalid ${cand}"; return 0
      fi
    done
  done
  echo "none ."
  return 0
}

# --- ROS 2 native interface (opt-in, runtime) -------------------------------
# ROS2=1 starts the server with `--ros2`, so it publishes DDS topics itself (no
# carla-ros-bridge). This only WORKS on a binary that was BUILT with ROS 2
# ([[build-carla-ue4]] / [[package-carla-ue4]] ROS2=1) — a plain binary accepts
# the flag and ignores it, with no error.
#
#   RMW             fastdds (server default) | cyclonedds | zenoh -> --rmw=
#                   zenoh additionally needs a router process (references/ros2.md)
#   ROS_DOMAIN_ID   0..232 -> --ros-domain-id=; unset lets the server resolve it
#                   from the ROS_DOMAIN_ID env var, then the default domain 0.
#                   Subscribers must be on the SAME domain or see no topics.
export ROS2="${ROS2:-0}"
export RMW="${RMW:-}"

# Build-time ROS 2 state of the checkout (source builds only; a Dist/ package
# carries whatever it was cooked with and this file no longer applies to it).
# BuildCarlaUE4.sh writes all module flags on ONE space-separated line.
# Echoes: on | off | absent.
carla_ros2_ini_state() {
  local ini="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Config/OptionalModules.ini"
  [ -f "${ini}" ] || { echo absent; return 0; }
  grep -q 'Ros2 ON' "${ini}" && echo on || echo off
}

# The ROS 2 runtime flags, as the server's own CarlaSettings parser expects them:
# FParse::Param(TEXT("-ros2")) matches the token "--ros2" (double dash), and
# FParse::Value(TEXT("-rmw=")) / TEXT("-ros-domain-id=") match "--rmw=<v>" /
# "--ros-domain-id=<n>". Echoes nothing when ROS2 != 1.
carla_ros2_flags() {
  [ "${ROS2}" = "1" ] || return 0
  printf '%s' "--ros2"
  [ -n "${RMW}" ] && printf ' --rmw=%s' "${RMW}"
  [ -n "${ROS_DOMAIN_ID:-}" ] && printf ' --ros-domain-id=%s' "${ROS_DOMAIN_ID}"
  return 0
}

unset _KEEP_CARLA_ROOT _KEEP_UE4_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] UE4_ROOT        = ${UE4_ROOT:-<unset — needed for uncooked modes>}"
if [ "${ROS2}" = "1" ]; then
  echo "[env] ROS2            = 1  (server flags: $(carla_ros2_flags))"
else
  echo "[env] ROS2            = 0  (set ROS2=1 for the native ROS 2 interface)"
fi
