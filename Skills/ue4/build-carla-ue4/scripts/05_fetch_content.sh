#!/usr/bin/env bash
# Step 05 — fetch the CARLA map/asset Content (~31GB).
#
# Two official methods. We use the GIT method (preferred for development /
# committing content), per user choice:
#
#   git clone -b master https://bitbucket.org/carla-simulator/carla-content \
#       ${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla
#
# (The script alternative is Update.sh, which pulls a versioned tarball.)
#
# Network + disk heavy. Independent of the UE4 build, so it can run in parallel.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${HERE}/env.sh"

DEST="${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla"

if [ -d "${DEST}/.git" ]; then
  echo "[content] already cloned at ${DEST} — pulling latest."
  git -C "${DEST}" pull --ff-only || true
else
  mkdir -p "$(dirname "${DEST}")"
  echo "[content] cloning carla-content (master) -> ${DEST} (~31GB)..."
  git clone -b master https://bitbucket.org/carla-simulator/carla-content "${DEST}"
fi

echo "[content] size:"; du -sh "${DEST}" 2>/dev/null
echo "[content] DONE."
