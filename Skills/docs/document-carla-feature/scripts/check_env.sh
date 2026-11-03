#!/usr/bin/env bash
# Prerequisites for document-carla-feature. PASS / WARN / FAIL per check;
# exits non-zero only when the skill cannot run at all.
set -u
cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1
# shellcheck source=env.sh
. ./env.sh

fail=0
say() { printf '%-5s %s\n' "$1" "$2"; }

if [ -n "${CARLA_ROOT:-}" ] && [ -d "${CARLA_ROOT}/PythonAPI/docs" ]; then
  say PASS "carla checkout: ${CARLA_ROOT}"
else
  say FAIL "no carla checkout resolved; export CARLA_ROOT=/path/to/carla"
  fail=1
fi

if command -v "${PYTHON}" >/dev/null 2>&1; then
  say PASS "python: $("${PYTHON}" --version 2>&1)"
else
  say FAIL "python not found; export PYTHON=/path/to/python3"
  fail=1
fi

if command -v mkdocs >/dev/null 2>&1; then
  say PASS "mkdocs: $(mkdocs --version 2>&1 | head -1)"
else
  say WARN "mkdocs missing: the docs cannot be built; pip install -r Docs/requirements.txt"
fi

if "${PYTHON}" -c 'import yaml' >/dev/null 2>&1; then
  say PASS "PyYAML available (doc_gen.py needs it)"
else
  say WARN "PyYAML missing: Docs/python_api.md cannot be regenerated"
fi

if "${PYTHON}" -c 'import PIL, numpy, matplotlib' >/dev/null 2>&1; then
  say PASS "Pillow, numpy and matplotlib available (image scripts)"
else
  say WARN "Pillow, numpy or matplotlib missing: the image scripts cannot run"
fi

if command -v google-chrome >/dev/null 2>&1; then
  say PASS "google-chrome available (diagram previews)"
else
  say WARN "google-chrome missing: diagrams render, but without a PNG preview"
fi

exit "${fail}"
