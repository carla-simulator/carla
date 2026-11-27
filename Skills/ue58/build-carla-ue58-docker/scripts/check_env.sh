#!/usr/bin/env bash
# Prerequisite checks for build-carla-ue58-docker. Read-only, no sudo, fast.
# Hard blockers: no docker, a daemon this user cannot reach, no NVIDIA runtime or CDI spec.
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

echo "== docker =="
if ! command -v docker >/dev/null; then
  bad "docker not installed (https://docs.docker.com/engine/install/)"
elif ! INFO="$(timeout 10 docker info 2>/dev/null)"; then
  bad "cannot reach the docker daemon as $(id -un): start it, or add the user to the docker group"
else
  ok "docker $(docker version -f '{{.Server.Version}}' 2>/dev/null)"
  cdi=""; [ -f /etc/cdi/nvidia.yaml ] || [ -f /var/run/cdi/nvidia.yaml ] && cdi=1
  rt=""; grep -q "Runtimes:.*nvidia" <<<"${INFO}" && rt=1
  [ -n "${cdi}" ] && ok "CDI spec present: containers get --device=nvidia.com/gpu=all"
  [ -n "${rt}" ] && ok "nvidia runtime registered (legacy --runtime=nvidia; Util/Docker/run.sh needs it)"
  [ -z "${cdi}${rt}" ] && bad "no NVIDIA Container Toolkit: neither a CDI spec nor the nvidia runtime"
  [ -z "${rt}" ] && [ -n "${cdi}" ] && warn "no nvidia runtime: dev-run (Util/Docker/run.sh) passes --runtime=nvidia and will fail"
  root="$(sed -n 's/^ *Docker Root Dir: *//p' <<<"${INFO}")"
  free_gb="$(df -BG --output=avail "${root:-/var/lib/docker}" 2>/dev/null | tail -1 | tr -dc 0-9)"
  if [ -n "${free_gb}" ]; then
    [ "${free_gb}" -ge 40 ] && ok "${free_gb} GB free under ${root}" || warn "${free_gb} GB free under ${root}: a release image needs ~18 GB, plus ~10 GB if a tarball is extracted"
  fi
fi

echo "== sources =="
if [ -n "${CARLA_UE58_ROOT}" ] && [ -f "${CARLA_UE58_ROOT}/Util/Docker/build.sh" ]; then
  ok "Util/Docker in ${CARLA_UE58_ROOT} (dev images)"
else
  warn "no ue58-dev checkout with Util/Docker: only the release image from a package is possible"
fi
pkg="$(ls -1dt "${CARLA_UE58_ROOT:-/nonexistent}"/Build/*/Package/Carla-*-Linux-Shipping* 2>/dev/null | head -1)"
[ -n "${pkg}" ] && ok "package found: ${pkg}" || warn "no package under Build/*/Package: build one with package-carla-ue58, or pass a downloaded tarball"
[ -n "${CARLA_UNREAL_ENGINE_PATH}" ] && ok "CARLA_UNREAL_ENGINE_PATH=${CARLA_UNREAL_ENGINE_PATH} (mounted by dev-run)" || warn "CARLA_UNREAL_ENGINE_PATH unset: dev-run refuses to start without it"

echo
[ $rc -eq 0 ] && echo "prerequisites OK — hard blockers clear" || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit $rc
