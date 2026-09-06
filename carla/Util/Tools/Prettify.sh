#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Prettify code files."

USAGE_STRING="Usage: $0 [-h|--help] [--all] [-f path|--file=path]"

PRETTIFY_ALL=false
PRETTIFY_FILE=false

OPTS=`getopt -o hf: --long help,all,file: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --all )
      PRETTIFY_ALL=true;
      shift ;;
    -f | --file )
      PRETTIFY_FILE="$2";
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

if ! { ${PRETTIFY_ALL} || [ -n "${PRETTIFY_FILE}" ]; } ; then
  fatal_error "Nothing selected to be done."
fi

if ${PRETTIFY_ALL} ; then
  PRETTIFY_FILE=false
elif [[ ! -f ${PRETTIFY_FILE} ]] ; then
  pwd
  fatal_error "\"${PRETTIFY_FILE}\" no such file."
fi

# ==============================================================================
# -- Get latest version of uncrustify ------------------------------------------
# ==============================================================================

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

UNCRUSTIFY_BASENAME=uncrustify-0.69.0

UNCRUSTIFY=${PWD}/${UNCRUSTIFY_BASENAME}-install/bin/uncrustify

if [[ -d "${UNCRUSTIFY_BASENAME}-install" ]] ; then
  log "${UNCRUSTIFY_BASENAME} already installed."
else
  rm -Rf ${UNCRUSTIFY_BASENAME}-source ${UNCRUSTIFY_BASENAME}-build

  log "Retrieving Uncrustify."

  git clone --depth=1 -b ${UNCRUSTIFY_BASENAME} https://github.com/uncrustify/uncrustify.git ${UNCRUSTIFY_BASENAME}-source

  log "Building Uncrustify."

  mkdir -p ${UNCRUSTIFY_BASENAME}-build

  pushd ${UNCRUSTIFY_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="../${UNCRUSTIFY_BASENAME}-install" \
      ../${UNCRUSTIFY_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  rm -Rf ${UNCRUSTIFY_BASENAME}-source ${UNCRUSTIFY_BASENAME}-build

fi

command -v ${UNCRUSTIFY} >/dev/null 2>&1 || {
  fatal_error "Failed to install Uncrustify!";
}

popd >/dev/null

command -v autopep8 >/dev/null 2>&1 || {
  log "Installing autopep8 for this user."
  pip3 install --user autopep8
}

# ==============================================================================
# -- Run uncrustify and/or autopep8 --------------------------------------------
# ==============================================================================

UNCRUSTIFY_CONFIG=${CARLA_BUILD_TOOLS_FOLDER}/uncrustify.cfg
UNCRUSTIFY_UE4_CONFIG=${CARLA_BUILD_TOOLS_FOLDER}/uncrustify-ue4.cfg
UNCRUSTIFY_COMMAND="${UNCRUSTIFY} --no-backup --replace"

AUTOPEP8_COMMAND="autopep8 --jobs 0 --in-place -a"

if ${PRETTIFY_ALL} ; then

  fatal_error "Prettify all not yet supported"

  # find ${CARLA_ROOT_FOLDER} -iregex '.*\.\(py\)$' -exec ${AUTOPEP8_COMMAND} {} +
  # find ${LIBCARLA_ROOT_FOLDER} -iregex '.*\.\(h\|cpp\)$' -exec ${UNCRUSTIFY_COMMAND} {} \;

elif [[ -f ${PRETTIFY_FILE} ]] ; then

  if [[ ${PRETTIFY_FILE} == *.py ]] ; then
    log "autopep8 ${PRETTIFY_FILE}"
    ${AUTOPEP8_COMMAND} ${PRETTIFY_FILE}
  elif [[ ${PRETTIFY_FILE} == *Unreal/CarlaUnreal/* ]] ; then
    log "uncrustify for UE4 ${PRETTIFY_FILE}"
    ${UNCRUSTIFY_COMMAND} -c ${UNCRUSTIFY_UE4_CONFIG} ${PRETTIFY_FILE}
  else
    log "uncrustify ${PRETTIFY_FILE}"
    ${UNCRUSTIFY_COMMAND} -c ${UNCRUSTIFY_CONFIG} ${PRETTIFY_FILE}
  fi

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
