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

CONTENT_FOLDER="${SCRIPT_DIR}/Unreal/CarlaUE4/Content/Carla"

CONTENT_REPO=https://bitbucket.org/carla-simulator/carla-content.git
CONTENT_ID=$(tac $SCRIPT_DIR/Util/ContentVersions.txt | egrep -m 1 . | rev | cut -d' ' -f1 | rev)
CONTENT_COMMIT=${CONTENT_ID##*_}

VERSION_FILE="${CONTENT_FOLDER}/.version"

function download_content {
  if [[ -d "$CONTENT_FOLDER" ]]; then
    echo "Backing up existing Content..."
    mv -v "$CONTENT_FOLDER" "${CONTENT_FOLDER}_$(date +%Y%m%d%H%M%S)"
  fi
  # Skip the LFS smudge until the pinned commit is checked out, otherwise the
  # blobs of the default branch get downloaded too.
  GIT_LFS_SKIP_SMUDGE=1 git clone "$CONTENT_REPO" "$CONTENT_FOLDER"
  GIT_LFS_SKIP_SMUDGE=1 git -C "$CONTENT_FOLDER" checkout ${CONTENT_COMMIT}
  git -C "$CONTENT_FOLDER" lfs pull
  echo ".version" >> "${CONTENT_FOLDER}/.git/info/exclude"
  echo "$CONTENT_ID" > "$VERSION_FILE"
  echo "Content updated successfully."
}

# ==============================================================================
# -- Download Content if necessary ---------------------------------------------
# ==============================================================================

if $SKIP_DOWNLOAD ; then
  echo "Skipping 'Content' update. Please manually clone the content repository"
  echo
  echo "  git clone ${CONTENT_REPO} Unreal/CarlaUE4/Content/Carla"
  echo "  git -C Unreal/CarlaUE4/Content/Carla checkout ${CONTENT_COMMIT}"
  echo
  echo "into Unreal/CarlaUE4/Content/Carla."
  exit 0
fi

if [[ -f "$CONTENT_FOLDER/.version" ]]; then
  if [ "$CONTENT_ID" == `cat $VERSION_FILE` ]; then
    echo "Content is up-to-date."
  else
    download_content
  fi
elif [[ -d "$CONTENT_FOLDER/.git" ]]; then
  echo "Using git version of 'Content', skipping update."
else
  download_content
fi

popd >/dev/null
