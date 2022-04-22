#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Download and install the required libraries for carla."

USAGE_STRING="Usage: $0 [--python-version=VERSION]"

OPTS=`getopt -o h --long help,chrono,python-version: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

PY_VERSION_LIST=3
USE_CHRONO=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PY_VERSION_LIST="$2";
      shift 2 ;;
    --chrono )
      USE_CHRONO=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

CARLA_LLVM_VERSION_MAJOR=$(cut -d'.' -f1 <<<"$(clang -dumpversion)")

source $(dirname "$0")/Environment.sh

command -v /usr/bin/clang++-$CARLA_LLVM_VERSION_MAJOR >/dev/null 2>&1 || {
  echo >&2 "clang-$CARLA_LLVM_VERSION_MAJOR is required, but it's not installed.";
  exit 1;
}

CXX_TAG=c$CARLA_LLVM_VERSION_MAJOR
export CC=/usr/bin/clang-$CARLA_LLVM_VERSION_MAJOR
export CXX=/usr/bin/clang++-$CARLA_LLVM_VERSION_MAJOR

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

# ==============================================================================
# -- Get and compile libc++ ----------------------------------------------------
# ==============================================================================

LLVM_BASENAME=llvm-$CARLA_LLVM_VERSION_MAJOR.0

LLVM_INCLUDE=${PWD}/${LLVM_BASENAME}-install/include/c++/v1
LLVM_LIBPATH=${PWD}/${LLVM_BASENAME}-install/lib

if [[ -d "${LLVM_BASENAME}-install" ]] ; then
  log "${LLVM_BASENAME} already installed."
else
  rm -Rf ${LLVM_BASENAME}-source ${LLVM_BASENAME}-build

  log "Retrieving libc++."

  git clone --depth=1 -b release_${CARLA_LLVM_VERSION_MAJOR}0  https://github.com/llvm-mirror/llvm.git ${LLVM_BASENAME}-source
  git clone --depth=1 -b release_${CARLA_LLVM_VERSION_MAJOR}0  https://github.com/llvm-mirror/libcxx.git ${LLVM_BASENAME}-source/projects/libcxx
  git clone --depth=1 -b release_${CARLA_LLVM_VERSION_MAJOR}0  https://github.com/llvm-mirror/libcxxabi.git ${LLVM_BASENAME}-source/projects/libcxxabi

  log "Compiling libc++."

  mkdir -p ${LLVM_BASENAME}-build

  pushd ${LLVM_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
      -DLIBCXX_INSTALL_EXPERIMENTAL_LIBRARY=OFF \
      -DLLVM_ENABLE_EH=OFF \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="../${LLVM_BASENAME}-install" \
      ../${LLVM_BASENAME}-source

  ninja cxx

  ninja install-libcxx

  ninja install-libcxxabi

  popd >/dev/null

  rm -Rf ${LLVM_BASENAME}-source ${LLVM_BASENAME}-build

fi

unset LLVM_BASENAME

# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION=1.72.0
BOOST_BASENAME="boost-${BOOST_VERSION}-${CXX_TAG}"

BOOST_INCLUDE=${PWD}/${BOOST_BASENAME}-install/include
BOOST_LIBPATH=${PWD}/${BOOST_BASENAME}-install/lib

for PY_VERSION in ${PY_VERSION_LIST[@]} ; do

  SHOULD_BUILD_BOOST=true
  PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
  LIB_NAME=${PYTHON_VERSION:7:3}
  LIB_NAME=${LIB_NAME//.}
  if [[ -d "${BOOST_BASENAME}-install" ]] ; then
    if [ -f "${BOOST_BASENAME}-install/lib/libboost_python${LIB_NAME}.a" ] ; then
      SHOULD_BUILD_BOOST=false
      log "${BOOST_BASENAME} already installed."
    fi
  fi

  if { ${SHOULD_BUILD_BOOST} ; } ; then
    rm -Rf ${BOOST_BASENAME}-source

    BOOST_PACKAGE_BASENAME=boost_${BOOST_VERSION//./_}

    log "Retrieving boost."
    wget "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/${BOOST_PACKAGE_BASENAME}.tar.gz" || true
    # try to use the backup boost we have in Jenkins
    if [[ ! -f "${BOOST_PACKAGE_BASENAME}.tar.gz" ]] ; then
      log "Using boost backup"
      wget "https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/${BOOST_PACKAGE_BASENAME}.tar.gz" || true
    fi

    log "Extracting boost for Python ${PY_VERSION}."
    tar -xzf ${BOOST_PACKAGE_BASENAME}.tar.gz
    mkdir -p ${BOOST_BASENAME}-install/include
    mv ${BOOST_PACKAGE_BASENAME} ${BOOST_BASENAME}-source
    # Boost patch for exception handling
    cp "${CARLA_BUILD_FOLDER}/../Util/BoostFiles/rational.hpp" "${BOOST_BASENAME}-source/boost/rational.hpp"
    cp "${CARLA_BUILD_FOLDER}/../Util/BoostFiles/read.hpp" "${BOOST_BASENAME}-source/boost/geometry/io/wkt/read.hpp"
    # ---

    pushd ${BOOST_BASENAME}-source >/dev/null

    BOOST_TOOLSET="clang-$CARLA_LLVM_VERSION_MAJOR.0"
    BOOST_CFLAGS="-fPIC -std=c++14 -DBOOST_ERROR_CODE_HEADER_ONLY"

    py3="/usr/bin/env python${PY_VERSION}"
    py3_root=`${py3} -c "import sys; print(sys.prefix)"`
    pyv=`$py3 -c "import sys;x='{v[0]}.{v[1]}'.format(v=list(sys.version_info[:2]));sys.stdout.write(x)";`
    ./bootstrap.sh \
        --with-toolset=clang \
        --prefix=../boost-install \
        --with-libraries=python,filesystem,system,program_options \
        --with-python=${py3} --with-python-root=${py3_root}

    if ${TRAVIS} ; then
      echo "using python : ${pyv} : ${py3_root}/bin/python${PY_VERSION} ;" > ${HOME}/user-config.jam
    else
      echo "using python : ${pyv} : ${py3_root}/bin/python${PY_VERSION} ;" > project-config.jam
    fi

    ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} stage release
    ./b2 toolset="${BOOST_TOOLSET}" cxxflags="${BOOST_CFLAGS}" --prefix="../${BOOST_BASENAME}-install" -j ${CARLA_BUILD_CONCURRENCY} install

    popd >/dev/null

    rm -Rf ${BOOST_BASENAME}-source
    rm ${BOOST_PACKAGE_BASENAME}.tar.gz

    # Boost patch for exception handling
    cp "${CARLA_BUILD_FOLDER}/../Util/BoostFiles/rational.hpp" "${BOOST_BASENAME}-install/include/boost/rational.hpp"
    cp "${CARLA_BUILD_FOLDER}/../Util/BoostFiles/read.hpp" "${BOOST_BASENAME}-install/include/boost/geometry/io/wkt/read.hpp"
    # ---

    # Install boost dependencies
    mkdir -p "${LIBCARLA_INSTALL_CLIENT_FOLDER}/include/system"
    mkdir -p "${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib"
    cp -rf ${BOOST_BASENAME}-install/include/* ${LIBCARLA_INSTALL_CLIENT_FOLDER}/include/system
    cp -rf ${BOOST_BASENAME}-install/lib/* ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib

  fi

done

unset BOOST_BASENAME

# ==============================================================================
# -- Get rpclib and compile it with libc++ and libstdc++ -----------------------
# ==============================================================================

RPCLIB_PATCH=v2.2.1_c5
RPCLIB_BASENAME=rpclib-${RPCLIB_PATCH}-${CXX_TAG}

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

  # rpclib does not use any cmake 3.9 feature.
  # As cmake 3.9 is not standard in Ubuntu 16.04, change cmake version to 3.5
  sed -i s/"3.9.0"/"3.5.0"/g ${RPCLIB_BASENAME}-source/CMakeLists.txt

  mkdir -p ${RPCLIB_BASENAME}-libcxx-build

  pushd ${RPCLIB_BASENAME}-libcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -Wl,-L${LLVM_LIBPATH} -DBOOST_NO_EXCEPTIONS -DASIO_NO_EXCEPTIONS" \
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

GTEST_VERSION=1.8.1
GTEST_BASENAME=gtest-${GTEST_VERSION}-${CXX_TAG}

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

  git clone --depth=1 -b release-${GTEST_VERSION} https://github.com/google/googletest.git ${GTEST_BASENAME}-source

  log "Building Google Test with libc++."

  mkdir -p ${GTEST_BASENAME}-libcxx-build

  pushd ${GTEST_BASENAME}-libcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -Wl,-L${LLVM_LIBPATH} -DBOOST_NO_EXCEPTIONS -fno-exceptions" \
      -DCMAKE_INSTALL_PREFIX="../${GTEST_BASENAME}-libcxx-install" \
      ../${GTEST_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  log "Building Google Test with libstdc++."

  mkdir -p ${GTEST_BASENAME}-libstdcxx-build

  pushd ${GTEST_BASENAME}-libstdcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14" \
      -DCMAKE_INSTALL_PREFIX="../${GTEST_BASENAME}-libstdcxx-install" \
      ../${GTEST_BASENAME}-source

  ninja

  ninja install

  popd >/dev/null

  rm -Rf ${GTEST_BASENAME}-source ${GTEST_BASENAME}-libcxx-build ${GTEST_BASENAME}-libstdcxx-build

fi

unset GTEST_BASENAME

# ==============================================================================
# -- Get Recast&Detour and compile it with libc++ ------------------------------
# ==============================================================================

RECAST_HASH=0b13b0
RECAST_COMMIT=0b13b0d288ac96fdc5347ee38299511c6e9400db
RECAST_BASENAME=recast-${RECAST_HASH}-${CXX_TAG}

RECAST_INCLUDE=${PWD}/${RECAST_BASENAME}-install/include
RECAST_LIBPATH=${PWD}/${RECAST_BASENAME}-install/lib

if [[ -d "${RECAST_BASENAME}-install" &&
      -f "${RECAST_BASENAME}-install/bin/RecastBuilder" ]] ; then
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

# make sure the RecastBuilder is corrctly copied
RECAST_INSTALL_DIR="${CARLA_BUILD_FOLDER}/../Util/DockerUtils/dist"
if [[ ! -f "${RECAST_INSTALL_DIR}/RecastBuilder" ]]; then
  cp "${RECAST_BASENAME}-install/bin/RecastBuilder" "${RECAST_INSTALL_DIR}/"
fi

unset RECAST_BASENAME

# ==============================================================================
# -- Get and compile libpng 1.6.37 ------------------------------
# ==============================================================================

LIBPNG_VERSION=1.6.37
LIBPNG_REPO=https://sourceforge.net/projects/libpng/files/libpng16/${LIBPNG_VERSION}/libpng-${LIBPNG_VERSION}.tar.xz
LIBPNG_BASENAME=libpng-${LIBPNG_VERSION}
LIBPNG_INSTALL=${LIBPNG_BASENAME}-install

LIBPNG_INCLUDE=${PWD}/${LIBPNG_BASENAME}-install/include/
LIBPNG_LIBPATH=${PWD}/${LIBPNG_BASENAME}-install/lib

if [[ -d ${LIBPNG_INSTALL} ]] ; then
  log "Libpng already installed."
else
  log "Retrieving libpng."
  wget ${LIBPNG_REPO}

  log "Extracting libpng."
  tar -xf libpng-${LIBPNG_VERSION}.tar.xz
  mv ${LIBPNG_BASENAME} ${LIBPNG_BASENAME}-source

  pushd ${LIBPNG_BASENAME}-source >/dev/null

  ./configure --prefix=${CARLA_BUILD_FOLDER}/${LIBPNG_INSTALL}
  make install

  popd >/dev/null

  rm -Rf libpng-${LIBPNG_VERSION}.tar.xz
  rm -Rf ${LIBPNG_BASENAME}-source
fi

# ==============================================================================
# -- Get and compile libxerces 3.2.3 ------------------------------
# ==============================================================================

XERCESC_VERSION=3.2.3
XERCESC_BASENAME=xerces-c-${XERCESC_VERSION}

XERCESC_TEMP_FOLDER=${XERCESC_BASENAME}
XERCESC_REPO=https://downloads.apache.org/xerces/c/3/sources/xerces-c-${XERCESC_VERSION}.tar.gz

XERCESC_SRC_DIR=${XERCESC_BASENAME}-source
XERCESC_INSTALL_DIR=${XERCESC_BASENAME}-install
XERCESC_LIB=${XERCESC_INSTALL_DIR}/lib/libxerces-c.a

if [[ -d ${XERCESC_INSTALL_DIR} ]] ; then
  log "Xerces-c already installed."
else
  log "Retrieving xerces-c."
  wget ${XERCESC_REPO}

  log "Extracting xerces-c."
  tar -xzf ${XERCESC_BASENAME}.tar.gz
  mv ${XERCESC_BASENAME} ${XERCESC_SRC_DIR}
  mkdir -p ${XERCESC_INSTALL_DIR}
  mkdir -p ${XERCESC_SRC_DIR}/build

  pushd ${XERCESC_SRC_DIR}/build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -fPIC -w" \
      -DCMAKE_INSTALL_PREFIX="../../${XERCESC_INSTALL_DIR}" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=OFF \
      -Dtranscoder=gnuiconv \
      -Dnetwork=OFF \
      ..
  ninja
  ninja install

  popd >/dev/null

  rm -Rf ${XERCESC_BASENAME}.tar.gz
  rm -Rf ${XERCESC_SRC_DIR}
fi

mkdir -p ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/
cp ${XERCESC_LIB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/

if ${USE_CHRONO} ; then

  # ==============================================================================
  # -- Get Eigen headers (Chrono dependency) -------------------------------------
  # ==============================================================================

  EIGEN_VERSION=3.3.7
  EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.gz
  EIGEN_BASENAME=eigen-${EIGEN_VERSION}

  EIGEN_SRC_DIR=eigen-${EIGEN_VERSION}-src
  EIGEN_INSTALL_DIR=eigen-${EIGEN_VERSION}-install
  EIGEN_INCLUDE=${EIGEN_INSTALL_DIR}/include


  if [[ -d ${EIGEN_INSTALL_DIR} ]] ; then
    log "Eigen already installed."
  else
    log "Retrieving Eigen."
    wget ${EIGEN_REPO}

    log "Extracting Eigen."
    tar -xzf ${EIGEN_BASENAME}.tar.gz
    mv ${EIGEN_BASENAME} ${EIGEN_SRC_DIR}
    mkdir -p ${EIGEN_INCLUDE}/unsupported
    mv ${EIGEN_SRC_DIR}/Eigen ${EIGEN_INCLUDE}
    mv ${EIGEN_SRC_DIR}/unsupported/Eigen ${EIGEN_INCLUDE}/unsupported/Eigen

    rm -Rf ${EIGEN_BASENAME}.tar.gz
    rm -Rf ${EIGEN_SRC_DIR}
  fi

  mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/
  cp -p -r ${EIGEN_INCLUDE}/* ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/

  # ==============================================================================
  # -- Get Chrono and compile it with libc++ -------------------------------------
  # ==============================================================================

  CHRONO_TAG=6.0.0
  # CHRONO_TAG=develop
  CHRONO_REPO=https://github.com/projectchrono/chrono.git

  CHRONO_SRC_DIR=chrono-source
  CHRONO_INSTALL_DIR=chrono-install

  if [[ -d ${CHRONO_INSTALL_DIR} ]] ; then
    log "chrono library already installed."
  else
    log "Retrieving chrono library."
    git clone --depth 1 --branch ${CHRONO_TAG} ${CHRONO_REPO} ${CHRONO_SRC_DIR}

    mkdir -p ${CHRONO_SRC_DIR}/build

    pushd ${CHRONO_SRC_DIR}/build >/dev/null

    cmake -G "Ninja" \
        -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH} -Wno-unused-command-line-argument" \
        -DEIGEN3_INCLUDE_DIR="../../${EIGEN_INCLUDE}" \
        -DCMAKE_INSTALL_PREFIX="../../${CHRONO_INSTALL_DIR}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_MODULE_VEHICLE=ON \
        ..
    ninja
    ninja install

    popd >/dev/null

    rm -Rf ${CHRONO_SRC_DIR}
  fi

  mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/
  cp -p ${CHRONO_INSTALL_DIR}/lib/*.so ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  cp -p -r ${CHRONO_INSTALL_DIR}/include/* ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/

fi

# ==============================================================================
# -- Get and compile Sqlite3 ---------------------------------------------------
# ==============================================================================

SQLITE_VERSION=sqlite-autoconf-3340100
SQLITE_REPO=https://www.sqlite.org/2021/${SQLITE_VERSION}.tar.gz

SQLITE_TAR=${SQLITE_VERSION}.tar.gz
SQLITE_SOURCE_DIR=sqlite-src
SQLITE_INSTALL_DIR=sqlite-install

SQLITE_INCLUDE_DIR=${PWD}/${SQLITE_INSTALL_DIR}/include
SQLITE_LIB=${PWD}/${SQLITE_INSTALL_DIR}/lib/libsqlite3.a
SQLITE_EXE=${PWD}/${SQLITE_INSTALL_DIR}/bin/sqlite3

if [[ -d ${SQLITE_INSTALL_DIR} ]] ; then
  log "Sqlite already installed."
else
  log "Retrieving Sqlite3"
  wget ${SQLITE_REPO}

  log "Extracting Sqlite3"
  tar -xzf ${SQLITE_TAR}
  mv ${SQLITE_VERSION} ${SQLITE_SOURCE_DIR}

  mkdir ${SQLITE_INSTALL_DIR}

  pushd ${SQLITE_SOURCE_DIR} >/dev/null

  export CFLAGS="-fPIC"
  ./configure --prefix=${PWD}/../sqlite-install/
  make
  make install

  popd >/dev/null

  rm -Rf ${SQLITE_TAR}
  rm -Rf ${SQLITE_SOURCE_DIR}
fi

mkdir -p ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/
cp ${SQLITE_LIB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/

# ==============================================================================
# -- Get and compile PROJ ------------------------------------------------------
# ==============================================================================

PROJ_VERSION=proj-7.2.1
PROJ_REPO=https://download.osgeo.org/proj/${PROJ_VERSION}.tar.gz

PROJ_TAR=${PROJ_VERSION}.tar.gz
PROJ_SRC_DIR=proj-src
PROJ_INSTALL_DIR=proj-install
PROJ_INSTALL_DIR_FULL=${PWD}/${PROJ_INSTALL_DIR}
PROJ_LIB=${PROJ_INSTALL_DIR_FULL}/lib/libproj.a

if [[ -d ${PROJ_INSTALL_DIR} ]] ; then
  log "PROJ already installed."
else
  log "Retrieving PROJ"
  wget ${PROJ_REPO}

  log "Extracting PROJ"
  tar -xzf ${PROJ_TAR}
  mv ${PROJ_VERSION} ${PROJ_SRC_DIR}

  mkdir ${PROJ_SRC_DIR}/build
  mkdir ${PROJ_INSTALL_DIR}

  pushd ${PROJ_SRC_DIR}/build >/dev/null

  cmake -G "Ninja" .. \
      -DCMAKE_CXX_FLAGS="-std=c++14 -fPIC" \
      -DSQLITE3_INCLUDE_DIR=${SQLITE_INCLUDE_DIR} -DSQLITE3_LIBRARY=${SQLITE_LIB} \
      -DEXE_SQLITE3=${SQLITE_EXE} \
      -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_PROJSYNC=OFF \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_PROJINFO=OFF \
      -DBUILD_CCT=OFF -DBUILD_CS2CS=OFF -DBUILD_GEOD=OFF -DBUILD_GIE=OFF \
      -DBUILD_PROJ=OFF -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX=${PROJ_INSTALL_DIR_FULL}
  ninja
  ninja install

  popd >/dev/null

  rm -Rf ${PROJ_TAR}
  rm -Rf ${PROJ_SRC_DIR}

fi

cp ${PROJ_LIB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/

# ==============================================================================
# -- Get and compile patchelf --------------------------------------------------
# ==============================================================================

PATCHELF_VERSION=0.12
PATCHELF_REPO=https://github.com/NixOS/patchelf/archive/${PATCHELF_VERSION}.tar.gz

PATCHELF_TAR=${PATCHELF_VERSION}.tar.gz
PATCHELF_SOURCE_DIR=patchelf-src
PATCHELF_INSTALL_DIR=patchelf-install

PATCHELF_INCLUDE_DIR=${PWD}/${PATCHELF_INSTALL_DIR}/include
PATCHELF_EXE=${PWD}/${PATCHELF_INSTALL_DIR}/bin/patchelf

if [[ -d ${PATCHELF_INSTALL_DIR} ]] ; then
  log "Patchelf already installed."
else
  log "Retrieving patchelf"
  wget ${PATCHELF_REPO}

  log "Extracting patchelf"
  tar -xzf ${PATCHELF_TAR}
  mv patchelf-${PATCHELF_VERSION} ${PATCHELF_SOURCE_DIR}

  mkdir ${PATCHELF_INSTALL_DIR}

  pushd ${PATCHELF_SOURCE_DIR} >/dev/null

  ./bootstrap.sh
  ./configure --prefix=${PWD}/../${PATCHELF_INSTALL_DIR}
  make
  make install

  popd >/dev/null

  rm -Rf ${PATCHELF_TAR}
  rm -Rf ${PATCHELF_SOURCE_DIR}
fi

mkdir -p ${LIBCARLA_INSTALL_CLIENT_FOLDER}/bin/
cp ${PATCHELF_EXE} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/bin/

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

# disable -Werror since the boost 1.72 doesn't compile with ad_rss without warnings (i.e. the geometry headers)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -std=c++14 -pthread -fPIC" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wdeprecated -Wshadow -Wuninitialized -Wunreachable-code" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wpessimizing-move -Wold-style-cast -Wnull-dereference" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wduplicate-enum -Wnon-virtual-dtor -Wheader-hygiene" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -Wconversion -Wfloat-overflow-conversion" CACHE STRING "" FORCE)

# @todo These flags need to be compatible with setup.py compilation.
set(CMAKE_CXX_FLAGS_RELEASE_CLIENT "\${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fno-strict-aliasing -Wdate-time -D_FORTIFY_SOURCE=2 -g -fstack-protector-strong -Wformat -Werror=format-security -fPIC -std=c++14 -Wno-missing-braces -DBOOST_ERROR_CODE_HEADER_ONLY" CACHE STRING "" FORCE)
EOL

# -- LIBCPP_TOOLCHAIN_FILE -----------------------------------------------------

# We can reuse the previous toolchain.
cp ${LIBSTDCPP_TOOLCHAIN_FILE}.gen ${LIBCPP_TOOLCHAIN_FILE}.gen

cat >>${LIBCPP_TOOLCHAIN_FILE}.gen <<EOL

set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -stdlib=libc++" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -isystem ${LLVM_INCLUDE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -fno-exceptions" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS} -L${LLVM_LIBPATH}" CACHE STRING "" FORCE)
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
  # Here libraries linking libc++.
  set(LLVM_INCLUDE_PATH "${LLVM_INCLUDE}")
  set(LLVM_LIB_PATH "${LLVM_LIBPATH}")
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
  set(LIBPNG_INCLUDE_PATH "${LIBPNG_INCLUDE}")
  set(LIBPNG_LIB_PATH "${LIBPNG_LIBPATH}")
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
