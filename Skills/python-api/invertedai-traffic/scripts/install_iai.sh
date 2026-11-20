#!/usr/bin/env bash
# Creates the venv the invertedai-traffic skill runs in and installs the SDK
# at the commit CARLA pins in PythonAPI/examples/requirements.txt.
#
#   install_iai.sh                 venv at $IAI_VENV (default ~/.venvs/carla-iai)
#   BASE_PY=python3.12 install_iai.sh
#
# Own venv on purpose: the pinned invertedai forces numpy 1.26, which breaks
# anything in the user site that needs numpy 2 (opencv-python, for one).
# --system-site-packages keeps the user's `carla` wheel visible inside it.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

BASE_PY="${BASE_PY:-python3}"
"${BASE_PY}" -c 'import sys; sys.exit(not ((3, 8) <= sys.version_info < (3, 13)))' || {
  echo "FAIL ${BASE_PY} is $("${BASE_PY}" -V 2>&1); invertedai needs Python 3.8-3.12. Set BASE_PY=python3.12 (or 3.10)."; exit 2; }

REQ="${CARLA_ROOT:-}/PythonAPI/examples/requirements.txt"
[ -f "${REQ}" ] || { echo "FAIL ${REQ} not found — set CARLA_ROOT to the CARLA tree"; exit 2; }
# The exact pin, without the environment marker after ';'.
PIN="$(grep -E '^invertedai' "${REQ}" | head -n1 | sed 's/[[:space:]]*;.*$//')"
[ -n "${PIN}" ] || { echo "FAIL no invertedai line in ${REQ}"; exit 2; }

if [ ! -x "${IAI_PY}" ]; then
  echo "[install] creating venv ${IAI_VENV} from $(command -v "${BASE_PY}")"
  "${BASE_PY}" -m venv --system-site-packages "${IAI_VENV}"
fi
"${IAI_PY}" -m pip install --quiet --upgrade pip
echo "[install] ${PIN}"
"${IAI_PY}" -m pip install "${PIN}" imageio tqdm
"${IAI_PY}" -c 'import invertedai, carla, numpy; print("[install] OK invertedai + carla, numpy", numpy.__version__)'

if [ "${IAI_VENV}" != "${HOME}/.venvs/carla-iai" ]; then
  echo "[install] non-default venv: add  IAI_VENV=${IAI_VENV}  to ${IAI_ENV_FILE} so the skill finds it"
fi
