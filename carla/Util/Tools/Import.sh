#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Import maps"

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--python-version=VERSION]
END
)

OPTS=`getopt -o h --long batch:,package:,no-carla-materials,json-only,python-version:, -n 'parse-options' -- "$@"`

ARGS=""

eval set -- "$OPTS"

source $(dirname "$0")/Environment.sh

PY_VERSION_LIST=3

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PY_VERSION_LIST="$2"
      shift 2 ;;
    -h | --help )
      echo "$DOC_STRING"
      echo -e "$USAGE_STRING"
      exit 1
      ;;
    --batch )
      ARGS="${ARGS} $1 $2"
      shift 2 ;;
    --package )
      ARGS="${ARGS} $1 $2"
      shift 2 ;;
    -- )
      shift ;;
    * )
      ARGS="${ARGS} $1"
      shift ;;
  esac
done

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

/usr/bin/env python${PY_VERSION_LIST[0]} ${CARLA_BUILD_TOOLS_FOLDER}/Import.py ${ARGS}
