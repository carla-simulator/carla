#!/usr/bin/env bash
# Step 04 — build the CARLA Python API client (LibCarla client + boost.python
# bindings + osm2odr) and install the wheel into the active client env.
#
# Prereqs: step 03 (UE4 built -> bundled clang SDK) and step 02 (client env
# prepared: a compatible interpreter active, with requirements.txt + numpy<2).
#
# Key correctness points encoded here:
#   * env.sh resolves the ACTIVE interpreter and exports its exact
#     minor as CARLA_PY_VERSION. We forward --python-version=${CARLA_PY_VERSION}
#     to `make PythonAPI`, which passes ARGS to the `setup` target too — so
#     boost.python (Setup.sh) and the wheel (BuildPythonAPI.sh) bind to the SAME
#     interpreter. Mismatch => ImportError at `import carla` (L7).
#   * The build invokes `/usr/bin/env python${CARLA_PY_VERSION}` by exact minor,
#     so that interpreter must be on PATH — it is, because it IS the active env's.
#   * On Ubuntu 24.04 the build sets _SKIP_PIP_INSTALL (PEP 668), leaving the
#     wheel in PythonAPI/carla/dist/. We install it explicitly into the env.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
[ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ] \
  || { echo "[py] ERROR: UE4 not built yet (run step 03)."; exit 1; }

carla_require_build_python || exit 1   # sets CARLA_PY_BIN + CARLA_PY_VERSION
echo "[py] building against: ${CARLA_PY_BIN} (${CARLA_PY_VERSION})"

# Idempotent ("if needed"), but the test must be "did THIS checkout produce a
# client", not "is any carla importable". A globally installed carla (a release
# wheel, or another checkout's build) otherwise makes a fresh clone skip its own
# PythonAPI forever — and the client you end up talking to the server with is then
# the wrong version, which fails as a mid-call abort rather than an import error
# (verified 2026-08: a 0.10.0 client against a 0.9.16 server died with
# std::bad_array_new_length).
#
# So: skip only when carla imports AND it resolves inside this checkout, or a
# wheel for this interpreter already sits in the checkout's dist/.
if [ "${FORCE:-0}" != "1" ]; then
  _origin="$("${CARLA_PY_BIN}" -c 'import carla,os;print(os.path.realpath(carla.__file__))' 2>/dev/null || true)"
  _dist_whl="$(ls "${CARLA_UE4_ROOT}"/PythonAPI/carla/dist/carla-*-cp"${CARLA_PY_VERSION//./}"-*.whl 2>/dev/null | head -1 || true)"
  case "${_origin}" in
    "${CARLA_UE4_ROOT}"/*)
      echo "[py] carla already importable FROM THIS CHECKOUT (${_origin}) — skipping. FORCE=1 to rebuild."
      exit 0;;
  esac
  if [ -n "${_dist_whl}" ]; then
    echo "[py] this checkout already built a wheel: ${_dist_whl}"
    echo "[py] install it into the target env, or FORCE=1 to rebuild:"
    echo "[py]   ${CARLA_PY_BIN} -m pip install '${_dist_whl}'"
    exit 0
  fi
  if [ -n "${_origin}" ]; then
    echo "[py] NOTE: carla imports from ${_origin}"
    echo "[py]       that is OUTSIDE this checkout, so it is a DIFFERENT build —"
    echo "[py]       building this checkout's own client now (activate a venv first"
    echo "[py]       if you do not want it installed over that one)."
  fi
fi

cd "${CARLA_UE4_ROOT}"

echo "[py] make PythonAPI (boost + LibCarla client + osm2odr + wheel)..."
make PythonAPI ARGS="--python-version=${CARLA_PY_VERSION} --build-wheel"

# Install the freshly built wheel into the active env (PEP 668 left it in dist/).
WHEEL="$(ls -t "${CARLA_UE4_ROOT}"/PythonAPI/carla/dist/*.whl 2>/dev/null | head -1 || true)"
[ -n "${WHEEL}" ] || { echo "[py] ERROR: no wheel produced in PythonAPI/carla/dist/"; exit 1; }
echo "[py] installing ${WHEEL} into ${CARLA_PY_BIN}..."
"${CARLA_PY_BIN}" -m pip install --force-reinstall "${WHEEL}"

echo "[py] verifying import..."
"${CARLA_PY_BIN}" -c "import carla; print('carla', getattr(carla, '__version__', 'imported OK'))"
echo "[py] DONE."
