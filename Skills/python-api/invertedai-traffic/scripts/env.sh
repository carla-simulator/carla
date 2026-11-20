#!/usr/bin/env bash
# Self-contained environment for the invertedai-traffic skill (RUNTIME/client).
# Source before the skill's other scripts:  source env.sh
#
# Drives NPC traffic on a *running* CARLA server with the Inverted AI API. Needs
# the `invertedai` package, which lives in its own venv because the pinned SDK
# forces numpy 1.26, and an Inverted AI API key.
#
#   CARLA_HOST     server address                          (default 127.0.0.1)
#   CARLA_PORT     server RPC port                         (default 2000)
#   IAI_VENV       venv holding invertedai                 (default ~/.venvs/carla-iai)
#   IAI_PY         its interpreter                         (default $IAI_VENV/bin/python)
#   IAI_ENV_FILE   KEY=value file read for IAI_API_KEY / IAI_VENV
#                  (default: .env at the root of the tree holding this skill)
#   IAI_API_KEY    the key. An exported value wins over the file.
#
# The key is never printed. Only its source and its last four characters are.

set -euo pipefail

_IAI_HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See Skills/_common/env_common.sh.
. "${_IAI_HERE}/../../../_common/env_common.sh"

# The tree this skill ships in: a source checkout or a package root. Both carry
# PythonAPI/examples/invertedai_traffic.py next to Skills/.
_IAI_TREE="$(cd "${_IAI_HERE}/../../../.." && pwd)"
if [ -z "${CARLA_ROOT:-}" ] && [ -f "${_IAI_TREE}/PythonAPI/examples/invertedai_traffic.py" ]; then
  export CARLA_ROOT="${_IAI_TREE}"
fi

# Pick the .env: explicit, then the skill's own tree, then CARLA_ROOT.
if [ -z "${IAI_ENV_FILE:-}" ]; then
  for _f in "${_IAI_TREE}/.env" "${CARLA_ROOT:+${CARLA_ROOT}/.env}"; do
    if [ -n "${_f}" ] && [ -f "${_f}" ]; then IAI_ENV_FILE="${_f}"; break; fi
  done
fi
export IAI_ENV_FILE="${IAI_ENV_FILE:-${_IAI_TREE}/.env}"

# Literal KEY=value parse, deliberately not `source`: the file must not be able
# to run commands. Only the keys this skill uses are read, and an exported
# value is never overwritten. Optional `export ` prefix and quotes are allowed.
IAI_KEY_SOURCE="environment"
if [ -f "${IAI_ENV_FILE}" ]; then
  while IFS= read -r _line || [ -n "${_line}" ]; do
    _line="${_line%$'\r'}"
    case "${_line}" in ''|'#'*) continue ;; esac
    _line="${_line#export }"
    _key="${_line%%=*}"; _val="${_line#*=}"
    _key="${_key//[[:space:]]/}"
    case "${_val}" in \"*\") _val="${_val:1:${#_val}-2}" ;; \'*\') _val="${_val:1:${#_val}-2}" ;; esac
    case "${_key}" in
      IAI_API_KEY)
        if [ -z "${IAI_API_KEY:-}" ] && [ -n "${_val}" ]; then
          export IAI_API_KEY="${_val}"; IAI_KEY_SOURCE="${IAI_ENV_FILE}"
        fi ;;
      IAI_VENV) [ -n "${IAI_VENV:-}" ] || export IAI_VENV="${_val/#\~/${HOME}}" ;;
    esac
  done < "${IAI_ENV_FILE}"
fi
[ -n "${IAI_API_KEY:-}" ] || IAI_KEY_SOURCE="none"
export IAI_KEY_SOURCE

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export IAI_VENV="${IAI_VENV:-${HOME}/.venvs/carla-iai}"
export IAI_PY="${IAI_PY:-${IAI_VENV}/bin/python}"
export PYTHONIOENCODING=utf-8

if [ -n "${IAI_API_KEY:-}" ]; then
  _masked="…${IAI_API_KEY: -4}"
else
  _masked="(not set)"
fi
echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}"
echo "[env] IAI_PY          = ${IAI_PY}"
echo "[env] IAI_API_KEY     = ${_masked}  (from ${IAI_KEY_SOURCE})"
unset _line _key _val _f _masked
