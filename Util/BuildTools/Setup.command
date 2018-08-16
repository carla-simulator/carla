#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

command -v /usr/bin/xcodebuild >/dev/null || {
  echo >&2 "Xcode not installed - need version 9.2";
  exit 1;
} 

function getXcodeVersion() {
  /usr/bin/xcodebuild -version | grep Xcode
}

XCODE_VERSION=$(getXcodeVersion)
if [[ ${XCODE_VERSION} != 'Xcode 9.2' ]] ; then
  # TODO - allow later versions
  echo >&2 "Found ${XCODE_VERSION}, but require 9.2" ;
  echo >&2 "Install Xcode 9.2 and if necessary activate using xcode-select";
  exit 2;
fi

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

source $(dirname "$0")/Environment.sh

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION=1.67.0
BOOST_UVERSION=${BOOST_VERSION//\./_}
BOOST_BASENAME=boost-${BOOST_VERSION}

BOOST_INCLUDE=${PWD}/${BOOST_BASENAME}-install/include
BOOST_LIBPATH=${PWD}/${BOOST_BASENAME}-install/lib

if [[ -d "${BOOST_BASENAME}-install" ]] ; then
  log "${BOOST_BASENAME} already installed."
else

  rm -Rf ${BOOST_BASENAME}-source

  BOOST_DISTNAME="boost_${BOOST_UVERSION}"
  BOOST_GZ="${BOOST_DISTNAME}.tar.gz"

  BOOST_BINTRAY_URL="https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/${BOOST_GZ}"
  BOOST_SOURCEFORGE_URL="https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_GZ}/download"
  BOOST_DOWNLOAD_URL=${BOOST_BINTRAY_URL}

  rm -Rf ${BOOST_BASENAME}-source

  if [[ ! -f ${BOOST_GZ} ]]; then
    log "Retrieving boost."
    if [[ -f ~/Downloads/${BOOST_GZ} ]]; then
      echo "Copying from ~/Downloads"
      cp ~/Downloads/${BOOST_GZ} .
    else
      wget ${BOOST_DOWNLOAD_URL}
      cp ${BOOST_GZ} ~/Downloads/
    fi
  fi

  log "Extracting boost."
  tar -xzf ${BOOST_GZ}
  rm ${BOOST_GZ}
  mkdir -p ${BOOST_BASENAME}-install/include
  mv ${BOOST_DISTNAME} ${BOOST_BASENAME}-source

  pushd ${BOOST_BASENAME}-source >/dev/null

  BOOST_TOOLSET="darwin"
  BOOST_CFLAGS="-fPIC -std=c++14 -DBOOST_ERROR_CODE_HEADER_ONLY"

  ./bootstrap.sh \
      --with-toolset=clang \
      --prefix=../boost-install \
      --with-libraries=python
  ./b2 clean
  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j 12 stage release
  ./b2 install toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j 12

  popd >/dev/null

  # rm -Rf ${BOOST_BASENAME}-source

fi

unset BOOST_BASENAME

# ==============================================================================
# -- Get rpclib and compile it with libc++ and libstdc++ -----------------------
# ==============================================================================

RPCLIB_BASENAME=rpclib-2.2.1

RPCLIB_LIBCXX_INCLUDE=${PWD}/${RPCLIB_BASENAME}-libcxx-install/include
RPCLIB_LIBCXX_LIBPATH=${PWD}/${RPCLIB_BASENAME}-libcxx-install/lib
RPCLIB_LIBSTDCXX_INCLUDE=${PWD}/${RPCLIB_BASENAME}-libstdcxx-install/include
RPCLIB_LIBSTDCXX_LIBPATH=${PWD}/${RPCLIB_BASENAME}-libstdcxx-install/lib

if [[ -d "${RPCLIB_BASENAME}-libcxx-install" && -d "${RPCLIB_BASENAME}-libstdcxx-install" ]] ; then
  log "${RPCLIB_BASENAME} already installed."
else
  rm -Rf \
      ${RPCLIB_BASENAME}-source \
      ${RPCLIB_BASENAME}-libcxx-build ${RPCLIB_BASENAME}-libstdcxx-build \
      ${RPCLIB_BASENAME}-libcxx-install ${RPCLIB_BASENAME}-libstdcxx-install

  log "Retrieving rpclib."

  git clone --depth=1 -b v2.2.1 https://github.com/rpclib/rpclib.git ${RPCLIB_BASENAME}-source

  log "Building rpclib with libc++."

  mkdir -p ${RPCLIB_BASENAME}-libcxx-build

  pushd ${RPCLIB_BASENAME}-libcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++" \
      -DCMAKE_INSTALL_PREFIX="../${RPCLIB_BASENAME}-libcxx-install" \
      ../${RPCLIB_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  log "Building rpclib with libstdc++."

  mkdir -p ${RPCLIB_BASENAME}-libstdcxx-build

  pushd ${RPCLIB_BASENAME}-libstdcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14" \
      -DCMAKE_INSTALL_PREFIX="../${RPCLIB_BASENAME}-libstdcxx-install" \
      ../${RPCLIB_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  rm -Rf ${RPCLIB_BASENAME}-source ${RPCLIB_BASENAME}-libcxx-build ${RPCLIB_BASENAME}-libstdcxx-build

fi

unset RPCLIB_BASENAME

# ==============================================================================
# -- Get GTest and compile it with libc++ --------------------------------------
# ==============================================================================

GTEST_BASENAME=googletest-1.8.0

GTEST_INCLUDE=${PWD}/${GTEST_BASENAME}-install/include
GTEST_LIBPATH=${PWD}/${GTEST_BASENAME}-install/lib

if [[ -d "${GTEST_BASENAME}-install" ]] ; then
  log "${GTEST_BASENAME} already installed."
else
  rm -Rf ${GTEST_BASENAME}-source ${GTEST_BASENAME}-build

  log "Retrieving Google Test."

  git clone --depth=1 -b release-1.8.0 https://github.com/google/googletest.git ${GTEST_BASENAME}-source

  log "Building Google Test."

  mkdir -p ${GTEST_BASENAME}-build

  pushd ${GTEST_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++ " \
      -DCMAKE_INSTALL_PREFIX="../${GTEST_BASENAME}-install" \
      ../${GTEST_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  rm -Rf ${GTEST_BASENAME}-source ${GTEST_BASENAME}-build

fi

# ==============================================================================
# -- Generate CMake toolchains and config --------------------------------------
# ==============================================================================

log "Generating CMake configuration files."

# -- LIBSTDCPP_TOOLCHAIN_FILE --------------------------------------------------

cat >${LIBSTDCPP_TOOLCHAIN_FILE}.gen <<EOL
# Automatically generated by `basename "$0"`

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -std=c++14 -fPIC" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra" CACHE STRING "" FORCE)
# See https://bugs.llvm.org/show_bug.cgi?id=21629
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wno-missing-braces" CACHE STRING "" FORCE)
EOL

# -- LIBCPP_TOOLCHAIN_FILE -----------------------------------------------------

# We can reuse the previous toolchain.
cp ${LIBSTDCPP_TOOLCHAIN_FILE}.gen ${LIBCPP_TOOLCHAIN_FILE}.gen

cat >>${LIBCPP_TOOLCHAIN_FILE}.gen <<EOL

set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS} -lc++ -lc++abi" CACHE STRING "" FORCE)
EOL

