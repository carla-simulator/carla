#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

command -v /usr/bin/xcodebuild >/dev/null || {
  echo >&2 "Xcode not installed - need version 9.2 or later";
  exit 1;
} 

function getXcodeVersion() {
  /usr/bin/xcodebuild -version | sed -e "s/Xcode //; q"
}

XCODE_VERSION=$(getXcodeVersion)
if [[ ! "${XCODE_VERSION}" =~ ^[1-9][0-9] ]] && [[ "${XCODE_VERSION}" < '9.2' ]] ; then
  echo >&2 "Found Xcode ${XCODE_VERSION}, but require version 9.2 or later" ;
  echo >&2 "Install appropriate version and if necessary activate using xcode-select";
  exit 2;
fi

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

BUILD_TOOLS_DIR=$(dirname "$0")

source $(dirname "$0")/Environment.sh

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Setup build including third-party libraries"

USAGE_STRING="Usage: $0 [-h|--help] [--[no]-xcode]"

USE_XCODE=true
if [[ -f ${CARLA_BUILD_FOLDER}/NO_XCODEBUILD ]] ; then
  USE_XCODE=false
fi
# Mac OSX standard getopt does not support long options, so we don't bother. 

while true; do
  case "$1" in
    --xcode )
      USE_XCODE=true;
      rm -f ${CARLA_BUILD_FOLDER}/NO_XCODEBUILD
      shift ;;
    --no-xcode )
      USE_XCODE=false;
      touch ${CARLA_BUILD_FOLDER}/NO_XCODEBUILD
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      if [ ! -z "$1" ]; then
        echo "Bad argument: '$1'"
        echo "$USAGE_STRING"
        exit 2
      fi
      break ;;
  esac
done

# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION=1.69.0
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
  #rm ${BOOST_GZ}
  mkdir -p ${BOOST_BASENAME}-install/include
  mv ${BOOST_DISTNAME} ${BOOST_BASENAME}-source

  pushd ${BOOST_BASENAME}-source >/dev/null

  BOOST_TOOLSET="darwin"
  BOOST_CFLAGS="-fPIC -std=c++14 -DBOOST_ERROR_CODE_HEADER_ONLY"

  py2="/usr/bin/env python2"
  py2_root=`${py2} -c "import sys; print(sys.prefix)"`
  pyv=`$py2 -c "import sys;x='{v[0]}.{v[1]}'.format(v=list(sys.version_info[:2]));sys.stdout.write(x)";`
  ./bootstrap.sh \
      --with-toolset=clang \
      --prefix=../boost-install \
      --with-libraries=python,filesystem \
      --with-python=${py2} --with-python-root=${py2_root}

  if ${TRAVIS}
  then
    ${py2} ${BUILD_TOOLS_DIR}/gen-boost-python-config.py ${HOME}/user-config.jam
  else
    ${py2} ${BUILD_TOOLS_DIR}/gen-boost-python-config.py project-config.jam
  fi

  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} stage release
  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} install
  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} --clean-all

  # Get rid of  python2 build artifacts completely & do a clean build for python3
  popd >/dev/null
  rm -Rf ${BOOST_BASENAME}-source
  tar -xzf ${BOOST_BASENAME//[-.]/_}.tar.gz
  mkdir -p ${BOOST_BASENAME}-install/include
  mv ${BOOST_BASENAME//[-.]/_} ${BOOST_BASENAME}-source
  pushd ${BOOST_BASENAME}-source >/dev/null

  py3="/usr/bin/env python3"
  py3_root=`${py3} -c "import sys; print(sys.prefix)"`
  pyv=`$py3 -c "import sys;x='{v[0]}.{v[1]}'.format(v=list(sys.version_info[:2]));sys.stdout.write(x)";`
  ./bootstrap.sh \
      --with-toolset=clang \
      --prefix=../boost-install \
      --with-libraries=python \
      --with-python=${py3} --with-python-root=${py3_root}

  if ${TRAVIS}
  then
    ${py3} ${BUILD_TOOLS_DIR}/gen-boost-python-config.py ${HOME}/user-config.jam
  else
    ${py3} ${BUILD_TOOLS_DIR}/gen-boost-python-config.py project-config.jam
  fi

  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} stage release
  ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} install

  popd >/dev/null

  rm -Rf ${BOOST_BASENAME}-source
  rm ${BOOST_BASENAME//[-.]/_}.tar.gz

fi

unset BOOST_BASENAME

# ==============================================================================
# -- Get rpclib and compile it with libc++ and libstdc++ -----------------------
# ==============================================================================

RPCLIB_PATCH=v2.2.1_c1
RPCLIB_BASENAME=rpclib-${RPCLIB_PATCH}

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

  git clone -b ${RPCLIB_PATCH} https://github.com/carla-simulator/rpclib.git ${RPCLIB_BASENAME}-source

  log "Building rpclib with libc++."

  mkdir -p ${RPCLIB_BASENAME}-libcxx-build

  pushd ${RPCLIB_BASENAME}-libcxx-build >/dev/null

  if ${USE_XCODE}; then
    RPC_TOOLSET=Xcode
  else
    RPC_TOOLSET=Ninja
  fi

  cmake -G ${RPC_TOOLSET} \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -DBOOST_NO_EXCEPTIONS -DASIO_NO_EXCEPTIONS " \
      -DCMAKE_INSTALL_PREFIX="../${RPCLIB_BASENAME}-libcxx-install" \
      ../${RPCLIB_BASENAME}-source

  if ${USE_XCODE}; then
    xcodebuild
    xcodebuild -target install -configuration Release
  else
    ninja
    ninja install
  fi

  popd >/dev/null

  log "Building rpclib with libstdc++."

  mkdir -p ${RPCLIB_BASENAME}-libstdcxx-build

  pushd ${RPCLIB_BASENAME}-libstdcxx-build >/dev/null

  cmake -G ${RPC_TOOLSET} \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14" \
      -DCMAKE_INSTALL_PREFIX="../${RPCLIB_BASENAME}-libstdcxx-install" \
      ../${RPCLIB_BASENAME}-source

  if ${USE_XCODE}; then
    xcodebuild
    xcodebuild -target install -configuration Release
  else
    ninja
    ninja install
  fi

  popd >/dev/null

 rm -Rf ${RPCLIB_BASENAME}-source ${RPCLIB_BASENAME}-libcxx-build ${RPCLIB_BASENAME}-libstdcxx-build

fi

unset RPCLIB_BASENAME

# ==============================================================================
# -- Get GTest and compile it with libc++ --------------------------------------
# ==============================================================================

GTEST_BASENAME=googletest-1.8.0-ex

GTEST_LIBCXX_INCLUDE=${PWD}/${GTEST_BASENAME}-libcxx-install/include
GTEST_LIBCXX_LIBPATH=${PWD}/${GTEST_BASENAME}-libcxx-install/lib
GTEST_LIBSTDCXX_INCLUDE=${PWD}/${GTEST_BASENAME}-libstdcxx-install/include
GTEST_LIBSTDCXX_LIBPATH=${PWD}/${GTEST_BASENAME}-libstdcxx-install/lib

if [[ -d "${GTEST_BASENAME}-libcxx-install" && -d "${GTEST_BASENAME}-libstdcxx-install" ]] ; then
  log "${GTEST_BASENAME} already installed."
else
  rm -Rf \
      ${GTEST_BASENAME}-source \
      ${GTEST_BASENAME}-libcxx-build ${GTEST_BASENAME}-libstdcxx-build \
      ${GTEST_BASENAME}-libcxx-install ${GTEST_BASENAME}-libstdcxx-install

  log "Retrieving Google Test."

  git clone --depth=1 -b release-1.8.0 https://github.com/google/googletest.git ${GTEST_BASENAME}-source

  log "Building Google Test with libc++."

  mkdir -p ${GTEST_BASENAME}-libcxx-build

  pushd ${GTEST_BASENAME}-libcxx-build >/dev/null

  if ${USE_XCODE}; then
    GTEST_TOOLSET=Xcode
  else
    GTEST_TOOLSET=Ninja
  fi

  cmake -G ${GTEST_TOOLSET} \
      -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++ -DBOOST_NO_EXCEPTIONS -fno-exceptions " \
      -DCMAKE_INSTALL_PREFIX="../${GTEST_BASENAME}-libcxx-install" \
      ../${GTEST_BASENAME}-source

  if ${USE_XCODE}; then
    xcodebuild
    xcodebuild -target install -configuration Release
  else
    ninja
    ninja install
  fi

  popd >/dev/null

  log "Building Google Test with libstdc++."

  mkdir -p ${GTEST_BASENAME}-libstdcxx-build

  pushd ${GTEST_BASENAME}-libstdcxx-build >/dev/null

  cmake -G ${GTEST_TOOLSET} \
      -DCMAKE_CXX_FLAGS="-std=c++14" \
      -DCMAKE_INSTALL_PREFIX="../${GTEST_BASENAME}-libstdcxx-install" \
      ../${GTEST_BASENAME}-source

  if ${USE_XCODE}; then
    xcodebuild
    xcodebuild -target install -configuration Release
  else
    ninja
    ninja install
  fi

  popd >/dev/null

  rm -Rf ${GTEST_BASENAME}-source ${GTEST_BASENAME}-libcxx-build ${GTEST_BASENAME}-libstdcxx-build

fi

unset GTEST_BASENAME

# ==============================================================================
# -- Get Recast&Detour and compile it with libc++ ------------------------------
# ==============================================================================

RECAST_HASH=cdce4e
RECAST_COMMIT=cdce4e1a270fdf1f3942d4485954cc5e136df1df
RECAST_BASENAME=recast-${RECAST_HASH}-${CXX_TAG}

RECAST_INCLUDE=${PWD}/${RECAST_BASENAME}-install/include
RECAST_LIBPATH=${PWD}/${RECAST_BASENAME}-install/lib

if [[ -d "${RECAST_BASENAME}-install" ]] ; then
  log "${RECAST_BASENAME} already installed."
else
  rm -Rf \
      ${RECAST_BASENAME}-source \
      ${RECAST_BASENAME}-build \
      ${RECAST_BASENAME}-install

  log "Retrieving Recast & Detour"

  git clone https://github.com/carla-simulator/recastnavigation.git ${RECAST_BASENAME}-source

  pushd ${RECAST_BASENAME}-source >/dev/null

  git reset --hard ${RECAST_COMMIT}

  popd >/dev/null

  log "Building Recast & Detour with libc++."

  mkdir -p ${RECAST_BASENAME}-build

  pushd ${RECAST_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -fPIC" \
      -DCMAKE_INSTALL_PREFIX="../${RECAST_BASENAME}-install" \
      -DRECASTNAVIGATION_DEMO=False \
      -DRECASTNAVIGATION_TEST=False \
      ../${RECAST_BASENAME}-source

  ninja
  ninja install

  popd >/dev/null

  rm -Rf ${RECAST_BASENAME}-source ${RECAST_BASENAME}-build

  # move headers inside 'recast' folder
  mkdir -p "${PWD}/${RECAST_BASENAME}-install/include/recast"
  mv "${PWD}/${RECAST_BASENAME}-install/include/"*h "${PWD}/${RECAST_BASENAME}-install/include/recast/"

fi

unset RECAST_BASENAME

# ==============================================================================
# -- Generate Version.h --------------------------------------------------------
# ==============================================================================

CARLA_VERSION=$(get_git_repository_version)

log "CARLA version ${CARLA_VERSION}."

VERSION_H_FILE=${LIBCARLA_ROOT_FOLDER}/source/carla/Version.h
VERSION_H_FILE_GEN=${CARLA_BUILD_FOLDER}/Version.h

sed -e "s|\${CARLA_VERSION}|${CARLA_VERSION}|g" ${VERSION_H_FILE}.in > ${VERSION_H_FILE_GEN}

move_if_changed "${VERSION_H_FILE_GEN}" "${VERSION_H_FILE}"

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

# @todo These flags need to be compatible with setup.py compilation.
set(CMAKE_CXX_FLAGS_RELEASE_CLIENT "\${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fno-strict-aliasing -Wdate-time -D_FORTIFY_SOURCE=2 -g -fstack-protector-strong -Wformat -Werror=format-security -fPIC -std=c++14 -Wno-missing-braces -DBOOST_ERROR_CODE_HEADER_ONLY" CACHE STRING "" FORCE)
EOL

# -- LIBCPP_TOOLCHAIN_FILE -----------------------------------------------------

# We can reuse the previous toolchain.
cp ${LIBSTDCPP_TOOLCHAIN_FILE}.gen ${LIBCPP_TOOLCHAIN_FILE}.gen

cat >>${LIBCPP_TOOLCHAIN_FILE}.gen <<EOL

set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -fno-exceptions" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS} -lc++ -lc++abi" CACHE STRING "" FORCE)
EOL

# -- CMAKE_CONFIG_FILE ---------------------------------------------------------

cat >${CMAKE_CONFIG_FILE}.gen <<EOL
# Automatically generated by `basename "$0"`

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

if (CMAKE_BUILD_TYPE STREQUAL "Server")
  add_definitions(-DASIO_NO_EXCEPTIONS)
  add_definitions(-DBOOST_NO_EXCEPTIONS)
  add_definitions(-DLIBCARLA_NO_EXCEPTIONS)
  add_definitions(-DPUGIXML_NO_EXCEPTIONS)
endif ()

# Uncomment to force support for an specific image format (require their
# respective libraries installed).
# add_definitions(-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT)
# add_definitions(-DLIBCARLA_IMAGE_WITH_JPEG_SUPPORT)
# add_definitions(-DLIBCARLA_IMAGE_WITH_TIFF_SUPPORT)

add_definitions(-DLIBCARLA_TEST_CONTENT_FOLDER="${LIBCARLA_TEST_CONTENT_FOLDER}")

set(BOOST_INCLUDE_PATH "${BOOST_INCLUDE}")

if (CMAKE_BUILD_TYPE STREQUAL "Server")
  set(RPCLIB_INCLUDE_PATH "${RPCLIB_LIBCXX_INCLUDE}")
  set(RPCLIB_LIB_PATH "${RPCLIB_LIBCXX_LIBPATH}")
  set(GTEST_INCLUDE_PATH "${GTEST_LIBCXX_INCLUDE}")
  set(GTEST_LIB_PATH "${GTEST_LIBCXX_LIBPATH}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Client")
  # Here libraries linking libstdc++.
  set(RPCLIB_INCLUDE_PATH "${RPCLIB_LIBSTDCXX_INCLUDE}")
  set(RPCLIB_LIB_PATH "${RPCLIB_LIBSTDCXX_LIBPATH}")
  set(GTEST_INCLUDE_PATH "${GTEST_LIBSTDCXX_INCLUDE}")
  set(GTEST_LIB_PATH "${GTEST_LIBSTDCXX_LIBPATH}")
  set(BOOST_LIB_PATH "${BOOST_LIBPATH}")
  set(RECAST_INCLUDE_PATH "${RECAST_INCLUDE}")
  set(RECAST_LIB_PATH "${RECAST_LIBPATH}")
endif ()

EOL

if [ "${TRAVIS}" == "true" ] ; then
  log "Travis CI build detected: disabling PNG support."
  echo "add_definitions(-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=false)" >> ${CMAKE_CONFIG_FILE}.gen
else
  echo "add_definitions(-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT=true)" >> ${CMAKE_CONFIG_FILE}.gen
fi

# -- Move files ----------------------------------------------------------------

move_if_changed "${LIBSTDCPP_TOOLCHAIN_FILE}.gen" "${LIBSTDCPP_TOOLCHAIN_FILE}"
move_if_changed "${LIBCPP_TOOLCHAIN_FILE}.gen" "${LIBCPP_TOOLCHAIN_FILE}"
move_if_changed "${CMAKE_CONFIG_FILE}.gen" "${CMAKE_CONFIG_FILE}"

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
