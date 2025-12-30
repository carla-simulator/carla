#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DOC_STRING="Build UE4.26"

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] --ue4-root UE4_ROOT --epic-user EPIC_USER --epic-token EPIC_TOKEN

END
)

usage() { echo "$DOC_STRING"; echo "$USAGE_STRING"; exit 1; }

UE4_ROOT=
EPIC_USER=
EPIC_TOKEN=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ue4-root )
      UE4_ROOT=$2
      shift 2 ;;
    --epic-user )
      EPIC_USER=$2
      shift 2 ;;
    --epic-token )
      EPIC_TOKEN=$2
      shift 2 ;;
    -h | --help )
      usage
      ;;
    * )
      shift ;;
  esac
done

if [ -z "$UE4_ROOT" ]; then
  echo "Error: 'UE4_ROOT' parameter is mandatory"
  exit 1
fi

if [ -z "$EPIC_USER" ]; then
  echo "Error: 'EPIC_USER' parameter is mandatory"
  exit 1
fi

if [ -z "$EPIC_TOKEN" ]; then
  echo "Error: 'EPIC_TOKEN' parameter is mandatory"
  exit 1
fi

git clone --depth 1 -b carla "https://${EPIC_USER}:${EPIC_TOKEN}@github.com/CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}

pushd $UE4_ROOT
./Setup.sh
./GenerateProjectFiles.sh
make
popd
