#!/usr/bin/env bash
# Read-only prerequisite checks for audit-carla-docs.
# Exits non-zero only on hard blockers: no checkout, or no python3.
set -uo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")" || exit 1
# shellcheck source=env.sh
source ./env.sh

fails=0
pass() { echo "PASS  $*"; }
warn() { echo "WARN  $*"; }
fail() { echo "FAIL  $*"; fails=$((fails + 1)); }

if [ -z "${CARLA_ROOT:-}" ]; then
  fail "no carla checkout found — export CARLA_ROOT=/path/to/carla"
else
  pass "carla checkout at ${CARLA_ROOT}"
fi

command -v "${PYTHON}" >/dev/null 2>&1 \
  && pass "${PYTHON} available ($(${PYTHON} --version 2>&1))" \
  || fail "${PYTHON} not found"

# Inputs the audit reads. Missing ones degrade the report rather than block it,
# so they are WARNs: a partial audit still finds real gaps.
for f in CHANGELOG.md mkdocs.yml; do
  [ -f "${CARLA_ROOT:-/nonexistent}/$f" ] && pass "$f present" || warn "$f missing — its signal is skipped"
done
for d in Docs PythonAPI/docs PythonAPI/carla/src; do
  [ -d "${CARLA_ROOT:-/nonexistent}/$d" ] && pass "$d/ present" || warn "$d/ missing — its signal is skipped"
done

# The audit never needs these, but say so once: a reader who knows CARLA will
# wonder, and the Tier B skill does need them.
pass "no build or running server required (static audit)"

echo
if [ "${fails}" -gt 0 ]; then
  echo "prerequisites FAILED (${fails} hard blocker(s))"
  exit 1
fi
echo "prerequisites OK — hard blockers clear"
