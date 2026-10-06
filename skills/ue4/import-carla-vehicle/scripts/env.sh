#!/usr/bin/env bash
# Self-contained environment for the import-carla-vehicle skill.
# Source before the skill's other scripts:  source env.sh
#
# carla-agentic-tools is standalone (not inside a CARLA checkout), so the target
# instance is chosen at runtime. Both roots are overridable and must point at a
# real, built CARLA + UE4:
#
#   CARLA_UE4_ROOT  the carla source checkout to import into
#   UE4_ROOT        the built CarlaUnreal UE 4.26 fork (runs the editor)
#
# The importer drives UE4Editor directly, so no Python environment is needed to
# IMPORT a vehicle. The client wheel is only needed to VERIFY one.
#
# Unlike the prop importer this skill boots the editor TWICE, in two different
# modes, because the two halves of the job need different engine states:
#
#   -run=pythonscript          mesh + physics + blueprint  (fast, pre-init)
#   -ExecutePythonScript=...   VehicleFactory registration  (full editor)
#
# Registration runs CARLA's own CarlaTools script, which compiles and saves the
# factory blueprint through Kismet — and a commandlet cannot compile blueprints.
# See references/walker_import.md, C1 and C6.

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
# path-derived guess (only meaningful when this repo sits inside a checkout).
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

# UE4_ROOT has no derivable default — export it, or let check_env.sh fail loudly
# with the path it looked for.
export UE4_ROOT="${UE4_ROOT:-}"

# --- Optional Python pin (verification only) --------------------------------
# Set CARLA_PY_VERSION only to force a specific minor (e.g. 3.10) for the client
# python that runs verify_vehicle.py. It must resolve as `python<pin>` INSIDE the
# active env, and that interpreter is the one that must import `carla`.
export CARLA_PY_VERSION="${CARLA_PY_VERSION:-}"

# --- The vehicle asset contract ---------------------------------------------
# A CARLA 4-wheeled vehicle needs a rig with the canonical wheel bones: the wheel
# bodies, the anim blueprint and the PxVehicle wheel setups are all addressed by
# these names, and UVehicleAuthoringLibrary::SetupVehiclePhysicsAsset hardcodes them.
export CARLA_VEHICLE_FACTORY="${CARLA_VEHICLE_FACTORY:-/Game/Carla/Blueprints/Vehicles/VehicleFactory}"
# VehicleFactory's blueprint MEMBER variable holding TArray<FVehicleParameters>.
export CARLA_VEHICLE_FACTORY_ARRAY="${CARLA_VEHICLE_FACTORY_ARRAY:-Vehicles}"
# Donor vehicle blueprint duplicated to make a new one. It supplies the inherited
# native components (Mesh, VehicleMovement) whose override slots serialise.
export CARLA_VEHICLE_DONOR_BP="${CARLA_VEHICLE_DONOR_BP:-/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance}"
# Donor wheel blueprints, front-left, front-right, rear-left, rear-right.
export CARLA_VEHICLE_DONOR_WHEELS="${CARLA_VEHICLE_DONOR_WHEELS:-/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance_FLW,/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance_FRW,/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance_RLW,/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance_RRW}"
# Animation blueprint template; CreateVehicleAnimBP duplicates and retargets it.
export CARLA_VEHICLE_DONOR_ANIM_BP="${CARLA_VEHICLE_DONOR_ANIM_BP:-/Game/Carla/Static/Truck/Ambulance/AnimBP_Ambulance}"

unset _KEEP_CARLA_ROOT _KEEP_UE4_ROOT _SKILL_SCRIPTS_DIR _DERIVED_ROOT _UPROJECT_REL

echo "[env] CARLA_UE4_ROOT  = ${CARLA_UE4_ROOT:-<unset — export it>}"
echo "[env] UE4_ROOT        = ${UE4_ROOT:-<unset — export it>}"
