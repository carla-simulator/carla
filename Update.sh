#! /bin/bash

################################################################################
# Updates CARLA contents.
################################################################################

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" >/dev/null

CONTENT_FOLDER=$SCRIPT_DIR/Unreal/CarlaUE4/Content

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
