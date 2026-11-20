#!/usr/bin/env bash
# Prerequisite checks for get-carla-additional-maps. Read-only, no sudo, fast.
# Hard blocker: no package to install into and no checkout to build from.
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

echo "== Package =="
if [ -n "${CARLA_PACKAGE_ROOT}" ] && carla_is_package "${CARLA_PACKAGE_ROOT}"; then
  ok "package ${CARLA_PACKAGE_ROOT} (stamp '$(cat "${CARLA_PACKAGE_ROOT}/CarlaUnreal/BaseRelease" 2>/dev/null)')"
  if [ -f "${CARLA_PACKAGE_ROOT}/CarlaUnreal/Packs/AdditionalMaps/carla-pack.json" ]; then
    ok "AdditionalMaps already installed"
  fi
  META="$(ls -1t "$(dirname "${CARLA_PACKAGE_ROOT}")"/*-release-metadata.tar.gz 2>/dev/null | head -1)"
  if [ -n "${META}" ]; then
    ok "locally built package: its metadata ${META}; build the pack from that tree (or use the published pair only with a published package)"
  else
    ok "no metadata next to the package: a downloaded package takes the AdditionalMaps published with it"
  fi
else
  warn "no package found — export CARLA_PACKAGE_ROOT, or download one with: additional_maps.py download --with-package"
fi

echo "== Source build (only to build the pack) =="
if [ -n "${CARLA_UE58_ROOT}" ] && [ -f "${CARLA_UE58_ROOT}/Util/ContentPacks/AdditionalMaps.json" ]; then
  ok "checkout ${CARLA_UE58_ROOT}"
  [ -d "${CARLA_UE58_ROOT}/Build/${CARLA_PRESET}" ] && ok "Build/${CARLA_PRESET} configured" || warn "Build/${CARLA_PRESET} missing: package-carla-ue58 first"
else
  warn "no ue58-dev checkout: you can only download the published pack"
fi
[ -z "${CARLA_PACKAGE_ROOT}" ] && [ -z "${CARLA_UE58_ROOT}" ] && bad "neither a package nor a checkout: nothing to install into or build from"

command -v curl >/dev/null && ok "curl present (resumable downloads)" || warn "no curl: downloads need it"
AVAIL_GB=$(df -BG --output=avail "${CARLA_PACKAGE_ROOT:-${CARLA_UE58_ROOT:-.}}" 2>/dev/null | tail -1 | tr -dc '0-9')
# The pack is 6.8 GB compressed and 7.5 GB unpacked; a local build stages ~3x that.
[ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -lt 15 ] && warn "only ${AVAIL_GB} GB free: download + install needs ~15 GB, a build ~25 GB" || ok "disk ${AVAIL_GB:-?} GB free"

echo
[ "${rc}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                  || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${rc}"
