#!/usr/bin/env bash
# Self-contained environment for the download-carla skill.
# Source before the other scripts:  source env.sh
#
# This skill only fetches CARLA. It builds nothing, launches nothing, and needs no
# existing CARLA — it is the step BEFORE every other skill.
#
#   CARLA_DOWNLOAD_DIR  where downloads land (default ~/carla-downloads). Whatever
#                       it ends up as, the script prints the resulting path and the
#                       exports the other skills read (CARLA_TARGET for a release,
#                       CARLA_UE4_ROOT for a checkout).
#
# Sets no shell options: this file is sourced.

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_DOWNLOAD_DIR="${CARLA_DOWNLOAD_DIR:-${HOME}/carla-downloads}"

# The interpreter used to run this skill's own script. Unlike the rest of the
# collection this needs no `carla` module, so any python3 will do — including an
# isolated MCP server's own.
export PYTHON="${PYTHON:-python3}"

echo "[env] CARLA_DOWNLOAD_DIR = ${CARLA_DOWNLOAD_DIR}"
