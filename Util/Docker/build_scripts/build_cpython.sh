#!/bin/bash

set -e

DOC_STRING="Build CPython from source and install it side-by-side with the system Python via 'make altinstall'."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--python-version PYTHON_VERSION]

  --python-version  Full CPython version to build (default: 3.10.14).
END
)

usage() { echo "$DOC_STRING"; echo "$USAGE_STRING"; exit 1; }

PYTHON_VERSION="3.10.14"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PYTHON_VERSION="$2"
      shift 2 ;;
    -h | --help )
      usage
      ;;
    * )
      shift ;;
  esac
done

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "${WORK_DIR}"' EXIT

pushd "${WORK_DIR}" >/dev/null

wget -q "https://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz"
tar -xzf "Python-${PYTHON_VERSION}.tgz"

pushd "Python-${PYTHON_VERSION}" >/dev/null
./configure --enable-optimizations --with-ensurepip=install
make -j"$(nproc)"
make altinstall
popd >/dev/null

popd >/dev/null
