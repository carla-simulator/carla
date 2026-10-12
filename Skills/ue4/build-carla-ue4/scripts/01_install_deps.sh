#!/usr/bin/env bash
# Step 01 — install system build dependencies (Ubuntu 22.04 / 24.04).
#
# Base list is the official CARLA Linux build set (22.04). Ubuntu 24.04 deltas:
#   * lld           -- REQUIRED. UE4's bundled ld (2019) cannot read .relr.dyn
#                      sections emitted by glibc >= 2.36; Ubuntu24Compat.sh wraps
#                      the compiler with -fuse-ld=lld and HARD-FAILS if lld is
#                      absent.
#   * libtiff-dev   -- replaces the libtiff5-dev transitional package.
#   * g++-12        -- present on both 22.04 and 24.04 repos.
#
# UE4 ships its own clang-10 + libc++ toolchain (downloaded by UE Setup.sh), so
# no system clang is needed for the CARLA C++ build.
set -euo pipefail

SUDO=""
[ "$(id -u)" -ne 0 ] && SUDO="sudo"

. /etc/os-release
echo "[deps] detected: ${PRETTY_NAME} (VERSION_ID=${VERSION_ID})"

PKGS=(
  build-essential g++-12 cmake ninja-build libvulkan1
  python3 python3-dev python3-pip python3-venv
  autoconf libtool
  wget curl rsync unzip git git-lfs
  libpng-dev libtiff-dev libjpeg-dev
  lld                       # 24.04 / glibc>=2.36 linker fix
)

$SUDO apt-get update
$SUDO apt-get install -y --no-install-recommends "${PKGS[@]}"

git lfs install || true

echo "[deps] verifying critical tools..."
fail=0
for t in cmake ninja git git-lfs ld.lld g++-12; do
  if command -v "$t" >/dev/null 2>&1; then
    echo "  ok   $t -> $(command -v "$t")"
  else
    echo "  MISS $t"; fail=1
  fi
done
[ "$fail" -eq 0 ] && echo "[deps] all critical tools present." || { echo "[deps] missing tools above"; exit 1; }
