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

if [ -f "${CARLA_ROOT:-}/Docs/audit_carla_docs.md" ]; then
  say PASS "audit report present (Docs/audit_carla_docs.md)"
else
  say WARN "no audit report; run the audit-carla-docs skill first, or name the targets yourself"
fi

if "${PYTHON}" -c 'import yaml' >/dev/null 2>&1; then
  say PASS "PyYAML available (doc_gen.py needs it)"
else
  say WARN "PyYAML missing: the python-api category cannot regenerate Docs/python_api.md"
fi

if [ -f "${CARLA_ROOT:-}/PythonAPI/docs/doc_gen.py" ]; then
  say PASS "doc_gen.py present"
else
  say WARN "doc_gen.py missing: the python-api category cannot regenerate Docs/python_api.md"
fi

exit "${fail}"
