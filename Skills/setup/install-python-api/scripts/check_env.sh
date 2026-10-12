#!/usr/bin/env bash
# Prerequisite checks for install-python-api. Read-only, no sudo, no network
# writes. Exits non-zero only on hard blockers (no usable interpreter, and no
# source at all to install from).
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

rc=0
ok(){   echo "PASS  $*"; }
warn(){ echo "WARN  $*"; }
bad(){  echo "FAIL  $*"; rc=1; }

echo "== Target interpreter =="
if [ -z "${PYTHON_BIN}" ]; then
  bad "PYTHON='${PYTHON}' not found on PATH — set PYTHON to the interpreter that should carry the client"
else
  ok "${PYTHON_BIN} (${CARLA_PY_XY}, wheel tag ${CARLA_PY_TAG})"
  # An isolated MCP server (uvx/pipx/npx) puts its OWN python first on PATH, so
  # the default python3 is usually the wrong target in that setup. Say so rather
  # than installing the client somewhere it will never be used.
  case "${PYTHON_BIN}" in
    */uv/*|*/pipx/*|*/.cache/uv/*)
      warn "that looks like an isolated tool env — the client installed there serves nothing;"
      warn "set PYTHON=/path/to/your/venv/bin/python (the one that talks to CARLA)";;
  esac
  if "${PYTHON_BIN}" -c 'import carla' 2>/dev/null; then
    ok "carla already importable$("${PYTHON_BIN}" -c 'import importlib.metadata as m; print(" ("+m.version("carla")+")")' 2>/dev/null)"
  else
    warn "carla NOT importable yet — that is what this skill fixes"
  fi
  "${PYTHON_BIN}" -c 'import pip' 2>/dev/null && ok "pip available for that interpreter" \
    || bad "no pip for ${PYTHON_BIN} — install pip, or use a venv"
  # PEP 668 marks system interpreters as externally managed; pip then refuses to
  # install without a venv, which is a blocker worth naming before we try.
  MARK="$("${PYTHON_BIN}" -c 'import sysconfig,os;p=os.path.join(sysconfig.get_paths()["stdlib"],"EXTERNALLY-MANAGED");print(p if os.path.exists(p) else "")' 2>/dev/null)"
  [ -n "${MARK}" ] && warn "PEP 668: ${PYTHON_BIN} is externally managed — install into a venv (or pass --break-system-packages yourself)" \
                   || ok "not PEP 668 externally-managed"
fi

echo "== Sources to install from =="
FOUND=0
if [ -n "${CARLA_PACKAGE_ROOT}${CARLA_UE4_ROOT}" ]; then
  for root in "${CARLA_PACKAGE_ROOT}" "${CARLA_UE4_ROOT}"; do
    [ -n "${root}" ] || continue
    W=$(find -L "${root}" -path "*PythonAPI/carla/dist/carla-*${CARLA_PY_TAG}*.whl" -print 2>/dev/null | head -3)
    E=$(find -L "${root}" -path "*PythonAPI/carla/dist/carla-*py${CARLA_PY_XY}*.egg" -print 2>/dev/null | head -3)
    if [ -n "${W}" ]; then
      ok "bundled wheel(s) matching ${CARLA_PY_TAG} under ${root}:"; printf '        %s\n' ${W}; FOUND=1
    fi
    if [ -n "${E}" ]; then
      ok "bundled egg(s) for py${CARLA_PY_XY} under ${root}:"; printf '        %s\n' ${E}; FOUND=1
    fi
    [ -n "${W}${E}" ] || warn "no artifact matching this interpreter under ${root}"
  done
else
  warn "neither CARLA_PACKAGE_ROOT nor CARLA_UE4_ROOT is set — only the PyPI source is available"
fi
# PyPI is the fallback and needs no local files, but it only has wheels for some
# interpreters (0.9.16: cp310/311/312, linux+windows), so probe rather than guess.
if command -v curl >/dev/null 2>&1; then
  if curl -fsS --max-time 10 https://pypi.org/pypi/carla/json >/dev/null 2>&1; then
    ok "PyPI reachable (source 'pypi' usable; run 'detect' for the tag matrix)"; FOUND=1
  else
    warn "PyPI unreachable — offline; only a bundled artifact can be used"
  fi
else
  warn "curl missing — cannot probe PyPI availability here"
fi
[ "${FOUND}" -eq 1 ] || bad "no installable source found (no matching bundled artifact, no PyPI)"

echo "== Result =="
[ "$rc" -eq 0 ] && echo "prerequisites OK (warnings are non-blocking)" \
               || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit $rc
