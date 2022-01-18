#! /bin/bash

# Sets the environment for other shell scripts.

set -e

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"
source $(dirname "$0")/Vars.mk
unset CURDIR


if [[ "$(uname)" == "Darwin" ]] ; then
  export MAC_OS=true # automatically set to false on non-Mac builds
else
  export MAC_OS=false
fi

if ${MAC_OS}; then
  ARCH="x86_64" # for building the UE4 package
  # ARCH="arm64" # for building the PythonAPI
  # for OSX apple silicon build (building x86 & using rosetta2)
  # NOTE: UE4 wants use of macos 10.14, but 12.1 works fine
  export ARCH_TARGET="-target ${ARCH}-apple-macos12.1"
  export OS_FLAGS=" -nostdinc++" # for macos
  export OS_STDLIB="-stdlib=libc++"
  # for cmake -arch flag: https://cmake.org/cmake/help/latest/prop_tgt/OSX_ARCHITECTURES.html#prop_tgt:OSX_ARCHITECTURES
  export CMAKE_OSX_ARCHITECTURES=${ARCH}
  export TARGET_PLATFORM="Mac"
else
  export ARCH_TARGET="" # use default arch on linux
  export OS_FLAGS="" 
  export OS_STDLIB=""
  export TARGET_PLATFORM="Linux"
fi

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