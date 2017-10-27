#! /bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" >/dev/null

PROTOBUF_SRC_DIR=Proto
PROTOBUF_CPP_OUT_DIR=CarlaServer/source/carla/server
PROTOBUF_PY_OUT_DIR0=../PythonClient/carla/protoc
PROTOBUF_PY_OUT_DIR1=TestingClient/carla
PROTO_BASENAME=carla_server

if [ "$1" == "--clean" ]; then
  # Delete existing ones.
  rm -f ${PROTOBUF_CPP_OUT_DIR}/*.pb.h ${PROTOBUF_CPP_OUT_DIR}/*.pb.cc
  rm -f ${PROTOBUF_PY_OUT_DIR0}/*_pb2.py
  rm -f ${PROTOBUF_PY_OUT_DIR1}/*_pb2.py
  exit 0
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./Build/llvm-install/lib
PROTOC=./Build/protobuf-install/bin/protoc

if [[ ! -f $PROTOC ]]; then
  echo >&2 "ERROR: Missing protobuf compiler."
  echo >&2 "Did you forget to run Setup.sh?"
  exit 1
fi

echo "Compiling ${PROTO_BASENAME}.proto..."

${PROTOC} \
    -I=${PROTOBUF_SRC_DIR} \
    --cpp_out=${PROTOBUF_CPP_OUT_DIR} \
    --python_out=${PROTOBUF_PY_OUT_DIR0} \
    --python_out=${PROTOBUF_PY_OUT_DIR1} \
  ${PROTOBUF_SRC_DIR}/${PROTO_BASENAME}.proto

popd >/dev/null
echo "done."
