#!/usr/bin/env bash
# Prerequisite checks for check-ue5-limitations. Read-only, no sudo, fast.
#
# `gaps.sh list` needs nothing at all; `check` needs a ue5-dev checkout, and
# `diff` additionally needs a ue58-dev one.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== ue5-dev checkout (UE 5.5) =="
if [ -z "${CARLA_UE5_ROOT}" ] || ! carla_ue5_is_root "${CARLA_UE5_ROOT}"; then
  warn "no ue5-dev checkout — export CARLA_UE5_ROOT; 'gaps.sh list' works anyway"
else
  ok "checkout at ${CARLA_UE5_ROOT} (branch $(carla_ue5_branch))"
  case "$(carla_ue5_flavor)" in
    ue5)  ok "flavor ue5-dev (UE 5.5): the gaps in this skill apply" ;;
    ue58) bad "this tree has 5.8 markers — it is ue58-dev; use the ue58 skills directly" ;;
    *)    warn "cannot classify this tree" ;;
  esac
  E="$(carla_ue5_expected_engine)"
  case "${E}" in
    ue5-dev-carla)  ok "tree expects engine branch ${E}" ;;
    ue58-dev-carla) bad "tree expects ${E} — that is the 5.8 engine, so this is not a 5.5 tree" ;;
    *)              warn "tree does not state an engine branch" ;;
  esac
  [ -f "${CARLA_UE5_ROOT}/Makefile" ] \
    && warn "a Makefile exists — UE5 CARLA builds with CMake only; is this really a UE5 tree?" \
    || ok "no Makefile (expected: CMake-only build)"
fi

echo "== ue58-dev checkout (optional, enables 'diff') =="
if [ -n "${CARLA_UE58_ROOT}" ] && [ -d "${CARLA_UE58_ROOT}" ]; then
  ok "ue58 tree at ${CARLA_UE58_ROOT} — 'gaps.sh diff' will measure instead of recite"
else
  warn "CARLA_UE58_ROOT unset — 'diff' unavailable, 'list' and 'check' still work"
fi

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" \
                || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
