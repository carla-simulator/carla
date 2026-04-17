#!/usr/bin/env bash
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
#
# Runs PythonAPI/test/unit/test_numpy_compat.py under both NumPy 1.x and
# NumPy 2.x in disposable venvs, proving the PythonAPI code is ABI-compatible
# with both major NumPy versions.
#
# Usage:
#   bash PythonAPI/test/unit/run_numpy_compat_matrix.sh [python-binary] [carla-wheel]
#
# Arguments:
#   python-binary   Python interpreter to use (default: python3).
#                   Must match the ABI of the carla wheel.
#   carla-wheel     Path to a carla-*.whl to install in each venv.
#                   If omitted, the script searches Dist/ and
#                   PythonAPI/carla/dist/ for a wheel whose cp-tag matches
#                   the base interpreter.  When no wheel is found,
#                   TestCarlaCExtensionLoad skips gracefully.
#
# Examples:
#   # Default: use system python3, auto-detect matching wheel from Dist/
#   bash PythonAPI/test/unit/run_numpy_compat_matrix.sh
#
#   # Pick a specific Python interpreter (auto-detects a matching cp-tag wheel):
#   bash PythonAPI/test/unit/run_numpy_compat_matrix.sh python3.12
#
#   # Explicit wheel path:
#   bash PythonAPI/test/unit/run_numpy_compat_matrix.sh \
#        python3.12 \
#        path/to/carla-0.9.16-cp312-*.whl
#
# Exit codes:
#   0  both NumPy 1.x and NumPy 2.x runs passed (or NumPy 2.x was skipped
#      because Python < 3.9, which is the NumPy 2 minimum)
#   1  at least one run failed

set -eu

PYTHON="${1:-python3}"
CARLA_WHEEL="${2:-}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../" && pwd)"
WORK_DIR="$(mktemp -d -t carla_numpy_matrix.XXXXXX)"

cleanup() { rm -rf "${WORK_DIR}"; }
trap cleanup EXIT

# Validate the base interpreter exists and has venv support.
if ! "${PYTHON}" -c 'import venv' 2>/dev/null ; then
    echo "error: ${PYTHON} does not have the 'venv' module available" >&2
    exit 1
fi

PY_MAJOR_MINOR="$("${PYTHON}" -c 'import sys; print("{}.{}".format(sys.version_info[0], sys.version_info[1]))')"
PY_MAJOR="$(echo "${PY_MAJOR_MINOR}" | cut -d. -f1)"
PY_MINOR="$(echo "${PY_MAJOR_MINOR}" | cut -d. -f2)"

# Build the cpython ABI tag for the base interpreter (e.g. cp312).
# Used to pick a compatible wheel from the build output directories.
PY_TAG="$("${PYTHON}" -c 'import sys; print("cp{}{}".format(sys.version_info[0], sys.version_info[1]))')"

# Auto-detect carla wheel if not provided.
# Search Dist/ (package output) then PythonAPI/carla/dist/ (dev build output),
# requiring the wheel filename to contain the base interpreter's cp-tag so pip
# does not reject it as incompatible.
if [ -z "${CARLA_WHEEL}" ] ; then
    CARLA_WHEEL="$(find "${REPO_ROOT}/Dist" "${REPO_ROOT}/PythonAPI/carla/dist" \
        -name "carla-*-${PY_TAG}-*.whl" 2>/dev/null | sort | tail -1 || true)"
fi

echo "Base interpreter: ${PYTHON} (Python ${PY_MAJOR_MINOR}, ${PY_TAG})"
echo "Work directory:   ${WORK_DIR}"
if [ -n "${CARLA_WHEEL}" ] && [ -f "${CARLA_WHEEL}" ] ; then
    echo "carla wheel:      ${CARLA_WHEEL}"
else
    echo "carla wheel:      not found -- TestCarlaCExtensionLoad will skip"
    CARLA_WHEEL=""
fi

print_header() {
    echo ""
    echo "================================================================"
    echo "== $1"
    echo "================================================================"
}

run_under_numpy() {
    local label="$1" spec="$2" venv_dir="$3"
    print_header "${label} (${spec})"

    "${PYTHON}" -m venv "${venv_dir}"
    "${venv_dir}/bin/pip" install --quiet --upgrade pip
    "${venv_dir}/bin/pip" install --quiet "${spec}" nose2

    if [ -n "${CARLA_WHEEL}" ] ; then
        "${venv_dir}/bin/pip" install --quiet "${CARLA_WHEEL}"
    fi

    "${venv_dir}/bin/python" -c "import numpy, sys; \
        print('Python', sys.version.split()[0], '/ NumPy', numpy.__version__)"

    ( cd "${SCRIPT_DIR}" && "${venv_dir}/bin/python" -m nose2 -v test_numpy_compat )
}

# --- NumPy 1.x run ---------------------------------------------------
run_under_numpy "NumPy 1.x run" "numpy>=1.18.4,<2.0.0" "${WORK_DIR}/npy1"

# --- NumPy 2.x run ---------------------------------------------------
# NumPy 2.0 requires Python >= 3.9.  Skip (not fail) if the base
# interpreter is too old.
if [ "${PY_MAJOR}" -lt 3 ] || { [ "${PY_MAJOR}" -eq 3 ] && [ "${PY_MINOR}" -lt 9 ]; } ; then
    print_header "NumPy 2.x run -- SKIPPED"
    echo "NumPy 2.x requires Python >= 3.9; base interpreter is ${PY_MAJOR_MINOR}."
    echo "Re-run with a newer interpreter to validate the NumPy 2 path."
else
    run_under_numpy "NumPy 2.x run" "numpy>=2.0.0" "${WORK_DIR}/npy2"
fi

print_header "Matrix validation OK"
