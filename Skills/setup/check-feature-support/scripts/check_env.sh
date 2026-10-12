#!/usr/bin/env bash
# Prerequisite checks for check-feature-support. Read-only, no sudo, fast.
#
# This skill has NO hard prerequisites on purpose: `matrix` and `broken` answer
# from the skill's own text, so they work on a machine with no CARLA at all. The
# checks below only report how much `probe` will be able to see.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
# env.sh sets no shell options, but stay explicit: a preflight reports every
# problem rather than stopping at the first.
set +e

ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }

echo "== What this skill needs =="
ok "nothing — 'matrix' and 'broken' are answerable offline"

echo "== What would make 'probe' more informative =="
ROOTS=0
for pair in "CARLA_UE58_ROOT:${CARLA_UE58_ROOT}" "CARLA_UE5_ROOT:${CARLA_UE5_ROOT}" \
            "CARLA_UE4_ROOT:${CARLA_UE4_ROOT}" "CARLA_TARGET:${CARLA_TARGET}"; do
  R="${pair#*:}"
  [ -n "${R}" ] || continue
  if [ -d "${R}" ]; then ok "${pair%%:*} = ${R}"; ROOTS=1
  else warn "${pair%%:*} points at a missing path: ${R}"; fi
done
[ "${ROOTS}" -eq 1 ] || warn "no CARLA checkout offered — build-flag and tree checks will be skipped"

if ROOT="$(carla_any_root)"; then
  if [ -f "${ROOT}/Build/${CARLA_PRESET}/CMakeCache.txt" ]; then
    ok "CMake cache for preset ${CARLA_PRESET} (build flags readable)"
  else
    warn "no cache for preset ${CARLA_PRESET} — set CARLA_PRESET, or the tree is unconfigured"
  fi
fi

if "${PYTHON}" -c 'import carla' 2>/dev/null; then
  ok "carla importable — probe can query a running server"
else
  warn "carla not importable — probe skips the live checks (see install-python-api)"
fi

if command -v ss >/dev/null 2>&1 && ss -ltn 2>/dev/null | grep -q ":${CARLA_PORT}\b"; then
  ok "something is listening on ${CARLA_PORT} (probe will query it)"
else
  warn "nothing listening on ${CARLA_HOST}:${CARLA_PORT} — start a server for the live checks"
fi

echo "== Result =="
echo "  preflight OK (this skill has no blockers by design)"
exit 0
