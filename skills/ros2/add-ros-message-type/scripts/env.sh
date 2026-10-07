#!/usr/bin/env bash
# Self-contained environment for the add-ros-message-type skill.
# Source before the other scripts:  source env.sh
#
# This skill edits C++ sources inside a CARLA checkout and verifies them with the
# LibCarla test suite. It never runs a simulator.
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

# The files this skill touches, all under the ros2 tree.
export CARLA_ROS2_SRC="${CARLA_UE4_ROOT}/LibCarla/source/carla/ros2"
export CARLA_ROS2_MSG_DIR="${CARLA_ROS2_SRC}/types/msg"
export CARLA_CDR_SERIALIZATION="${CARLA_ROS2_SRC}/types/CdrSerialization.h"
export CARLA_CDR_TOPIC_INFO="${CARLA_ROS2_SRC}/types/CdrTopicInfo.h"
export CARLA_TYPE_HASH_TEST="${CARLA_UE4_ROOT}/LibCarla/source/test/server/test_type_hash.cpp"
export CARLA_HASH_TOOL="${CARLA_UE4_ROOT}/Util/ros2/compute_type_hash.sh"

unset _KEEP_CARLA_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] ros2 sources    = ${CARLA_ROS2_SRC}"
