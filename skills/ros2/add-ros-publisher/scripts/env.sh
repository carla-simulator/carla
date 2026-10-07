#!/usr/bin/env bash
# Self-contained environment for the add-ros-publisher skill.
# Source before the other scripts:  source env.sh
#
# This skill edits C++ sources inside a CARLA checkout: the ros2 publisher /
# subscriber layer, its dispatch in ROS2.cpp, and the call sites in the UE4
# plugin. It never runs a simulator.
#
#   CARLA_UE4_ROOT  the carla checkout whose LibCarla/source/carla/ros2 to extend
#   CARLA_MAKE_JOBS parallelism for the verify build (defaults to nproc)
#
# Sets no shell options on purpose: this file is sourced.

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

_KEEP_CARLA_ROOT="${CARLA_UE4_ROOT:-}"
if [ -n "${CARLA_ENV_ACTIVATE:-}" ] && [ -f "${CARLA_ENV_ACTIVATE}" ]; then
  # shellcheck disable=SC1090
  source "${CARLA_ENV_ACTIVATE}"
fi
[ -n "${_KEEP_CARLA_ROOT}" ] && CARLA_UE4_ROOT="${_KEEP_CARLA_ROOT}"

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
export CARLA_MAKE_JOBS="${CARLA_MAKE_JOBS:-$(nproc 2>/dev/null || echo 4)}"

# The places this skill touches.
export CARLA_ROS2_SRC="${CARLA_UE4_ROOT}/LibCarla/source/carla/ros2"
export CARLA_ROS2_PUBLISHERS="${CARLA_ROS2_SRC}/publishers"
export CARLA_ROS2_SUBSCRIBERS="${CARLA_ROS2_SRC}/subscribers"
export CARLA_ROS2_MIDDLEWARE="${CARLA_ROS2_SRC}/middleware"
export CARLA_ROS2_DISPATCH="${CARLA_ROS2_SRC}/ROS2.cpp"
export CARLA_ROS2_DISPATCH_H="${CARLA_ROS2_SRC}/ROS2.h"
export CARLA_PLUGIN_SENSORS="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Plugins/Carla/Source/Carla/Sensor"
export CARLA_MIDDLEWARE_TEST="${CARLA_UE4_ROOT}/LibCarla/source/test/server/test_ros2_middleware.cpp"

unset _KEEP_CARLA_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] ros2 sources    = ${CARLA_ROS2_SRC}"
