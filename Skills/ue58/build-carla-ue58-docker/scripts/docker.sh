#!/usr/bin/env bash
# Builds and runs CARLA UE 5.8 Docker images: the release image from a package, and the development image from Util/Docker.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
DISTRO="${UBUNTU_DISTRO:-22.04}"

usage() {
  cat <<EOF
usage: docker.sh <command> [args]
  release-build PACKAGE [TAG]    build the release image from a package folder or Carla-*-Linux-Shipping.tar.gz (TAG default carla-local:<version>)
  release-run TAG [PORT] [-- ARGS]   start the server container detached as carla-<PORT>, offscreen, waits for the RPC port
  wheel TAG DIR                  copy the image's carla wheel into DIR (install it in the client's interpreter)
  probe PORT                     connect with the active python3 and report versions, map and a camera frame
  stop PORT                      stop the container carla-<PORT> (only that container)
  dev-build [build.sh args]      build carla-base and carla-development images (Util/Docker/build.sh)
  dev-run                        start the interactive development container (Util/Docker/run.sh --dev)
  dev-exec CMD...                run a command in the running development container
EOF
}

gpu_flags() {
  # CDI when the spec exists, else the legacy nvidia runtime; both need the NVIDIA Container Toolkit.
  if [ -f /etc/cdi/nvidia.yaml ] || [ -f /var/run/cdi/nvidia.yaml ]; then
    echo "--device=nvidia.com/gpu=${GPU:-all}"
  else
    echo "--runtime=nvidia --env=NVIDIA_VISIBLE_DEVICES=${GPU:-all} --env=NVIDIA_DRIVER_CAPABILITIES=all"
  fi
}

port_up() { (exec 3<>"/dev/tcp/127.0.0.1/$1") 2>/dev/null; }

cmd="${1:-}"; shift || true
case "${cmd}" in
  release-build)
    src="${1:?release-build needs a package folder or tarball}"
    if [ -f "${src}" ]; then
      dest="${WORK_DIR:-$(dirname "${src}")}"
      echo "[docker] extracting ${src} into ${dest}"
      tar -xzf "${src}" -C "${dest}"
      src="${dest}/$(tar -tzf "${src}" | head -1 | cut -d/ -f1)"
    fi
    [ -f "${src}/Dockerfile" ] && [ -f "${src}/CarlaUnreal.sh" ] || { echo "[docker] ${src} has no Dockerfile + CarlaUnreal.sh: not a Linux package root" >&2; exit 1; }
    ver="$(sed -n 's/^Carla git hash: *\(.\{9\}\).*/\1/p' "${src}/VERSION" 2>/dev/null || true)"
    tag="${2:-carla-local:${ver:-latest}}"
    docker build -t "${tag}" "${src}"
    size="$(docker image inspect -f '{{.Size}}' "${tag}")"
    [ "${size}" -gt $((4 * 1024 * 1024 * 1024)) ] || { echo "[docker] FAIL: ${tag} is only $((size / 1048576)) MB; the package copy is missing" >&2; exit 1; }
    echo "[docker] OK ${tag} ($((size / 1073741824)) GB)" ;;
  release-run)
    tag="${1:?release-run needs TAG}"; port="${2:-2000}"; shift $(( $# >= 2 ? 2 : 1 ))
    [ "${1:-}" = "--" ] && shift
    port_up "${port}" && { echo "[docker] port ${port} is already taken" >&2; exit 1; }
    # shellcheck disable=SC2046
    docker run -d --rm --name "carla-${port}" $(gpu_flags) --net=host "${tag}" \
      bash CarlaUnreal.sh -RenderOffScreen -nosound -carla-rpc-port="${port}" -carla-streaming-port=$((port + 1)) -carla-secondary-port=$((port + 2)) "$@" >/dev/null
    for _ in $(seq 1 100); do
      port_up "${port}" && { echo "[docker] carla-${port} up on ${port}"; exit 0; }
      docker inspect "carla-${port}" >/dev/null 2>&1 || { echo "[docker] container exited; run it without -d to see why" >&2; exit 1; }
      sleep 3
    done
    echo "[docker] port ${port} not up after 300 s; docker logs carla-${port}" >&2; exit 1 ;;
  wheel)
    tag="${1:?wheel needs TAG}"; dir="$(cd "${2:?wheel needs DIR}" && pwd)"
    docker run --rm --user="$(id -u):$(id -g)" --volume="${dir}:/out" "${tag}" bash -c "cp PythonAPI/carla/dist/carla-*.whl /out/"
    ls "${dir}"/carla-*.whl ;;
  probe)
    "${PYTHON}" "${HERE}/probe.py" "${1:-2000}" ;;
  stop)
    docker stop "carla-${1:?stop needs PORT}" >/dev/null && echo "[docker] stopped carla-$1" ;;
  dev-build)
    bash "${CARLA_UE58_ROOT}/Util/Docker/build.sh" --ubuntu-distro "${DISTRO}" "$@"
    docker image inspect "carla-development:ue5-${DISTRO}" >/dev/null && echo "[docker] OK carla-development:ue5-${DISTRO}" ;;
  dev-run)
    export CARLA_UNREAL_ENGINE_PATH
    bash "${CARLA_UE58_ROOT}/Util/Docker/run.sh" --dev --ubuntu-distro "${DISTRO}" ;;
  dev-exec)
    docker exec "carla-development-ue5-${DISTRO}" "$@" ;;
  *) usage; exit 1 ;;
esac
