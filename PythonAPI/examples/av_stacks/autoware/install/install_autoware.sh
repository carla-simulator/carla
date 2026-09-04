#!/usr/bin/env bash
#
# install_autoware.sh - fetch/install Autoware for use with CARLA (UE5.8, native ROS2)
#
# Modes:
#   --source     (default) source build: clone autowarefoundation/autoware at a
#                release tag, vcs import, rosdep install, colcon build with
#                BOUNDED parallelism (this machine OOMs on unbounded builds).
#   --docker     pull the official prebuilt Autoware image from GHCR and print
#                the run pattern (no image is built locally).
#   --check      read-only prerequisite report. Never modifies anything.
#
# Extras:
#   --with-vad   also download the TensorRT-VAD model from HuggingFace into
#                ~/autoware_data/ml_models/vad/v0.1/ and (source mode) check out
#                the autoware_launch PR #1685 branch that wires VAD into
#                e2e_simulator.launch.xml.
#
# Version policy (both tags verified to exist on 2026-08-19):
#   0.45.1  TIER IV-validated baseline for CARLA native ROS2 (Humble / 22.04
#           source build). Default when ROS_DISTRO=humble.
#   1.9.0   current Autoware release (Jazzy is the upstream Docker default).
#           Default when ROS_DISTRO=jazzy and for --docker.
#
# Why manual rosdep instead of the upstream ansible dev-env playbook:
#   docs.autoware.org's canonical path runs `ansible-playbook
#   autoware.dev_env.install_dev_env`, which is INTERACTIVE (prompts for
#   NVIDIA/CUDA/TensorRT install and artifact downloads) and mutates the base
#   system well beyond ROS dependencies. `rosdep install -y` is deterministic
#   and non-interactive, which is what we want in a script. CUDA/cuDNN/TensorRT
#   (only needed for GPU perception / VAD) must already be present system-wide;
#   --check reports on that. If you do want the upstream ansible flow, run it
#   manually from the cloned workspace:
#       bash ansible/scripts/install-ansible.sh
#       ansible-galaxy collection install -f -r ansible-galaxy-requirements.yaml
#       ansible-playbook autoware.dev_env.install_dev_env   # add --skip-tags nvidia for CPU-only
#
set -euo pipefail

# ---------------------------------------------------------------------------
# Verified constants (all URLs HEAD/GET-checked 2026-08-19)
# ---------------------------------------------------------------------------
AUTOWARE_REPO_URL="https://github.com/autowarefoundation/autoware.git"
TIERIV_BASELINE_TAG="0.45.1"          # TIER IV-validated against CARLA native ROS2 (Humble)
CURRENT_RELEASE_TAG="1.9.0"           # latest 1.x release

# acados (OCP solver): required by autoware_path_optimizer (universe >= 0.52),
# which vendors an MPC whose solver code is generated at build time. Not in
# apt/rosdep -- upstream installs it via their ansible playbook, we build it
# here. v0.5.5 is the newest release whose acados_template Python API still
# matches the generator in autoware_path_optimizer (master dropped the
# json_file kwarg from AcadosOcpSolver.generate() and the codegen breaks).
ACADOS_REPO_URL="https://github.com/acados/acados.git"
ACADOS_VERSION="v0.5.5"
ACADOS_SRC_DIR="${ACADOS_SRC_DIR:-$HOME/acados-src}"
ACADOS_PREFIX="${ACADOS_PREFIX:-$HOME/acados}"
# Tera renderer binary for acados codegen (>= TERA_DEFAULT_VERSION of v0.5.5).
TERA_RENDERER_URL="https://github.com/acados/tera_renderer/releases/download/v0.2.1/t_renderer-v0.2.1-linux-amd64"

# PR autowarefoundation/autoware_launch#1685 "feat: e2e vad carla simulator".
# Verified via the GitHub API: state=open, head repo = autowarefoundation/autoware_launch
# (same repo, NOT a fork), head branch = feat/e2e-vad-carla-simulator, base = main.
# If the PR merges later, this branch may be deleted - the code then lives on
# autoware_launch main and exposes use_e2e_planning / e2e_planning_type:=vad;
# the checkout below fails gracefully with that hint.
VAD_LAUNCH_REPO_URL="https://github.com/autowarefoundation/autoware_launch.git"
VAD_LAUNCH_BRANCH="feat/e2e-vad-carla-simulator"

# HuggingFace VAD model (AutowareFoundation/tensorrt_vad, tag v0.1).
# File list verified against the HF tree API; every resolve URL returned 200.
HF_VAD_BASE="https://huggingface.co/AutowareFoundation/tensorrt_vad/resolve/v0.1"
VAD_FILES=(
    "vad-carla-tiny_backbone.onnx"      # ~94 MB
    "vad-carla-tiny_head_no_prev.onnx"  # ~143 MB
    "vad-carla-tiny_head.onnx"          # ~143 MB
    "vad-carla-tiny.param.json"
    "deploy_metadata.yaml"
)
VAD_DATA_DIR="${HOME}/autoware_data/ml_models/vad/v0.1"

