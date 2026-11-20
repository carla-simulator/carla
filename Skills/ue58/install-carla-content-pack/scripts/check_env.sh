#!/usr/bin/env bash
# Prerequisite checks for install-carla-content-pack. Read-only, no sudo, fast.
# Hard blockers: no package, no BaseRelease stamp, no carla_pack.py, no Python 3.8+.
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
if [ -z "${CARLA_PACKAGE_ROOT}" ] || ! carla_is_package "${CARLA_PACKAGE_ROOT}"; then
  bad "no CARLA UE 5.8 package found — export CARLA_PACKAGE_ROOT=<folder holding CarlaUnreal.sh>"
else
  ok "package ${CARLA_PACKAGE_ROOT}"
  STAMP="$(cat "${CARLA_PACKAGE_ROOT}/CarlaUnreal/BaseRelease" 2>/dev/null || cat "${CARLA_PACKAGE_ROOT}/BaseRelease" 2>/dev/null)"
  if [ -n "${STAMP}" ]; then
    ok "BaseRelease stamp '${STAMP}' (a pack must be built against it)"
  else
    bad "no CarlaUnreal/BaseRelease: this package predates content packs; rebuild it or get a newer one"
  fi
  sed -n 's/^\(Carla\|UnrealEngine\) git hash: *\(.\{12\}\).*/  PASS \1 commit \2/p' "${CARLA_PACKAGE_ROOT}/VERSION" 2>/dev/null
  N="$(ls -1d "${CARLA_PACKAGE_ROOT}"/CarlaUnreal/Packs/*/carla-pack.json 2>/dev/null | wc -l)"
  ok "${N} pack(s) installed in CarlaUnreal/Packs/"
  [ -d "${CARLA_PACKAGE_ROOT}/CarlaUnreal/Plugins/Packs" ] && warn "CarlaUnreal/Plugins/Packs exists: packs belong in CarlaUnreal/Packs/, not Plugins/"
fi

echo "== Tools =="
if [ -n "${CARLA_PACK_TOOL}" ]; then ok "carla_pack.py ${CARLA_PACK_TOOL}"; else bad "no carla_pack.py in <package>/Tools/ or a checkout's Util/ContentPacks/"; fi
"${PYTHON}" -c 'import sys; sys.exit(sys.version_info < (3, 8))' 2>/dev/null \
  && ok "$("${PYTHON}" -V 2>&1)" || bad "${PYTHON} missing or older than 3.8"

if [ -n "${CARLA_PACKAGE_ROOT}" ]; then
  AVAIL_GB=$(df -BG --output=avail "${CARLA_PACKAGE_ROOT}" 2>/dev/null | tail -1 | tr -dc '0-9')
  # install extracts next to the package before moving: the pack's full unpacked size.
  [ -n "${AVAIL_GB}" ] && [ "${AVAIL_GB}" -lt 10 ] \
    && warn "only ${AVAIL_GB} GB free next to the package (AdditionalMaps unpacks to ~7.5 GB)" \
    || ok "disk ${AVAIL_GB:-?} GB free"
  if pgrep -f "[C]arlaUnreal-Linux" >/dev/null 2>&1; then
    warn "a CARLA server is running: a new pack appears at its next start (or via mount_content_pack); replacing or removing one needs it stopped"
  fi
fi

echo
[ "${rc}" -eq 0 ] && echo "prerequisites OK — hard blockers clear" \
                  || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit "${rc}"
