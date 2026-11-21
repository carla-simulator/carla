#!/usr/bin/env bash
# Prerequisite checks for calibrate-carla-ue58-camera-profile. Read-only, fast, never hangs.
# Hard blockers: no python3/numpy/Pillow/carla, no CARLA server answering, no writable profile directory.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"
set +e
rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }
PORT="${CARLA_PORT:-2000}"
ROOT="${CARLA_UE58_ROOT:-${CARLA_PACKAGE_ROOT:-${CARLA_ROOT:-}}}"

echo "== python =="
command -v python3 >/dev/null && ok "python3" || { bad "python3 missing"; exit 1; }
python3 -c "import numpy, PIL" 2>/dev/null && ok "numpy, Pillow" || bad "numpy / Pillow missing: pip install numpy pillow"
python3 -c "import carla" 2>/dev/null && ok "carla client importable" \
  || bad "no carla module in this python: see the install-python-api skill"

echo "== server where the profiles live (CARLA_UE58_ROOT or CARLA_PACKAGE_ROOT) =="
if [ -z "${ROOT}" ]; then
  bad "no root set: ask the user which ue58-dev tree or package runs the server, then record it"
elif [ -d "${ROOT}/Unreal/CarlaUnreal/Content" ]; then
  D="${ROOT}/Unreal/CarlaUnreal/Content/Carla/Config/PostProcess"
  [ -w "${D}" ] && ok "source tree, profiles in ${D}" || bad "${D} not writable"
elif [ -d "${ROOT}/CarlaUnreal/Content" ]; then
  D="${ROOT}/CarlaUnreal/Content/Carla/Config/PostProcess"
  [ -w "${ROOT}/CarlaUnreal/Content" ] && ok "package, new profiles go to ${D} (loose file next to the .pak)" \
    || bad "${ROOT}/CarlaUnreal/Content not writable"
else
  bad "${ROOT} is neither a ue58-dev tree nor a package"
fi

echo "== CARLA server on port ${PORT} =="
timeout 20 python3 - "${PORT}" <<'PY' 2>/dev/null
import sys, carla
c = carla.Client('localhost', int(sys.argv[1])); c.set_timeout(10)
w = c.get_world()
bp = w.get_blueprint_library().find('sensor.camera.rgb')
print('  PASS server %s, map %s' % (c.get_server_version(), w.get_map().name))
if not bp.has_attribute('post_process_profile'):
    print('  FAIL sensor.camera.rgb has no post_process_profile: needs CARLA 1.0 / ue58-dev'); sys.exit(1)
print('  PASS post_process_profile attribute')
s = w.get_settings()
if s.synchronous_mode:
    print('  WARN the world is already in synchronous mode: another client may be ticking it')
PY
case $? in
  0) ;;
  *) bad "no CARLA server answering on port ${PORT} (start one with the run-carla-ue58-server skill)";;
esac

echo
[ ${rc} -eq 0 ] && echo "READY" || echo "NOT READY"
exit ${rc}