# lidar_centerpoint perception models. The HuggingFace bundle lands under
# ~/autoware_data/ml_models/lidar_centerpoint/ in SUBDIRS (e.g. tiny/), but the
# launch files expect these FLAT files directly in the lidar_centerpoint dir:
CENTERPOINT_DATA_DIR="${HOME}/autoware_data/ml_models/lidar_centerpoint"
CENTERPOINT_FLAT_FILES=(
    "centerpoint_tiny_ml_package.param.yaml"
    "detection_class_remapper.param.yaml"
    "pts_voxel_encoder.onnx"
    "pts_backbone_neck_head.onnx"
)

# GHCR images (tags verified against the GHCR manifest API, all 200):
#   ghcr.io/autowarefoundation/autoware:universe-cuda-jazzy      (GPU, upstream default)
#   ghcr.io/autowarefoundation/autoware:universe-jazzy           (CPU-only)
#   ghcr.io/autowarefoundation/autoware:universe-cuda-humble / universe-humble
#   pinned releases append -<version>, e.g. universe-cuda-jazzy-1.9.0
DOCKER_IMAGE_BASE="ghcr.io/autowarefoundation/autoware"

# ---------------------------------------------------------------------------
# Defaults / CLI
# ---------------------------------------------------------------------------
MODE="source"
WITH_VAD=0
VERSION=""                                   # resolved after distro detection
WORKSPACE="${AUTOWARE_WS:-${HOME}/autoware}"
CUDA="auto"                                  # auto|yes|no (docker mode)
DOCKER_DISTRO="jazzy"                        # upstream Docker default

NPROC="$(nproc)"
DEFAULT_JOBS=$(( NPROC / 2 )); [ "${DEFAULT_JOBS}" -lt 1 ] && DEFAULT_JOBS=1
JOBS="${DEFAULT_JOBS}"

usage() {
    cat <<EOF
Usage: $(basename "$0") [MODE] [OPTIONS]

Modes (pick one; default --source):
  --source              Source build into a colcon workspace.
  --docker              Pull the official prebuilt image and print the run pattern.
  --check               Read-only prerequisite report.

Options:
  --version TAG         Autoware release tag (source: default ${TIERIV_BASELINE_TAG} on Humble,
                        ${CURRENT_RELEASE_TAG} on Jazzy; docker: default latest rolling tag,
                        pass e.g. ${CURRENT_RELEASE_TAG} to pin).
  --with-vad            Also fetch the TensorRT-VAD model (HuggingFace
                        AutowareFoundation/tensorrt_vad v0.1) and, in source
                        mode, check out autoware_launch PR #1685.
  --workspace DIR       Source-build workspace (default: \$AUTOWARE_WS or ~/autoware).
  --jobs N              Build parallelism (default: nproc/2 = ${DEFAULT_JOBS} here).
                        Sets MAKEFLAGS=-jN and colcon --parallel-workers N.
  --cuda | --no-cuda    Docker mode: force GPU/CPU image (default: auto via nvidia-smi).
  --distro NAME         Docker mode: humble or jazzy (default: jazzy, upstream default).
  -h, --help            This help.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --source) MODE="source" ;;
        --docker) MODE="docker" ;;
        --check)  MODE="check" ;;
        --with-vad) WITH_VAD=1 ;;
        --version)   VERSION="${2:?--version needs a tag}"; shift ;;
        --workspace) WORKSPACE="${2:?--workspace needs a dir}"; shift ;;
        --jobs)      JOBS="${2:?--jobs needs a number}"; shift ;;
        --cuda)    CUDA="yes" ;;
        --no-cuda) CUDA="no" ;;
        --distro)  DOCKER_DISTRO="${2:?--distro needs humble|jazzy}"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "ERROR: unknown argument '$1'" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

case "${JOBS}" in
    ''|*[!0-9]*) echo "ERROR: --jobs must be a positive integer" >&2; exit 2 ;;
esac
[ "${JOBS}" -lt 1 ] && JOBS=1

log()  { printf '\n==> %s\n' "$*"; }
warn() { printf 'WARNING: %s\n' "$*" >&2; }
die()  { printf 'ERROR: %s\n' "$*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# Environment detection
# ---------------------------------------------------------------------------
detect_ubuntu() {
    # Query os-release in a subshell: sourcing it in this shell would clobber
    # our VERSION variable (os-release defines its own VERSION field), which
    # silently breaks the release-tag default and clones a nonsense tag.
    UBUNTU_VERSION="$(
        # shellcheck disable=SC1091
        [ -r /etc/os-release ] && . /etc/os-release && \
            [ "${ID:-}" = "ubuntu" ] && printf '%s' "${VERSION_ID:-}"
    )" || UBUNTU_VERSION=""
}

detect_ros_distro() {
    ROS_DISTRO_DETECTED="${ROS_DISTRO:-}"
    if [ -z "${ROS_DISTRO_DETECTED}" ]; then
        for d in humble jazzy; do
            if [ -f "/opt/ros/${d}/setup.bash" ]; then
                ROS_DISTRO_DETECTED="${d}"
                break
            fi
        done
    fi
}

