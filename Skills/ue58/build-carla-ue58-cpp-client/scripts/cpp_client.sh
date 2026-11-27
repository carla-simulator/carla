#!/usr/bin/env bash
# Builds and runs C++ clients against CARLA UE 5.8: the in-tree examples, or your own project linked to an installed libcarla-client.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
ASSETS="${HERE}/../assets/project"
BUILD="$(carla_ue58_build_dir)"

usage() {
  cat <<EOF
usage: cpp_client.sh <command> [args]
  examples                       build carla-example-client and carla-agent-demo in ${BUILD}
  example [HOST PORT]            run carla-example-client (loads a random map, spawns and destroys one vehicle)
  agent-demo [HOST PORT MAP BEHAVIOR]   run carla-agent-demo (BehaviorAgent drives to a far spawn point)
  install PREFIX                 install libcarla-client, its headers and CarlaConfig.cmake into PREFIX
  new DIR                        scaffold an out-of-tree project (CMakeLists.txt, CarlaDeps.cmake, main.cpp) in DIR
  build DIR PREFIX               configure and build the project in DIR against the install in PREFIX
  run DIR [HOST PORT]            run the project's my-carla-client
EOF
}

need_build() {
  [ -f "${BUILD}/CMakeCache.txt" ] || { echo "[cpp] ${BUILD} is not configured; run build-carla-ue58 configure first" >&2; exit 1; }
}

check_bin() {
  local f="$1" floor=$((1024 * 1024))
  [ -x "$f" ] && [ "$(stat -c %s "$f")" -gt "$floor" ] || { echo "[cpp] FAIL: $f missing or under 1 MB" >&2; exit 1; }
  echo "[cpp] OK $f ($(du -h "$f" | cut -f1))"
}

cmd="${1:-}"; shift || true
case "${cmd}" in
  examples)
    need_build
    cmake --build "${BUILD}" --target carla-example-client carla-agent-demo
    check_bin "${BUILD}/Examples/carla-example-client"
    check_bin "${BUILD}/Examples/carla-agent-demo" ;;
  example)
    "${BUILD}/Examples/carla-example-client" "${1:-${CARLA_HOST}}" "${2:-${CARLA_PORT}}" ;;
  agent-demo)
    "${BUILD}/Examples/carla-agent-demo" "${1:-${CARLA_HOST}}" "${2:-${CARLA_PORT}}" "${3:-Town10HD_Opt}" "${4:-normal}" ;;
  install)
    need_build
    prefix="${1:?install needs PREFIX}"
    cmake --build "${BUILD}" --target carla-client
    cmake --install "${BUILD}/LibCarla" --prefix "${prefix}" >/dev/null
    for f in lib/libcarla-client.a lib/cmake/Carla/CarlaConfig.cmake include/carla/client/Client.h; do
      [ -f "${prefix}/${f}" ] || { echo "[cpp] FAIL: ${prefix}/${f} not installed" >&2; exit 1; }
    done
    echo "[cpp] OK installed into ${prefix} ($(du -sh "${prefix}" | cut -f1))" ;;
  new)
    dir="${1:?new needs DIR}"
    [ -e "${dir}/CMakeLists.txt" ] && { echo "[cpp] ${dir}/CMakeLists.txt exists; not overwriting" >&2; exit 1; }
    mkdir -p "${dir}" && cp "${ASSETS}"/{CMakeLists.txt,CarlaDeps.cmake,main.cpp} "${dir}/"
    echo "[cpp] scaffolded ${dir}" ;;
  build)
    dir="${1:?build needs DIR}"; prefix="${2:?build needs PREFIX}"
    cmake -G Ninja -S "${dir}" -B "${dir}/build" --toolchain="${CARLA_UE58_ROOT}/CMake/Toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="${prefix}" -DCARLA_BUILD_DIR="${BUILD}"
    cmake --build "${dir}/build"
    check_bin "${dir}/build/my-carla-client" ;;
  run)
    dir="${1:?run needs DIR}"
    "${dir}/build/my-carla-client" "${2:-${CARLA_HOST}}" "${3:-${CARLA_PORT}}" ;;
  *) usage; exit 1 ;;
esac
