#! /bin/bash

################################################################################
# Updates CARLA content.
################################################################################

set -e

DOC_STRING="Update CARLA content to the latest version, to be run after 'git pull'."

USAGE_STRING="Usage: $0 [-h|--help] [-s|--skip-download]"

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

SKIP_DOWNLOAD=false

OPTS=`getopt -o hs --long help,skip-download -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    -s | --skip-download )
      SKIP_DOWNLOAD=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      break ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" >/dev/null

CONTENT_FOLDER=$SCRIPT_DIR/Unreal/CarlaUE4/Content/Carla

CONTENT_GDRIVE_ID=$(tac $SCRIPT_DIR/Util/ContentVersions.txt | egrep -m 1 . | rev | cut -d' ' -f1 | rev)

VERSION_FILE=${CONTENT_FOLDER}/.version

function download_content {
  if [[ -d "$CONTENT_FOLDER" ]]; then
    echo "Backing up existing Content..."
    mv -v "$CONTENT_FOLDER" "${CONTENT_FOLDER}_$(date +%Y%m%d%H%M%S)"
  fi
  mkdir -p $CONTENT_FOLDER
  mkdir -p Content
  ./Util/download_from_gdrive.py $CONTENT_GDRIVE_ID Content.tar.gz
  tar -xvzf Content.tar.gz -C Content
  rm Content.tar.gz
  mv Content/* $CONTENT_FOLDER
  echo "$CONTENT_GDRIVE_ID" > "$VERSION_FILE"
  echo "Content updated successfully."
}

# ==============================================================================
# -- Download Content if necessary ---------------------------------------------
# ==============================================================================

if $SKIP_DOWNLOAD ; then
  echo "Skipping 'Content' update. Please manually download the package from"
  echo
  echo "  https://drive.google.com/open?id=$CONTENT_GDRIVE_ID"
  echo
  echo "and extract it under Unreal/CarlaUE4/Content/Carla."
  exit 0
fi

if [[ -d "$CONTENT_FOLDER/.git" ]]; then
  echo "Using git version of 'Content', skipping update."
elif [[ -f "$CONTENT_FOLDER/.version" ]]; then
  if [ "$CONTENT_GDRIVE_ID" == `cat $VERSION_FILE` ]; then
    echo "Content is up-to-date."
  else
    download_content
  fi
else
  download_content
fi

popd >/dev/null