require_distro_os_match() {
    # Supported pairings: Humble <-> Ubuntu 22.04, Jazzy <-> Ubuntu 24.04.
    [ -n "${ROS_DISTRO_DETECTED}" ] || die "no ROS 2 found: ROS_DISTRO is unset and neither /opt/ros/humble nor /opt/ros/jazzy exists.
Install ROS 2 Humble (Ubuntu 22.04) or Jazzy (Ubuntu 24.04) first, or use --docker."
    [ -n "${UBUNTU_VERSION}" ] || die "this does not look like Ubuntu (no /etc/os-release ID=ubuntu). Source builds support Ubuntu 22.04 (Humble) or 24.04 (Jazzy); use --docker elsewhere."
    local want=""
    case "${ROS_DISTRO_DETECTED}" in
        humble) want="22.04" ;;
        jazzy)  want="24.04" ;;
        *) die "unsupported ROS distro '${ROS_DISTRO_DETECTED}'. Autoware supports humble (22.04) and jazzy (24.04)." ;;
    esac
    if [ "${UBUNTU_VERSION}" != "${want}" ]; then
        die "ROS/OS mismatch: ROS 2 ${ROS_DISTRO_DETECTED} requires Ubuntu ${want}, but this is Ubuntu ${UBUNTU_VERSION}.
Fix: use the matching Ubuntu, pick the other ROS distro, or use --docker (runs anywhere with Docker)."
    fi
}

resolve_source_version() {
    if [ -z "${VERSION}" ]; then
        case "${ROS_DISTRO_DETECTED}" in
            humble) VERSION="${TIERIV_BASELINE_TAG}" ;;  # TIER IV-validated CARLA baseline
            *)      VERSION="${CURRENT_RELEASE_TAG}" ;;
        esac
    fi
}

have() { command -v "$1" >/dev/null 2>&1; }

