#! /bin/bash
DOC_STRING="Build STREETMAP."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)

REMOVE_INTERMEDIATE=false
BUILD_STREETMAP=false
GIT_PULL=true
CURRENT_STREETMAP_COMMIT=78e5c40e0ce1b6d109c79b59bd84f7f24e35e347
STREETMAP_BRANCH=master
STREETMAP_REPO=https://github.com/carla-simulator/StreetMap.git

OPTS=`getopt -o h --long help,rebuild,build,clean,carsim,no-pull -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_STREETMAP=true;
      shift ;;
    --build )
      BUILD_STREETMAP=true;
      shift ;;
    --no-pull )
      GIT_PULL=false
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

source $(dirname "$0")/Environment.sh

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_STREETMAP}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${CARLAUE4_STREETMAP_FOLDER}*

fi

# ==============================================================================
# -- Build library -------------------------------------------------------------
# ==============================================================================

if ! [ -d ${CARLAUE4_STREETMAP_FOLDER} ]; then
  log "Downloading STREETMAP plugin."
  # [ ! -d ${STREETMAP_BUILD_FOLDER} ] && mkdir ${STREETMAP_BUILD_FOLDER}
  if ${GIT_PULL} ; then
    if [ ! -d ${CARLAUE4_STREETMAP_FOLDER} ] ; then
      git clone -b ${STREETMAP_BRANCH} ${STREETMAP_REPO} ${CARLAUE4_STREETMAP_FOLDER}
    fi
    cd ${CARLAUE4_STREETMAP_FOLDER}
    git fetch
    git checkout ${CURRENT_STREETMAP_COMMIT}
  fi
fi

log " STREETMAP Success!"
