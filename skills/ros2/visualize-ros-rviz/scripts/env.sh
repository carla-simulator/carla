#!/usr/bin/env bash
# Self-contained environment for the visualize-ros-rviz skill.
# Source before the other scripts:  source env.sh
#
# This skill drives the ROS 2 tooling that ships INSIDE a CARLA checkout
# (PythonAPI/examples/ros2), from Docker images built on demand. It needs no
# local ROS 2 installation and never launches a simulator.
#
#   CARLA_UE4_ROOT  the carla checkout holding PythonAPI/examples/ros2
#   ROS_DISTRO_TAG  humble (default) | jazzy      -> image tag + python tag
#   RMW             fastdds (default) | cyclonedds | zenoh
#   ROS_DOMAIN_ID   0..232; must match the server's --ros-domain-id
#   CARLA_HOST/CARLA_PORT  the running server the demo stack connects to
#
# Sets no shell options on purpose: this file is sourced.

# --- Optional environment hook ----------------------------------------------
# CARLA_ENV_ACTIVATE, when set, names an activation script to source into this
# (possibly non-interactive) shell. Unset, it is a silent no-op. Roots the caller
# exported explicitly outrank anything that script sets.
# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

_KEEP_CARLA_ROOT="${CARLA_UE4_ROOT:-}"
if [ -n "${CARLA_ENV_ACTIVATE:-}" ] && [ -f "${CARLA_ENV_ACTIVATE}" ]; then
  # shellcheck disable=SC1090
  source "${CARLA_ENV_ACTIVATE}"
fi
[ -n "${_KEEP_CARLA_ROOT}" ] && CARLA_UE4_ROOT="${_KEEP_CARLA_ROOT}"

# --- Resolve the target CARLA checkout --------------------------------------
# Precedence: explicit CARLA_UE4_ROOT > $PWD if it is a checkout > path-derived
# guess (only meaningful when this repo was dropped INTO a checkout).
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

# The in-checkout demo directory this skill drives. Everything below lives there.
export CARLA_ROS2_EXAMPLES="${CARLA_UE4_ROOT}/PythonAPI/examples/ros2"

# --- Middleware / distro / domain -------------------------------------------
# ROS_DISTRO is a standard variable a sourced ROS setup.bash would own, so the
# knob is named ROS_DISTRO_TAG to avoid clashing with it.
export ROS_DISTRO_TAG="${ROS_DISTRO_TAG:-humble}"
export RMW="${RMW:-fastdds}"

# The RViz base image is what `ros2` CLI calls run in; the demo image extends it
# with the carla wheel. Names are fixed by the in-checkout scripts.
export CARLA_RVIZ_IMAGE="carla-rviz-${ROS_DISTRO_TAG}-${RMW}"
export CARLA_DEMO_IMAGE="carla-map-and-lidar-demo-${ROS_DISTRO_TAG}-${RMW}"

# Python tag of the wheel the demo image needs — fixed by each distro's system
# python (humble: 3.10, jazzy: 3.12).
case "${ROS_DISTRO_TAG}" in
  humble) export CARLA_WHEEL_TAG="cp310" ;;
  jazzy)  export CARLA_WHEEL_TAG="cp312" ;;
  *)      export CARLA_WHEEL_TAG="" ;;
esac

# rmw_zenoh_cpp / rmw_cyclonedds_cpp / rmw_fastrtps_cpp — what the containers set
# as RMW_IMPLEMENTATION.
case "${RMW}" in
  zenoh)      export RMW_IMPLEMENTATION="rmw_zenoh_cpp" ;;
  cyclonedds) export RMW_IMPLEMENTATION="rmw_cyclonedds_cpp" ;;
  *)          export RMW_IMPLEMENTATION="rmw_fastrtps_cpp" ;;
esac

export CARLA_HOST="${CARLA_HOST:-localhost}"
export CARLA_PORT="${CARLA_PORT:-2000}"

unset _KEEP_CARLA_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] distro/rmw      = ${ROS_DISTRO_TAG}/${RMW} (${RMW_IMPLEMENTATION})"
echo "[env] domain          = ${ROS_DOMAIN_ID:-<default 0 — must match the server>}"
echo "[env] server          = ${CARLA_HOST}:${CARLA_PORT}"