# ---------------------------------------------------------------------------
# --check : read-only prerequisite report
# ---------------------------------------------------------------------------
do_check() {
    detect_ubuntu
    detect_ros_distro

    echo "=== Autoware prerequisite check (read-only) ==="

    echo
    echo "--- OS / ROS ---"
    echo "Ubuntu version : ${UBUNTU_VERSION:-<not Ubuntu / unknown>}"
    echo "ROS_DISTRO env : ${ROS_DISTRO:-<unset>}"
    local found_ros=0
    for d in /opt/ros/*/; do
        [ -f "${d}setup.bash" ] && { echo "ROS install    : ${d}"; found_ros=1; }
    done
    [ "${found_ros}" -eq 0 ] && echo "ROS install    : none found under /opt/ros"
    if [ -n "${ROS_DISTRO_DETECTED}" ] && [ -n "${UBUNTU_VERSION}" ]; then
        case "${ROS_DISTRO_DETECTED}:${UBUNTU_VERSION}" in
            humble:22.04|jazzy:24.04) echo "Pairing        : OK (${ROS_DISTRO_DETECTED} on ${UBUNTU_VERSION})" ;;
            *) echo "Pairing        : MISMATCH (${ROS_DISTRO_DETECTED} on ${UBUNTU_VERSION}; humble needs 22.04, jazzy needs 24.04) -> --source will refuse; --docker will still work" ;;
        esac
    fi
    echo "RMW note       : CycloneDDS is Autoware's recommended RMW (rmw_cyclonedds_cpp)."

    echo
    echo "--- Hardware ---"
    echo "CPUs           : ${NPROC} (default build jobs: ${DEFAULT_JOBS} = nproc/2, bounded to avoid OOM)"
    local ram_gb
    ram_gb="$(awk '/MemTotal/ {printf "%d", $2/1024/1024}' /proc/meminfo 2>/dev/null || echo '?')"
    echo "RAM            : ${ram_gb} GB total (Autoware source builds want >=32 GB or bounded -j; keep jobs low if <2 GB/job)"
    if have nvidia-smi; then
        echo "GPU            : $(nvidia-smi --query-gpu=name,memory.total,driver_version --format=csv,noheader 2>/dev/null | head -1 || echo 'nvidia-smi present but query failed')"
    else
        echo "GPU            : nvidia-smi not found (CPU-only; GPU perception and VAD/TensorRT unavailable)"
    fi

    echo
    echo "--- Disk ---"
    local free_gb
    free_gb="$(df -BG --output=avail "$(dirname "${WORKSPACE}")" 2>/dev/null | tail -1 | tr -dc '0-9' || echo 0)"
    echo "Free at $(dirname "${WORKSPACE}") : ${free_gb} GB"
    if [ "${free_gb:-0}" -ge 100 ]; then
        echo "Disk           : OK (>=100 GB needed for a source build: sources + build + install + rosdep debs)"
    else
        echo "Disk           : INSUFFICIENT for a source build (need >=100 GB free); --docker needs ~20 GB"
    fi

    echo
    echo "--- Tooling ---"
    for t in git vcs rosdep colcon curl docker; do
        if have "$t"; then echo "$t : $(command -v "$t")"; else echo "$t : MISSING"; fi
    done
    have vcs    || echo "  -> vcs:    apt install python3-vcstool (or pip install vcstool in a venv)"
    have rosdep || echo "  -> rosdep: apt install python3-rosdep"
    have colcon || echo "  -> colcon: apt install python3-colcon-common-extensions"

    echo
    echo "--- Kernel / container runtime (README prerequisites) ---"
    local rmem wmem
    rmem="$(sysctl -n net.core.rmem_max 2>/dev/null || echo 0)"
    wmem="$(sysctl -n net.core.wmem_max 2>/dev/null || echo 0)"
    if [ "${rmem}" -ge 67108864 ] && [ "${wmem}" -ge 67108864 ]; then
        echo "UDP buffers    : OK (rmem_max=${rmem} wmem_max=${wmem})"
    else
        echo "UDP buffers    : TOO SMALL (rmem_max=${rmem} wmem_max=${wmem}; need 67108864). Fix: sudo sysctl -w net.core.rmem_max=67108864 net.core.wmem_max=67108864 and persist in /etc/sysctl.d/"
    fi
    if have docker && docker info 2>/dev/null | grep -qiE 'nvidia|cdi'; then
        echo "NVIDIA runtime : OK (docker reports an nvidia runtime or CDI spec)"
    elif have nvidia-ctk || [ -f /etc/cdi/nvidia.yaml ]; then
        echo "NVIDIA runtime : nvidia-ctk/CDI present but docker does not report it -- run 'sudo nvidia-ctk runtime configure --runtime=docker' and restart docker"
    else
        echo "NVIDIA runtime : MISSING (nvidia-container-toolkit not found; GPU perception in --docker mode will fail)"
    fi

    echo
    echo "--- Existing installs ---"
    if [ -d "${WORKSPACE}" ]; then
        echo "Workspace      : ${WORKSPACE} EXISTS"
        [ -d "${WORKSPACE}/src" ] && echo "  src pkgs     : $(find "${WORKSPACE}/src" -maxdepth 4 -name package.xml 2>/dev/null | wc -l) package.xml files"
        [ -f "${WORKSPACE}/install/setup.bash" ] && echo "  built        : yes (install/setup.bash present)"
        if [ -d "${WORKSPACE}/.git" ]; then
            echo "  autoware ref : $(git -C "${WORKSPACE}" describe --tags --always 2>/dev/null || echo unknown)"
        fi
    else
        echo "Workspace      : ${WORKSPACE} not present (fresh install)"
    fi
    if [ -d "${VAD_DATA_DIR}" ]; then
        echo "VAD model dir  : ${VAD_DATA_DIR} ($(find "${VAD_DATA_DIR}" -maxdepth 1 -type f 2>/dev/null | wc -l)/${#VAD_FILES[@]} files present)"
    else
        echo "VAD model dir  : ${VAD_DATA_DIR} not present"
    fi
    if [ -d "${CENTERPOINT_DATA_DIR}" ]; then
        local cp_flat=0
        for f in "${CENTERPOINT_FLAT_FILES[@]}"; do
            [ -e "${CENTERPOINT_DATA_DIR}/${f}" ] && cp_flat=$((cp_flat + 1))
        done
        echo "centerpoint dir: ${CENTERPOINT_DATA_DIR} (${cp_flat}/${#CENTERPOINT_FLAT_FILES[@]} FLAT files; launch needs all 4 flat -- source/docker modes fix this up)"
    else
        echo "centerpoint dir: ${CENTERPOINT_DATA_DIR} not present (needed for classical perception_mode:=lidar)"
    fi
    if have docker; then
        local imgs
        imgs="$(docker image ls --format '{{.Repository}}:{{.Tag}}' 2>/dev/null | grep -c "^${DOCKER_IMAGE_BASE}:" || true)"
        echo "Docker images  : ${imgs:-0} ${DOCKER_IMAGE_BASE} image(s) present locally"
    fi

    echo
    echo "=== Check complete (nothing was modified) ==="
}

# ---------------------------------------------------------------------------
# --with-vad helpers
# ---------------------------------------------------------------------------
fetch_vad_model() {
    log "Fetching TensorRT-VAD model v0.1 into ${VAD_DATA_DIR}"
    have curl || die "curl is required for --with-vad"
    mkdir -p "${VAD_DATA_DIR}"
    local f url
    for f in "${VAD_FILES[@]}"; do
        url="${HF_VAD_BASE}/${f}"
        if [ -s "${VAD_DATA_DIR}/${f}" ]; then
            echo "  ${f}: already present, skipping"
            continue
        fi
        echo "  ${f}: downloading"
        curl -fL --retry 3 -C - -o "${VAD_DATA_DIR}/${f}" "${url}" \
            || die "download failed: ${url}"
        [ -s "${VAD_DATA_DIR}/${f}" ] || die "downloaded file is empty: ${VAD_DATA_DIR}/${f}"
    done
    # vad_carla_tiny.launch.xml expects the ONNX files FLAT in ml_models/vad/
    # (model_path = data_path/vad); the versioned download dir keeps upgrades
    # clean, flat symlinks bridge the two. Idempotent.
    local parent
    parent="$(dirname "${VAD_DATA_DIR}")"
    for f in "${VAD_FILES[@]}"; do
        [ -e "${VAD_DATA_DIR}/${f}" ] && ln -sf "$(basename "${VAD_DATA_DIR}")/${f}" "${parent}/${f}"
    done
    echo "VAD model files in place (flat symlinks in ${parent}). TensorRT engines are"
    echo "built automatically on the first VAD run (slow; cached next to the ONNX files)."
}

# ---------------------------------------------------------------------------
# acados: build the pinned release, set up the codegen environment
# (acados_template venv + t_renderer) that autoware_path_optimizer's
# build-time MPC solver generation needs. Idempotent.
# ---------------------------------------------------------------------------
install_acados() {
    if [ -x "${ACADOS_SRC_DIR}/bin/t_renderer" ] && [ -e "${ACADOS_PREFIX}/lib/libacados.so" ] \
        && [ "$(git -C "${ACADOS_SRC_DIR}" describe --tags 2>/dev/null)" = "${ACADOS_VERSION}" ]; then
        log "acados ${ACADOS_VERSION} already installed at ${ACADOS_PREFIX}"
        return 0
    fi
    log "Building acados ${ACADOS_VERSION} into ${ACADOS_PREFIX}"
    if [ ! -d "${ACADOS_SRC_DIR}/.git" ]; then
        git clone --depth 1 --branch "${ACADOS_VERSION}" "${ACADOS_REPO_URL}" "${ACADOS_SRC_DIR}"
    else
        git -C "${ACADOS_SRC_DIR}" fetch --depth 1 origin tag "${ACADOS_VERSION}"
        git -C "${ACADOS_SRC_DIR}" checkout "${ACADOS_VERSION}"
    fi
    git -C "${ACADOS_SRC_DIR}" submodule update --init --recursive --depth 1
    cmake -S "${ACADOS_SRC_DIR}" -B "${ACADOS_SRC_DIR}/build" \
        -DCMAKE_BUILD_TYPE=Release -DACADOS_WITH_QPOASES=ON \
        -DCMAKE_INSTALL_PREFIX="${ACADOS_PREFIX}"
    make -C "${ACADOS_SRC_DIR}/build" -j"${JOBS}" install
    # Codegen expects the SOURCE tree layout: .venv with acados_template,
    # bin/t_renderer, and lib/include reachable under ACADOS_SOURCE_DIR.
    python3 -m venv "${ACADOS_SRC_DIR}/.venv"
    "${ACADOS_SRC_DIR}/.venv/bin/pip" install -q "${ACADOS_SRC_DIR}/interfaces/acados_template"
    mkdir -p "${ACADOS_SRC_DIR}/bin"
    curl -fL --retry 3 -o "${ACADOS_SRC_DIR}/bin/t_renderer" "${TERA_RENDERER_URL}"
    chmod +x "${ACADOS_SRC_DIR}/bin/t_renderer"
    ln -sfn "${ACADOS_PREFIX}/lib" "${ACADOS_SRC_DIR}/lib" 2>/dev/null || true
    ln -sfn "${ACADOS_PREFIX}/include" "${ACADOS_SRC_DIR}/include" 2>/dev/null || true
    log "acados ready (prefix ${ACADOS_PREFIX}, codegen tree ${ACADOS_SRC_DIR})"
}

# ---------------------------------------------------------------------------
# lidar_centerpoint flat-layout fixup
#
# The perception model bundle lands under ml_models/lidar_centerpoint/<subdir>/
# (e.g. tiny/) but lidar_centerpoint's launch expects the four files FLAT in
# ml_models/lidar_centerpoint/. Without them classical-mode perception dies at
# startup. Create flat symlinks from whatever subdir carries a matching file.
# Idempotent; no-op if the model dir does not exist yet.
# ---------------------------------------------------------------------------
fixup_centerpoint_layout() {
    [ -d "${CENTERPOINT_DATA_DIR}" ] || return 0
    log "Checking lidar_centerpoint flat layout in ${CENTERPOINT_DATA_DIR}"
    local f src missing=0
    for f in "${CENTERPOINT_FLAT_FILES[@]}"; do
        if [ -e "${CENTERPOINT_DATA_DIR}/${f}" ]; then
            echo "  ${f}: present"
            continue
        fi
        # exact-name match anywhere below (tiny/, base/, ...) wins
        src="$(find "${CENTERPOINT_DATA_DIR}" -mindepth 2 -type f -name "${f}" 2>/dev/null | sort | head -1)"
        if [ -z "${src}" ]; then
            # fuzzy fallback: subdir files often carry variant-suffixed names
            case "${f}" in
                pts_voxel_encoder.onnx)      src="$(find "${CENTERPOINT_DATA_DIR}" -mindepth 2 -type f -name 'pts_voxel_encoder*.onnx' 2>/dev/null | sort | head -1)" ;;
                pts_backbone_neck_head.onnx) src="$(find "${CENTERPOINT_DATA_DIR}" -mindepth 2 -type f -name 'pts_backbone_neck_head*.onnx' 2>/dev/null | sort | head -1)" ;;
                centerpoint_tiny_ml_package.param.yaml) src="$(find "${CENTERPOINT_DATA_DIR}" -mindepth 2 -type f -name '*tiny*ml_package*.param.yaml' 2>/dev/null | sort | head -1)" ;;
            esac
        fi
        if [ -n "${src}" ]; then
            # RELATIVE link: the dir is bind-mounted into containers at a
            # different absolute path, where an absolute target would dangle.
            ln -s "${src#"${CENTERPOINT_DATA_DIR}"/}" "${CENTERPOINT_DATA_DIR}/${f}"
            echo "  ${f}: symlinked -> ${src#"${CENTERPOINT_DATA_DIR}"/}"
        else
            echo "  ${f}: MISSING (no candidate found in subdirs)"
            missing=1
        fi
    done
    if [ "${missing}" -eq 1 ]; then
        warn "lidar_centerpoint flat layout incomplete -- classical-mode lidar perception
(perception_mode:=lidar) will fail to start until the files above exist in
${CENTERPOINT_DATA_DIR}/ (flat, not in a subdir)."
    fi
}

checkout_vad_launch_branch() {
    log "Checking out autoware_launch PR #1685 branch (${VAD_LAUNCH_BRANCH})"
    local dir
    dir="$(find "${WORKSPACE}/src" -maxdepth 4 -type d -name autoware_launch \
             -exec test -e '{}/.git' ';' -print 2>/dev/null | head -1)"
    [ -n "${dir}" ] || die "autoware_launch checkout not found under ${WORKSPACE}/src (run the vcs import first)"
    if git -C "${dir}" fetch "${VAD_LAUNCH_REPO_URL}" "${VAD_LAUNCH_BRANCH}" 2>/dev/null; then
        git -C "${dir}" checkout -B "${VAD_LAUNCH_BRANCH}" FETCH_HEAD
        echo "autoware_launch now on ${VAD_LAUNCH_BRANCH} (PR #1685, verified open as of 2026-08-19)."
        warn "the PR branch predates the autoware_launch restructure that vendored the
tier4_*_launch packages (present from tag 0.52.0) -- as-is it CANNOT launch
against universe >= 0.52 (missing tier4_vehicle_launch etc. also breaks
rosdep). Validated fix (2026-08-24): cherry-pick the PR's feature commit onto
the pinned autoware_launch tag, e.g.
  git -C ${dir} checkout -B vad-on-pinned <pinned-tag>
  git -C ${dir} cherry-pick 90d1465d   # resolve: keep the tag's include args,
                                       # take the PR's e2e wiring and gating
and skip the follow-up pre-commit style commit. Then apply the required
workspace patches from $(dirname "$0")/patches/ (gating string fix, STOPPED
departure fix, vad_node TF-miss segfault fix, 800x450 rig input) -- see
patches/README.md for the full validated sequence."
    else
        warn "could not fetch branch '${VAD_LAUNCH_BRANCH}' from ${VAD_LAUNCH_REPO_URL}.
If PR #1685 has since been MERGED, the branch was likely deleted and the VAD glue
now lives on autoware_launch main (use_e2e_planning:=true e2e_planning_type:=vad).
Check https://github.com/autowarefoundation/autoware_launch/pull/1685 and either
check out main in ${dir} or launch
e2e/autoware_tensorrt_vad/launch/vad_carla_tiny.launch.xml manually."
        return 1
    fi
}

# ---------------------------------------------------------------------------
# --source : source build
# ---------------------------------------------------------------------------
do_source() {
    detect_ubuntu
    detect_ros_distro
    require_distro_os_match
    resolve_source_version

    for t in git vcs rosdep colcon; do
        have "$t" || die "'$t' is required for a source build (see --check for install hints)"
    done

    log "Source-building Autoware ${VERSION} (ROS 2 ${ROS_DISTRO_DETECTED}, Ubuntu ${UBUNTU_VERSION})"
    echo "Workspace : ${WORKSPACE}"
    echo "Jobs      : ${JOBS} (MAKEFLAGS -j${JOBS} + colcon --parallel-workers ${JOBS}; override with --jobs)"
    if [ "${VERSION}" = "${TIERIV_BASELINE_TAG}" ]; then
        echo "Note      : ${TIERIV_BASELINE_TAG} is the TIER IV-validated baseline for CARLA native ROS2."
    elif [ "${ROS_DISTRO_DETECTED}" = "humble" ] && [ "${VERSION}" != "${TIERIV_BASELINE_TAG}" ]; then
        echo "Note      : TIER IV validated CARLA against ${TIERIV_BASELINE_TAG}; you chose ${VERSION}."
    fi

    # 1. Clone the meta-repo at the release tag.
    if [ -d "${WORKSPACE}/.git" ]; then
        log "Workspace exists; fetching tag ${VERSION}"
        git -C "${WORKSPACE}" fetch --tags origin
        git -C "${WORKSPACE}" checkout "${VERSION}"
    else
        log "Cloning ${AUTOWARE_REPO_URL} @ ${VERSION}"
        git clone --branch "${VERSION}" --depth 1 "${AUTOWARE_REPO_URL}" "${WORKSPACE}"
    fi
    cd "${WORKSPACE}"

    # 2. vcs import. The .repos file moved between releases:
    #    0.45.1 has ./autoware.repos, 1.x has ./repositories/autoware.repos
    #    (both layouts verified against the tagged trees).
    local repos_file=""
    for c in repositories/autoware.repos autoware.repos; do
        [ -f "${c}" ] && { repos_file="${c}"; break; }
    done
    [ -n "${repos_file}" ] || die "no autoware.repos found in ${WORKSPACE} (unexpected layout at ${VERSION})"
    log "Importing repositories from ${repos_file}"
    mkdir -p src
    vcs import --retry 3 src < "${repos_file}"

    # 3. Optional: VAD launch glue (must happen before rosdep/build so its deps build too).
    if [ "${WITH_VAD}" -eq 1 ]; then
        checkout_vad_launch_branch || true
        fetch_vad_model
    fi

    # 4. rosdep (non-interactive; see header for why we skip the ansible playbook).
    #    --skip-keys nebula_sensor_driver: exec_depend of the vendored sensor-kit
    #    launch packages, resolvable neither via apt nor src at current pins.
    #    It is a real-lidar hardware driver -- irrelevant against a simulator.
    log "Installing ROS dependencies via rosdep (needs sudo for apt)"
    # ROS environment scripts are not nounset-clean (e.g. they read
    # AMENT_TRACE_SETUP_FILES unguarded); relax -u around the source.
    set +u
    # shellcheck disable=SC1090
    source "/opt/ros/${ROS_DISTRO_DETECTED}/setup.bash"
    set -u
    if [ ! -e /etc/ros/rosdep/sources.list.d/20-default.list ]; then
        sudo rosdep init
    fi
    rosdep update
    rosdep install -y --from-paths src --ignore-src --rosdistro "${ROS_DISTRO_DETECTED}" \
        --skip-keys nebula_sensor_driver
    # Autoware runs on CycloneDDS (rmw_cyclonedds_cpp), but ros-*-desktop only
    # ships the FastDDS default and no package declares the RMW as a dep --
    # without this, every node dies at startup with "failed to load shared
    # library 'librmw_cyclonedds_cpp.so'".
    sudo apt-get install -y "ros-${ROS_DISTRO_DETECTED}-rmw-cyclonedds-cpp"

    # 4b. acados for autoware_path_optimizer's build-time MPC codegen.
    install_acados
    export ACADOS_SOURCE_DIR="${ACADOS_SRC_DIR}"
    export CMAKE_PREFIX_PATH="${ACADOS_PREFIX}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"

    # 5. Bounded-parallelism build. Unbounded colcon+make on this class of
    #    machine triggers the OOM killer; cap both the package-level and the
    #    per-package make/ninja parallelism.
    #    An activated conda/virtualenv poisons the build: CMake's FindPython3
    #    prefers the active env (Python3_FIND_VIRTUALENV=FIRST) regardless of
    #    PATH order, and e.g. anaconda's python lacks catkin_pkg -- every
    #    ament package then fails at configure. Scrub the markers.
    unset CONDA_PREFIX CONDA_DEFAULT_ENV CONDA_EXE CONDA_PYTHON_EXE CONDA_SHLVL \
          VIRTUAL_ENV PYTHONPATH PYTHONHOME 2>/dev/null || true
    log "Building with colcon (bounded: ${JOBS} workers)"
    export MAKEFLAGS="-j${JOBS}"
    export CMAKE_BUILD_PARALLEL_LEVEL="${JOBS}"
    colcon build \
        --symlink-install \
        --parallel-workers "${JOBS}" \
        --cmake-args -DCMAKE_BUILD_TYPE=Release

    # Perception model layout fix (see function header).
    fixup_centerpoint_layout

    log "Done"
    cat <<EOF
Source build complete.

Next steps:
  source ${WORKSPACE}/install/setup.bash
  export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp   # Autoware's recommended RMW
  export LD_LIBRARY_PATH=${ACADOS_PREFIX}/lib:\$LD_LIBRARY_PATH   # path_optimizer links libacados at runtime
  # start CARLA with native ROS2 (one ticking client, sync mode), then:
  ros2 launch autoware_launch e2e_simulator.launch.xml \\
      vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit \\
      map_path:=/path/to/town_map_dir
EOF
    if [ "${WITH_VAD}" -eq 1 ]; then
        cat <<EOF

VAD notes:
  - Model files: ${VAD_DATA_DIR}
  - autoware_launch is on PR #1685 (${VAD_LAUNCH_BRANCH}); the VAD e2e path is
    enabled via that PR's launch arguments (use_e2e_planning / e2e_planning_type:=vad).
    Alternatively launch e2e/autoware_tensorrt_vad/launch/vad_carla_tiny.launch.xml
    directly with sensing/localization/perception disabled.
  - First VAD run builds TensorRT engines from the ONNX files (slow, cached).
EOF
    fi
}

# ---------------------------------------------------------------------------
# --docker : pull prebuilt images, document run pattern
# ---------------------------------------------------------------------------
do_docker() {
    have docker || die "docker is not installed"
    case "${DOCKER_DISTRO}" in humble|jazzy) : ;; *) die "--distro must be humble or jazzy" ;; esac

    local cuda_part=""
    if [ "${CUDA}" = "yes" ] || { [ "${CUDA}" = "auto" ] && have nvidia-smi; }; then
        cuda_part="-cuda"
    fi
    local tag="universe${cuda_part}-${DOCKER_DISTRO}"
    [ -n "${VERSION}" ] && tag="${tag}-${VERSION}"
    local image="${DOCKER_IMAGE_BASE}:${tag}"

    log "Pulling ${image}"
    echo "(official prebuilt image from GHCR; nothing is built locally)"
    docker pull "${image}"

    # Perception model layout fix (host dir is bind-mounted into the container).
    fixup_centerpoint_layout

    log "Run pattern"
    local gpu_flag=""
    [ -n "${cuda_part}" ] && gpu_flag="--gpus all "
    cat <<EOF
Image pulled: ${image}

The recommended way to run against CARLA is the orchestrator, which generates
the DDS configs, applies the CARLA-specific Autoware overrides and drives the
whole session:

  ../run/run_carla_autoware.sh --mode classical --stack docker --image ${image}

Manual run pattern (adjust paths):

  docker run --rm -it --net=host ${gpu_flag}\\
      -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \\
      -e CYCLONEDDS_URI=file:///dds/cyclonedds.xml \\
      -e ROS_DOMAIN_ID=\${ROS_DOMAIN_ID:-42} \\
      -v /path/to/generated/dds:/dds:ro \\
      -v \$HOME/autoware_map:/autoware_map \\
      -v \$HOME/autoware_data:/root/autoware_data \\
      ${image} \\
      ros2 launch autoware_launch e2e_simulator.launch.xml \\
          vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit \\
          perception_mode:=lidar rviz:=false \\
          map_path:=/autoware_map/<TownXX_dir>

Notes:
  - CYCLONEDDS_URI is MANDATORY: the official image ships a cyclonedds.xml that
    demands 10 MB socket buffers (most hosts cap rmem_max at 4 MB) and pins the
    'lo' interface, which breaks discovery with the simulator. Override it with
    a config pinned to the docker bridge interface (stable IP; NEVER a WiFi/DHCP
    interface) with MaxAutoParticipantIndex >= 300 -- run_carla_autoware.sh
    generates a validated one into its log dir (dds/cyclonedds.xml).
  - The validated topology is MIXED-RMW: the CARLA server runs Fast DDS
    (-rmw=fastdds, UDPv4 whitelisted to the bridge IP) while the Autoware
    container runs CycloneDDS. DDS vendors interoperate over UDPv4.
    Match -ros-domain-id on the CARLA side with ROS_DOMAIN_ID (default 42).
  - GUI (rviz2) runs as a separate container with the same DDS env: add
      -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix   (after: xhost +local:docker)
    and run: rviz2 -d /opt/autoware/autoware_launch/share/autoware_launch/rviz/autoware.rviz
  - map_path must contain pointcloud_map.pcd, lanelet2_map.osm (WITH the
    <MetaInfo format_version="1.0.0" map_version="1"/> element -- see
    map_tools/fetch_prebuilt_maps.sh) and map_projector_info.yaml
    (projector_type: Local).
  - Pin a release with --version (e.g. --version ${CURRENT_RELEASE_TAG} ->
    ${DOCKER_IMAGE_BASE}:universe${cuda_part}-${DOCKER_DISTRO}-${CURRENT_RELEASE_TAG}).
EOF

    if [ "${WITH_VAD}" -eq 1 ]; then
        fetch_vad_model
        cat <<EOF

VAD + docker note:
  The prebuilt images do NOT include autoware_launch PR #1685 (open, unmerged),
  so the VAD e2e glue is absent in-container. Options:
    - mount a source checkout of ${VAD_LAUNCH_REPO_URL} @ ${VAD_LAUNCH_BRANCH}
      over the container's autoware_launch and rebuild that package inside, or
    - use --source --with-vad for the fully wired workspace.
  The model directory above is mounted via -v \$HOME/autoware_data:/root/autoware_data.
EOF
    fi
}

# ---------------------------------------------------------------------------
case "${MODE}" in
    check)  do_check ;;
    source) do_source ;;
    docker) do_docker ;;
esac
