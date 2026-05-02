#! /bin/bash

# Sets the environment for other shell scripts.

set -e

function log {
  echo "[`basename "$0"`]: $1"
}

function fatal_error {
  echo -e >&2 "[`basename "$0"`]: ERROR: $1"
  exit 2
}