# -- CMAKE_CONFIG_FILE ---------------------------------------------------------

cat >${CMAKE_CONFIG_FILE}.gen <<EOL
# Automatically generated by `basename "$0"`

set(CARLA_VERSION $(get_carla_version))

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

set(BOOST_INCLUDE_PATH "${BOOST_INCLUDE}")

if (CMAKE_BUILD_TYPE STREQUAL "Server")
  set(GTEST_INCLUDE_PATH "${GTEST_INCLUDE}")
  set(GTEST_LIB_PATH "${GTEST_LIBPATH}")
  set(RPCLIB_INCLUDE_PATH "${RPCLIB_LIBCXX_INCLUDE}")
  set(RPCLIB_LIB_PATH "${RPCLIB_LIBCXX_LIBPATH}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Client")
  # Here libraries linking libstdc++.
  set(RPCLIB_INCLUDE_PATH "${RPCLIB_LIBSTDCXX_INCLUDE}")
  set(RPCLIB_LIB_PATH "${RPCLIB_LIBSTDCXX_LIBPATH}")
  set(BOOST_LIB_PATH "${BOOST_LIBPATH}")
endif ()
EOL

# -- Move files ----------------------------------------------------------------

move_if_changed "${LIBSTDCPP_TOOLCHAIN_FILE}.gen" "${LIBSTDCPP_TOOLCHAIN_FILE}"
move_if_changed "${LIBCPP_TOOLCHAIN_FILE}.gen" "${LIBCPP_TOOLCHAIN_FILE}"
move_if_changed "${CMAKE_CONFIG_FILE}.gen" "${CMAKE_CONFIG_FILE}"

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
