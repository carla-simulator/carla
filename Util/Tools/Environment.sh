#! /bin/bash

# Sets the environment for other shell scripts.

set -e

# Directory containing these tool scripts. Import.sh and Prettify.sh interpolate
# ${CARLA_BUILD_TOOLS_FOLDER} to locate their Python entry points; while it was
# unset it expanded to empty and they invoked /Import.py. BASH_SOURCE, not $0,
# because this file is sourced: $0 is the caller.
CARLA_BUILD_TOOLS_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function log {
  echo "[`basename "$0"`]: $1"
}

function fatal_error {
  echo -e >&2 "[`basename "$0"`]: ERROR: $1"
  exit 2
}

