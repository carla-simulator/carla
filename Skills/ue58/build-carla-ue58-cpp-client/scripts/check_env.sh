#!/usr/bin/env bash
# Prerequisite checks for build-carla-ue58-cpp-client. Read-only, no sudo, fast.
# Hard blockers: no ue58 checkout, no engine (its clang and libc++ are the toolchain), no configured build tree.
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

echo "== checkout and engine =="
if [ -n "${CARLA_UE58_ROOT}" ] && carla_ue58_is_root "${CARLA_UE58_ROOT}"; then
  ok "checkout at ${CARLA_UE58_ROOT} (branch $(carla_ue58_branch))"
else
  bad "no ue58-dev checkout; export CARLA_UE58_ROOT=/path/to/carla"
fi
if [ -n "${CARLA_UNREAL_ENGINE_PATH}" ] && [ -d "${CARLA_UNREAL_ENGINE_PATH}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux" ]; then
  ok "engine toolchain under ${CARLA_UNREAL_ENGINE_PATH} (engine $(carla_engine_version))"
else
  bad "CARLA_UNREAL_ENGINE_PATH unset or has no HostLinux SDK; CMake/Toolchain.cmake needs it for clang and libc++"
fi

echo "== tools =="
if command -v cmake >/dev/null; then
  v="$(cmake --version | head -1 | awk '{print $3}')"
  [ "$(printf '%s\n3.28\n' "$v" | sort -V | head -1)" = "3.28" ] && ok "cmake ${v}" || bad "cmake ${v} is older than 3.28"
else
  bad "cmake not found"
fi
command -v ninja >/dev/null && ok "ninja $(ninja --version)" || bad "ninja not found"

echo "== build tree =="
B="$(carla_ue58_build_dir)"
if carla_ue58_configured; then
  ok "configured: ${B}"
  [ "$(carla_ue58_cmake_opt BUILD_CARLA_CLIENT)" = "ON" ] && ok "BUILD_CARLA_CLIENT=ON" || bad "BUILD_CARLA_CLIENT is not ON in ${B}; reconfigure with -DBUILD_CARLA_CLIENT=ON"
  [ "$(carla_ue58_cmake_opt BUILD_EXAMPLES)" = "ON" ] && ok "BUILD_EXAMPLES=ON" || warn "BUILD_EXAMPLES is not ON: the in-tree example targets do not exist (out-of-tree projects still work)"
  [ -d "${B}/_deps/boost-src" ] && ok "vendored dependencies in ${B}/_deps" || warn "${B}/_deps/boost-src missing: build carla-client once before an out-of-tree project"
else
  bad "${B} is not configured; run the build-carla-ue58 skill's configure step (or set CARLA_PRESET)"
fi

echo
[ $rc -eq 0 ] && echo "prerequisites OK — hard blockers clear" || echo "prerequisites BLOCKED — resolve FAIL lines above"
exit $rc
