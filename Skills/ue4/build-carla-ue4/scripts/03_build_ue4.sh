#!/usr/bin/env bash
# Step 03 — build the CarlaUnreal UE 4.26 fork.
#
# Prereq: the fork is already cloned at $UE4_ROOT:
#   git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git $UE4_ROOT
# (Requires a GitHub account linked to Epic Games; see SKILL.md "Access" note.)
#
# Produces ~80GB: the bundled clang-10 SDK, libc++, and UE4Editor binary that
# CARLA's C++ build links against. Must complete before step 04.
#
# IMPORTANT: UE4's top-level `make` must run single-threaded. Parallel UBT
# invocations race on generated files and OOM on large RAM-per-core ratios;
# the official docs explicitly warn against -j here.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${HERE}/env.sh"

if [ ! -d "${UE4_ROOT}/Engine" ]; then
  echo "[ue4] ERROR: ${UE4_ROOT}/Engine not found."
  echo "       Clone the fork first:"
  echo "       git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git ${UE4_ROOT}"
  exit 1
fi

cd "${UE4_ROOT}"

# 1. Download commit dependencies + bundled toolchain (~10GB). Idempotent.
if [ ! -d "${UE4_ROOT}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64" ]; then
  echo "[ue4] running Setup.sh (downloads bundled clang SDK + deps)..."
  ./Setup.sh
else
  echo "[ue4] Setup.sh already done (HostLinux SDK present) — skipping."
fi

# 2. Generate Makefiles.
echo "[ue4] GenerateProjectFiles.sh..."
./GenerateProjectFiles.sh

# 3. Build engine + editor. Single-threaded by design.
if [ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ]; then
  echo "[ue4] UE4Editor already built — skipping make."
else
  echo "[ue4] make (single-threaded, ~1h)..."
  make
fi

echo "[ue4] DONE. UE4Editor:"
ls -la "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor"
