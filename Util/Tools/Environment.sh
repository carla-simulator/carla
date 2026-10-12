#! /bin/bash

# Sets the environment for other shell scripts.

set -e

# Directory containing these tool scripts.
CARLA_BUILD_TOOLS_FOLDER="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function log {
  echo "[`basename "$0"`]: $1"
}

function fatal_error {
  echo -e >&2 "[`basename "$0"`]: ERROR: $1"
  exit 2
}

