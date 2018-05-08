#! /bin/bash

################################################################################
# CARLA Setup.sh
#
# This script sets up the environment and dependencies for compiling CARLA on
# Linux.
#
#   1) Download CARLA Content if necessary.
#   2) Download and compile libc++.
#   3) Download other third-party libraries and compile them with libc++.
#
# Thanks to the people at https://github.com/Microsoft/AirSim for providing the
# important parts of this script.
################################################################################

set -e

DOC_STRING="Download and compile CARLA content and dependencies."

USAGE_STRING="Usage: $0 [-h|--help] [-s|--skip-download] [--jobs=N]"

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

UPDATE_SCRIPT_FLAGS=
NUMBER_OF_ASYNC_JOBS=1

OPTS=`getopt -o hs --long help,skip-download,jobs:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    -s | --skip-download )
      UPDATE_SCRIPT_FLAGS=--skip-download;
      shift ;;
    --jobs)
        case "$2" in
          "") NUMBER_OF_ASYNC_JOBS=4 ; shift 2 ;;
          *) NUMBER_OF_ASYNC_JOBS=$2 ; shift 2 ;;
        esac ;;
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

# Require clang 3.9
command -v clang++-3.9 >/dev/null 2>&1 || {
  echo >&2 "clang 3.9 is required, but it's not installed.";
  echo >&2 "make sure you build Unreal Engine with clang 3.9 too.";
  exit 1;
}

mkdir -p Util/Build
pushd Util/Build >/dev/null

# ==============================================================================
# -- Get and compile libc++ ----------------------------------------------------
# ==============================================================================

# Get libc++ source
if [[ ! -d "llvm-source" ]]; then
  echo "Retrieving libc++..."
  git clone --depth=1 -b release_39  https://github.com/llvm-mirror/llvm.git llvm-source
  git clone --depth=1 -b release_39  https://github.com/llvm-mirror/libcxx.git llvm-source/projects/libcxx
  git clone --depth=1 -b release_39  https://github.com/llvm-mirror/libcxxabi.git llvm-source/projects/libcxxabi
else
  echo "Folder llvm-source already exists, skipping git clone..."
fi

# Build libc++
rm -rf llvm-build
mkdir -p llvm-build

pushd llvm-build >/dev/null

export C_COMPILER=clang-3.9
export COMPILER=clang++-3.9

cmake -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
      -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF -DLIBCXX_INSTALL_EXPERIMENTAL_LIBRARY=OFF \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="../llvm-install" \
            ../llvm-source

make -j $NUMBER_OF_ASYNC_JOBS cxx

#install libc++ locally in llvm-install folder
make -j $NUMBER_OF_ASYNC_JOBS install-libcxx install-libcxxabi

popd >/dev/null

# ==============================================================================
# -- Get Boost and compile it with libc++ --------------------------------------
# ==============================================================================

# Get boost source
if [[ ! -d "boost-source" ]]; then
  echo "Retrieving boost..."
  wget https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
  tar -xvzf boost_1_64_0.tar.gz
  rm boost_1_64_0.tar.gz
  mv boost_1_64_0 boost-source
else
  echo "Folder boost-source already exists, skipping download..."
fi

pushd boost-source >/dev/null

BOOST_TOOLSET="clang-3.9"
BOOST_CFLAGS="-fPIC -std=c++1y -stdlib=libc++ -I../llvm-install/include/c++/v1"
BOOST_LFLAGS="-stdlib=libc++ -L../llvm-install/lib"

./bootstrap.sh \
    --with-toolset=clang \
    --prefix=../boost-install \
    --with-libraries=system
./b2 clean
./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" linkflags="${BOOST_LFLAGS}" --prefix="../boost-install" -j $NUMBER_OF_ASYNC_JOBS stage release
./b2 install toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" linkflags="${BOOST_LFLAGS}" --prefix="../boost-install" -j $NUMBER_OF_ASYNC_JOBS

popd >/dev/null

# ==============================================================================
# -- Get Protobuf and compile it with libc++ -----------------------------------
# ==============================================================================

# Get protobuf source
if [[ ! -d "protobuf-source" ]]; then
  echo "Retrieving protobuf..."
  git clone --depth=1 -b v3.3.0 --recurse-submodules https://github.com/google/protobuf.git protobuf-source
else
  echo "Folder protobuf-source already exists, skipping git clone..."
fi

pushd protobuf-source >/dev/null

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD/../llvm-install/lib/"

./autogen.sh
./configure \
    CC="clang-3.9" \
    CXX="clang++-3.9" \
    CXXFLAGS="-fPIC -stdlib=libc++ -I$PWD/../llvm-install/include/c++/v1" \
    LDFLAGS="-stdlib=libc++ -L$PWD/../llvm-install/lib/" \
    --prefix="$PWD/../protobuf-install" \
    --disable-shared
make -j $NUMBER_OF_ASYNC_JOBS
make -j $NUMBER_OF_ASYNC_JOBS install

popd >/dev/null

# ==============================================================================
# -- Get GTest and compile it with libc++ --------------------------------------
# ==============================================================================

# Get googletest source
if [[ ! -d "googletest-source" ]]; then
  echo "Retrieving googletest..."
  git clone --depth=1 -b release-1.8.0 https://github.com/google/googletest.git googletest-source
else
  echo "Folder googletest-source already exists, skipping git clone..."
fi

pushd googletest-source >/dev/null

cmake -H. -B./build \
    -DCMAKE_C_COMPILER=${C_COMPILER} -DCMAKE_CXX_COMPILER=${COMPILER} \
    -DCMAKE_CXX_FLAGS="-stdlib=libc++ -I$PWD/../llvm-install/include/c++/v1 -Wl,-L$PWD/../llvm-install/lib" \
    -DCMAKE_INSTALL_PREFIX="../googletest-install" \
    -G "Ninja"

pushd build >/dev/null
ninja
ninja install
popd >/dev/null

popd >/dev/null

# ==============================================================================
# -- Other CARLA files ---------------------------------------------------------
# ==============================================================================

popd >/dev/null

CARLA_SETTINGS_FILE="./Unreal/CarlaUE4/Config/CarlaSettings.ini"

if [[ ! -f $CARLA_SETTINGS_FILE ]]; then
  echo "Copying CarlaSettings.ini..."
  sed -e 's/UseNetworking=true/UseNetworking=false/' ./Docs/Example.CarlaSettings.ini > $CARLA_SETTINGS_FILE
fi

./Util/Protoc.sh

# ==============================================================================
# -- Update CARLA Content ------------------------------------------------------
# ==============================================================================

echo
./Update.sh $UPDATE_SCRIPT_FLAGS

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

set +x
echo ""
echo "****************"
echo "*** Success! ***"
echo "****************"
