#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DOC_STRING="Build CPython."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--python-version PYTHON_VERSION]

END
)

usage() { echo "$DOC_STRING"; echo "$USAGE_STRING"; exit 1; }

PYTHON_VERSION="3.8.19"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PYTHON_VERSION=$2
      shift 2 ;;
    -h | --help )
      usage
      ;;
    * )
      shift ;;
  esac
done

function pyver_major {
  echo $PYTHON_VERSION | awk -F "." '{printf "%d", $1}'
}

function pyver_minor {
  echo $PYTHON_VERSION | awk -F "." '{printf "%d", $2}'
}

function pyver_micro {
  echo $PYTHON_VERSION | awk -F "." '{printf "%d", $3}'
}

wget https://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz
tar -xvzf Python-${PYTHON_VERSION}.tgz
pushd Python-${PYTHON_VERSION}
./configure
make altinstall
popd

rm -rf Python-${PYTHON_VERSION}
rm -rf Python-${PYTHON_VERSION}.tgz
