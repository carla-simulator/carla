#!/usr/bin/env bash
# Environment for the document-carla-feature skill.  source env.sh
#
# Unlike most skills here, this one edits the repository it ships in rather
# than acting on an external CARLA instance, so there is no built engine or
# running server to resolve. The only root it needs is the carla checkout:
#
#   CARLA_ROOT   the carla checkout to document
#
# Resolution order: an explicit export, then $PWD if it looks like a checkout,
# then the repo this script lives in (Skills/docs/document-carla-feature/scripts ->
# four levels up). Never a bare parents[n] guess alone.

# Sets no shell options on purpose: this file is sourced, and `set -e` here
# would change its callers' error semantics. Each script sets its own.

is_carla_checkout() {
  [ -f "$1/CHANGELOG.md" ] && [ -d "$1/Docs" ] && [ -d "$1/PythonAPI/docs" ]
}

if [ -z "${CARLA_ROOT:-}" ]; then
  if is_carla_checkout "$PWD"; then
    CARLA_ROOT="$PWD"
  else
    _self="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    _guess="$(cd "${_self}/../../../.." && pwd)"
    if is_carla_checkout "${_guess}"; then
      CARLA_ROOT="${_guess}"
    fi
  fi
fi
export CARLA_ROOT

export PYTHON="${PYTHON:-python3}"

echo "[env] CARLA_ROOT = ${CARLA_ROOT:-<unresolved>}"
echo "[env] PYTHON     = ${PYTHON}"
