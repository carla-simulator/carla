#! /bin/bash

# Sets the environment for other shell scripts.

set -e

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
source $(dirname "$0")/Vars.mk
unset CURDIR

if [ -n "${CARLA_BUILD_NO_COLOR}" ]; then

  function log {
      echo "`basename "$0"`: $1"
  }

  function fatal_error {
    echo -e >&2 "`basename "$0"`: ERROR: $1"
    exit 2
  }

else

  function log {
    echo -e "\033[1;35m`basename "$0"`: $1\033[0m"
  }

  function fatal_error {
    echo -e >&2 "\033[0;31m`basename "$0"`: ERROR: $1\033[0m"
    exit 2
  }

fi

function get_git_repository_version {
  git describe --tags --dirty --always
}

function copy_if_changed {
  mkdir -p $(dirname $2)
  rsync -cIr --out-format="%n" $1 $2
}

function move_if_changed {
  copy_if_changed $1 $2
  rm -f $1
}

CARLA_BUILD_CONCURRENCY=`nproc --all`
