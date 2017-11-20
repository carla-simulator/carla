#! /bin/bash

################################################################################
# CARLA Util Setup
#
# This downloads and compiles libc++. So we can build and compile our
# dependencies with libc++ for linking against Unreal.
#
# Thanks to the people at https://github.com/Microsoft/AirSim for providing the
# important parts of this script.
################################################################################

set -e

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
# -- Download the content ------------------------------------------------------
# ==============================================================================

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
  ../download_from_gdrive.py $CONTENT_GDRIVE_ID Content.tar.gz
  tar -xvzf Content.tar.gz -C Content
  rm Content.tar.gz
  mv Content/* $CONTENT_FOLDER
  echo "$CONTENT_GDRIVE_ID" > "$VERSION_FILE"
}

if [[ -d "$CONTENT_FOLDER/.git" ]]; then
  echo "Using git version of 'Content', skipping download..."
elif [[ -f "$CONTENT_FOLDER/.version" ]]; then
  if [ "$CONTENT_GDRIVE_ID" == `cat $VERSION_FILE` ]; then
    echo "Content is up-to-date, skipping download..."
  else
    download_content
  fi
else
  download_content
fi

exit 1

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

make cxx

#install libc++ locally in llvm-install folder
make install-libcxx install-libcxxabi

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

./bootstrap.sh --with-toolset=clang --prefix=../boost-install
./b2 clean
./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" linkflags="${BOOST_LFLAGS}" --prefix="../boost-install" -j 4 stage release
./b2 install toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" linkflags="${BOOST_LFLAGS}" --prefix="../boost-install"

popd >/dev/null

# ==============================================================================
# -- Get Protobuf and compile it with libc++ -----------------------------------
# ==============================================================================

# Get protobuf source
if [[ ! -d "protobuf-source" ]]; then
  echo "Retrieving protobuf..."
  git clone --depth=1 -b v3.3.0  https://github.com/google/protobuf.git protobuf-source
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
    --prefix="$PWD/../protobuf-install"
make
make install

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
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

set +x
echo ""
echo "****************"
echo "*** Success! ***"
echo "****************"
