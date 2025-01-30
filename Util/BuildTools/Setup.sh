#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Download and install the required libraries for carla."

USAGE_STRING="Usage: $0 [--python-version=VERSION]"

OPTS=`getopt -o h --long help,chrono,ros2,pytorch,python-version: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

PY_VERSION_LIST=3
USE_CHRONO=false
USE_PYTORCH=false
USE_ROS2=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PY_VERSION_LIST="$2";
      shift 2 ;;
    --chrono )
      USE_CHRONO=true;
      shift ;;
    --pytorch )
      USE_PYTORCH=true;
      shift ;;
    --ros2 )
      USE_ROS2=true;
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

source $(dirname "$0")/Environment.sh

export CC="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang"
export CXX="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++"
export PATH="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin:$PATH"

CXX_TAG=c10

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

LLVM_INCLUDE="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1"
LLVM_LIBPATH="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu"
UNREAL_HOSTED_CFLAGS="--sysroot=$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/"

# ==============================================================================
# -- Get boost includes --------------------------------------------------------
# ==============================================================================

BOOST_VERSION=1.80.0
BOOST_BASENAME="boost-${BOOST_VERSION}-${CXX_TAG}"
BOOST_SHA256SUM="4b2136f98bdd1f5857f1c3dea9ac2018effe65286cf251534b6ae20cc45e1847"

BOOST_INCLUDE=${PWD}/${BOOST_BASENAME}-install/include
BOOST_LIBPATH=${PWD}/${BOOST_BASENAME}-install/lib

for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
  SHOULD_BUILD_BOOST=true
  PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
  LIB_NAME=$(cut -d . -f 1,2 <<< "$PYTHON_VERSION" | tr -d .)
  LIB_NAME=${LIB_NAME:7}
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

    start=$(date +%s)
    wget "https://archives.boost.io/release/${BOOST_VERSION}/source/${BOOST_PACKAGE_BASENAME}.tar.gz" -O ${BOOST_PACKAGE_BASENAME}.tar.gz || true
    end=$(date +%s)
    echo "Elapsed Time downloading from boost webpage: $(($end-$start)) seconds"

    # try to use the backup boost we have in Jenkins
    if [ ! -f "${BOOST_PACKAGE_BASENAME}.tar.gz" ] || [[ $(sha256sum "${BOOST_PACKAGE_BASENAME}.tar.gz" | cut -d " " -f 1 ) != "${BOOST_SHA256SUM}" ]] ; then
      log "Using boost backup"

      start=$(date +%s)
      wget "https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/${BOOST_PACKAGE_BASENAME}.tar.gz" -O ${BOOST_PACKAGE_BASENAME}.tar.gz || true
      end=$(date +%s)
      echo "Elapsed Time downloading from boost carla backup in backblaze: $(($end-$start)) seconds"

    fi

    log "Extracting boost for Python ${PY_VERSION}."

    start=$(date +%s)
    tar -xzf ${BOOST_PACKAGE_BASENAME}.tar.gz
    end=$(date +%s)
    echo "Elapsed Time Extracting boost for Python: $(($end-$start)) seconds"

    mkdir -p ${BOOST_BASENAME}-install/include
    mv ${BOOST_PACKAGE_BASENAME} ${BOOST_BASENAME}-source

    pushd ${BOOST_BASENAME}-source >/dev/null

    BOOST_TOOLSET="clang-10.0"
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

  start_download_time=$(date +%s)

  git clone -b ${RPCLIB_PATCH} https://github.com/carla-simulator/rpclib.git ${RPCLIB_BASENAME}-source

  end_download_time=$(date +%s)

  echo "Elapsed Time downloading rpclib: $(($end_download_time-$start_download_time)) seconds"
  log "Building rpclib with libc++."

  # rpclib does not use any cmake 3.9 feature.
  # As cmake 3.9 is not standard in Ubuntu 16.04, change cmake version to 3.5
  sed -i s/"3.9.0"/"3.5.0"/g ${RPCLIB_BASENAME}-source/CMakeLists.txt

  mkdir -p ${RPCLIB_BASENAME}-libcxx-build

  pushd ${RPCLIB_BASENAME}-libcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -Wl,-L${LLVM_LIBPATH} -DBOOST_NO_EXCEPTIONS -DASIO_NO_EXCEPTIONS ${UNREAL_HOSTED_CFLAGS}" \
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

  start_download_time=$(date +%s)

  git clone --depth=1 -b release-${GTEST_VERSION} https://github.com/google/googletest.git ${GTEST_BASENAME}-source

  end_download_time=$(date +%s)
  echo "Elapsed Time downloading rpclib: $(($end_download_time-$start_download_time)) seconds"

  log "Building Google Test with libc++."

  mkdir -p ${GTEST_BASENAME}-libcxx-build

  pushd ${GTEST_BASENAME}-libcxx-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -Wl,-L${LLVM_LIBPATH} -DBOOST_NO_EXCEPTIONS -fno-exceptions ${UNREAL_HOSTED_CFLAGS}" \
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

RECAST_BASENAME=recast-${CXX_TAG}

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

  start=$(date +%s)

  git clone --depth 1 -b carla https://github.com/carla-simulator/recastnavigation.git ${RECAST_BASENAME}-source

  end=$(date +%s)
  echo "Elapsed Time downloading: $(($end-$start)) seconds"

  pushd ${RECAST_BASENAME}-source >/dev/null

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

  start=$(date +%s)
  wget ${LIBPNG_REPO}
  end=$(date +%s)
  echo "Elapsed Time downloading libpng: $(($end-$start)) seconds"

  start=$(date +%s)
  log "Extracting libpng."
  tar -xf libpng-${LIBPNG_VERSION}.tar.xz
  end=$(date +%s)
  echo "Elapsed Time Extracting libpng: $(($end-$start)) seconds"

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
XERCESC_REPO=https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-${XERCESC_VERSION}.tar.gz

XERCESC_SRC_DIR=${XERCESC_BASENAME}-source
XERCESC_INSTALL_DIR=${XERCESC_BASENAME}-install
XERCESC_INSTALL_SERVER_DIR=${XERCESC_BASENAME}-install-server
XERCESC_LIB=${XERCESC_INSTALL_DIR}/lib/libxerces-c.a
XERCESC_SERVER_LIB=${XERCESC_INSTALL_SERVER_DIR}/lib/libxerces-c.a

if [[ -d ${XERCESC_INSTALL_DIR} &&  -d ${XERCESC_INSTALL_SERVER_DIR} ]] ; then
  log "Xerces-c already installed."
else
  log "Retrieving xerces-c."
  start=$(date +%s)
  wget ${XERCESC_REPO}
  end=$(date +%s)
  echo "Elapsed Time downloading from xerces repo: $(($end-$start)) seconds"
  # try to use the backup boost we have in Jenkins
  if [[ ! -f "${XERCESC_BASENAME}.tar.gz" ]] ; then
    log "Using xerces backup"
    start=$(date +%s)
    wget "https://carla-releases.s3.us-east-005.backblazeb2.com/Backup/${XERCESC_BASENAME}.tar.gz" || true
    end=$(date +%s)
    echo "Elapsed Time downloading from xerces backup: $(($end-$start)) seconds"
  fi

  log "Extracting xerces-c."

  start=$(date +%s)
  tar -xzf ${XERCESC_BASENAME}.tar.gz
  end=$(date +%s)
  echo "Elapsed Time Extracting xerces-c: $(($end-$start)) seconds"

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

  mkdir -p ${XERCESC_INSTALL_SERVER_DIR}

  pushd ${XERCESC_SRC_DIR}/build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++ -fPIC -w -I${LLVM_INCLUDE} -L${LLVM_LIBPATH}" \
      -DCMAKE_INSTALL_PREFIX="../../${XERCESC_INSTALL_SERVER_DIR}" \
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

mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
cp -p ${XERCESC_SERVER_LIB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/

# ==============================================================================
# -- Get Eigen headers 3.1.0 (CARLA dependency) -------------------------------------
# ==============================================================================

EIGEN_VERSION=3.1.0
EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz
EIGEN_BASENAME=eigen-${EIGEN_VERSION}

EIGEN_SRC_DIR=eigen-${EIGEN_VERSION}-src
EIGEN_INSTALL_DIR=eigen-${EIGEN_VERSION}-install
EIGEN_INCLUDE=${EIGEN_INSTALL_DIR}/include


if [[ -d ${EIGEN_INSTALL_DIR} ]] ; then
  log "Eigen already installed."
else
  log "Retrieving Eigen."

  start=$(date +%s)
  wget ${EIGEN_REPO}
  end=$(date +%s)
  echo "Elapsed Time downloading from eigen repo: $(($end-$start)) seconds"

  log "Extracting Eigen."
  start=$(date +%s)
  tar -xzf ${EIGEN_BASENAME}.tar.gz
  end=$(date +%s)
  echo "Elapsed Time Extracting EIGEN: $(($end-$start)) seconds"

  mv ${EIGEN_BASENAME} ${EIGEN_SRC_DIR}
  mkdir -p ${EIGEN_INCLUDE}/unsupported
  mv ${EIGEN_SRC_DIR}/Eigen ${EIGEN_INCLUDE}
  mv ${EIGEN_SRC_DIR}/unsupported/Eigen ${EIGEN_INCLUDE}/unsupported/Eigen

  rm -Rf ${EIGEN_BASENAME}.tar.gz
  rm -Rf ${EIGEN_SRC_DIR}
fi

mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/
cp -p -r ${EIGEN_INCLUDE}/* ${LIBCARLA_INSTALL_SERVER_FOLDER}/include/

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

    start=$(date +%s)
    wget ${EIGEN_REPO}
    end=$(date +%s)
    echo "Elapsed Time: $(($end-$start)) seconds"

    log "Extracting Eigen."

    start=$(date +%s)
    tar -xzf ${EIGEN_BASENAME}.tar.gz
    end=$(date +%s)
    echo "Elapsed Time Extracting for Eigen: $(($end-$start)) seconds"

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
    start=$(date +%s)
    git clone --depth 1 --branch ${CHRONO_TAG} ${CHRONO_REPO} ${CHRONO_SRC_DIR}
    end=$(date +%s)
    echo "Elapsed Time dowloading chrono: $(($end-$start)) seconds"

    mkdir -p ${CHRONO_SRC_DIR}/build

    pushd ${CHRONO_SRC_DIR}/build >/dev/null

    cmake -G "Ninja" \
        -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH} -Wno-unused-command-line-argument ${UNREAL_HOSTED_CFLAGS}" \
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
SQLITE_FULL_LIB=${PWD}/${SQLITE_INSTALL_DIR}/lib/
SQLITE_EXE=${PWD}/${SQLITE_INSTALL_DIR}/bin/sqlite3

if [[ -d ${SQLITE_INSTALL_DIR} ]] ; then
  log "Sqlite already installed."
else
  log "Retrieving Sqlite3"

  start=$(date +%s)
  wget ${SQLITE_REPO}
  end=$(date +%s)
  echo "Elapsed Time: $(($end-$start)) seconds"

  log "Extracting Sqlite3"

  start=$(date +%s)
  tar -xzf ${SQLITE_TAR}
  end=$(date +%s)
  echo "Elapsed Time Extracting for SQlite: $(($end-$start)) seconds"

  mv ${SQLITE_VERSION} ${SQLITE_SOURCE_DIR}

  mkdir ${SQLITE_INSTALL_DIR}

  pushd ${SQLITE_SOURCE_DIR} >/dev/null

  export CFLAGS="-fPIC -w"
  ./configure --prefix=${PWD}/../sqlite-install/
  make
  make install

  popd >/dev/null

  rm -Rf ${SQLITE_TAR}
  rm -Rf ${SQLITE_SOURCE_DIR}
fi

mkdir -p ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/
cp ${SQLITE_LIB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/

mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
cp -p -r ${SQLITE_FULL_LIB} ${LIBCARLA_INSTALL_SERVER_FOLDER}

# ==============================================================================
# -- Get and compile PROJ ------------------------------------------------------
# ==============================================================================

PROJ_VERSION=proj-7.2.1
PROJ_REPO=https://download.osgeo.org/proj/${PROJ_VERSION}.tar.gz

PROJ_TAR=${PROJ_VERSION}.tar.gz
PROJ_SRC_DIR=proj-src
PROJ_INSTALL_DIR=proj-install
PROJ_INSTALL_SERVER_DIR=proj-install-server
PROJ_INSTALL_DIR_FULL=${PWD}/${PROJ_INSTALL_DIR}
PROJ_INSTALL_SERVER_DIR_FULL=${PWD}/${PROJ_INSTALL_SERVER_DIR}
PROJ_LIB=${PROJ_INSTALL_DIR_FULL}/lib/libproj.a
PROJ_SERVER_LIB=${PROJ_INSTALL_SERVER_DIR_FULL}/lib/libproj.a

if [[ -d ${PROJ_INSTALL_DIR} && -d ${PROJ_INSTALL_SERVER_DIR_FULL} ]] ; then
  log "PROJ already installed."
else
  log "Retrieving PROJ"

  start=$(date +%s)
  wget ${PROJ_REPO}
  end=$(date +%s)
  echo "Elapsed Time: $(($end-$start)) seconds"

  log "Extracting PROJ"
  start=$(date +%s)
  tar -xzf ${PROJ_TAR}
  end=$(date +%s)
  echo "Elapsed Time Extracting for PROJ: $(($end-$start)) seconds"

  mv ${PROJ_VERSION} ${PROJ_SRC_DIR}

  mkdir -p ${PROJ_SRC_DIR}/build
  mkdir -p ${PROJ_INSTALL_DIR}

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

  mkdir -p ${PROJ_INSTALL_SERVER_DIR}

  pushd ${PROJ_SRC_DIR}/build >/dev/null

  cmake -G "Ninja" .. \
      -DCMAKE_CXX_FLAGS="-std=c++14 -fPIC -stdlib=libc++ -I${LLVM_INCLUDE} -Wl,-L${LLVM_LIBPATH}"  \
      -DSQLITE3_INCLUDE_DIR=${SQLITE_INCLUDE_DIR} -DSQLITE3_LIBRARY=${SQLITE_LIB} \
      -DEXE_SQLITE3=${SQLITE_EXE} \
      -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_PROJSYNC=OFF \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_PROJINFO=OFF \
      -DBUILD_CCT=OFF -DBUILD_CS2CS=OFF -DBUILD_GEOD=OFF -DBUILD_GIE=OFF \
      -DBUILD_PROJ=OFF -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX=${PROJ_INSTALL_SERVER_DIR_FULL}
  ninja
  ninja install

  popd >/dev/null

  rm -Rf ${PROJ_TAR}
  rm -Rf ${PROJ_SRC_DIR}

fi

cp ${PROJ_LIB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/lib/

mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
cp -p ${PROJ_SERVER_LIB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/

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

  start=$(date +%s)
  wget ${PATCHELF_REPO}
  end=$(date +%s)
  echo "Elapsed Time: $(($end-$start)) seconds"

  log "Extracting patchelf"
  start=$(date +%s)
  tar -xzf ${PATCHELF_TAR}
  end=$(date +%s)
  echo "Elapsed Time Extracting patchelf: $(($end-$start)) seconds"

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
# -- Download libtorch and dependencies ----------------------------------------
# ==============================================================================

if ${USE_PYTORCH} ; then

  LIBTORCH_BASENAME=libtorch

  LIBTORCH_PATH=${PWD}/${LIBTORCH_BASENAME}
  LIBTORCH_INCLUDE=${LIBTORCH_PATH}/include
  LIBTORCH_LIB=${LIBTORCH_PATH}/lib
  LIBTORCH_ZIPFILE=libtorch-shared-with-deps-1.11.0+cu113.zip
  LIBTORCH_REPO=https://download.pytorch.org/libtorch/cu113/libtorch-shared-with-deps-1.11.0%2Bcu113.zip
  if [[ ! -d ${LIBTORCH_PATH} ]] ; then

    start=$(date +%s)
    wget ${LIBTORCH_REPO}
    end=$(date +%s)
    echo "Elapsed Time downloading LIBTORCH_REPO: $(($end-$start)) seconds"

    unzip ${LIBTORCH_ZIPFILE}
  fi

  function build_torch_extension {

    LIB_SOURCE=$1
    LIB_INSTALL=$2
    LIB_REPO=$3
    if [[ ! -d ${LIB_INSTALL} ]] ; then
      if [[ ! -d ${LIB_SOURCE} ]] ; then
        mkdir -p ${LIB_SOURCE}
        log "${LIB_REPO}"
        git clone ${LIB_REPO} ${LIB_SOURCE}
        mkdir -p ${LIB_SOURCE}/build
      fi
      pushd ${LIB_SOURCE}/build >/dev/null

      cmake -DCMAKE_PREFIX_PATH="${LIBTORCH_PATH}" \
          -DCMAKE_CUDA_COMPILER="/usr/local/cuda/bin/nvcc" \
          -DCMAKE_INSTALL_PREFIX="${LIB_INSTALL}" \
          -DCMAKE_CUDA_FLAGS="-gencode=arch=compute_35,code=sm_35 -gencode=arch=compute_37,code=sm_37 -gencode=arch=compute_50,code=sm_50 -gencode=arch=compute_52,code=sm_52 -gencode=arch=compute_53,code=sm_53 -gencode=arch=compute_60,code=sm_60 -gencode=arch=compute_61,code=sm_61 -gencode=arch=compute_62,code=sm_62 -gencode=arch=compute_70,code=sm_70 -gencode=arch=compute_72,code=sm_72 -gencode=arch=compute_75,code=sm_75 -gencode=arch=compute_80,code=sm_80 -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_87,code=sm_87 -Wno-deprecated-gpu-targets" \
          -DWITH_CUDA=ON \
          ..
      make
      make install
      popd >/dev/null
    fi

  }

  log "Build libtorch scatter"
  #LibtorchScatter
  LIBTORCHSCATTER_BASENAME=libtorchscatter
  LIBTORCHSCATTER_SOURCE_DIR=${PWD}/${LIBTORCHSCATTER_BASENAME}-source
  LIBTORCHSCATTER_INSTALL_DIR=${PWD}/${LIBTORCHSCATTER_BASENAME}-install
  LIBTORCHSCATTER_INCLUDE=${LIBTORCHSCATTER_INSTALL_DIR}/include
  LIBTORCHSCATTER_LIB=${LIBTORCHSCATTER_INSTALL_DIR}/lib
  LIBTORCHSCATTER_REPO="https://github.com/rusty1s/pytorch_scatter.git"

  build_torch_extension ${LIBTORCHSCATTER_SOURCE_DIR} ${LIBTORCHSCATTER_INSTALL_DIR} "${LIBTORCHSCATTER_REPO}"

  log "Build libtorch cluster"
  LIBTORCHCLUSTER_BASENAME=libtorchcluster
  LIBTORCHCLUSTER_SOURCE_DIR=${PWD}/${LIBTORCHCLUSTER_BASENAME}-source
  LIBTORCHCLUSTER_INSTALL_DIR=${PWD}/${LIBTORCHCLUSTER_BASENAME}-install
  LIBTORCHCLUSTER_INCLUDE=${LIBTORCHCLUSTER_INSTALL_DIR}/include
  LIBTORCHCLUSTER_LIB=${LIBTORCHCLUSTER_INSTALL_DIR}/lib
  LIBTORCHCLUSTER_REPO="https://github.com/rusty1s/pytorch_cluster.git"

  build_torch_extension ${LIBTORCHCLUSTER_SOURCE_DIR} ${LIBTORCHCLUSTER_INSTALL_DIR} "${LIBTORCHCLUSTER_REPO}"

  mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  cp -p ${LIBTORCH_LIB}/*.a ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  cp -p ${LIBTORCH_LIB}/*.so* ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  cp -p ${LIBTORCHSCATTER_LIB}/*.so ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  cp -p ${LIBTORCHCLUSTER_LIB}/*.so ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/

  mkdir -p ${CARLAUE4_PLUGIN_ROOT_FOLDER}/Binaries/Linux/
  cp -p ${LIBTORCH_LIB}/*.so* ${CARLAUE4_PLUGIN_ROOT_FOLDER}/Binaries/Linux/
  cp -p ${LIBTORCHSCATTER_LIB}/*.so* ${CARLAUE4_PLUGIN_ROOT_FOLDER}/Binaries/Linux/
  cp -p ${LIBTORCHCLUSTER_LIB}/*.so* ${CARLAUE4_PLUGIN_ROOT_FOLDER}/Binaries/Linux/
fi

# ==============================================================================
# -- Download Fast DDS and dependencies ----------------------------------------
# ==============================================================================

FASTDDS_BASENAME=fast-dds
FASTDDS_INSTALL_DIR=${PWD}/${FASTDDS_BASENAME}-install
FASTDDS_INCLUDE=${FASTDDS_INSTALL_DIR}/include
FASTDDS_LIB=${FASTDDS_INSTALL_DIR}/lib
if ${USE_ROS2} ; then
  function build_fastdds_extension {
    LIB_SOURCE=$1
    LIB_REPO=$2
    LIB_BRANCH=$3
    if [[ ! -d ${LIB_SOURCE} ]] ; then
      mkdir -p ${LIB_SOURCE}
      log "${LIB_REPO}"
      start=$(date +%s)
      git clone --depth 1 --branch ${LIB_BRANCH} ${LIB_REPO} ${LIB_SOURCE}
      end=$(date +%s)
      echo "Elapsed Time dowloading fastdds extension: $(($end-$start)) seconds"
      mkdir -p ${LIB_SOURCE}/build
    fi
  }
  if [[ ! -d ${FASTDDS_INSTALL_DIR} ]] ; then
    mkdir -p ${FASTDDS_INSTALL_DIR}
    log "Build foonathan memory vendor"
    FOONATHAN_MEMORY_VENDOR_BASENAME=foonathan-memory-vendor
    FOONATHAN_MEMORY_VENDOR_SOURCE_DIR=${PWD}/${FOONATHAN_MEMORY_VENDOR_BASENAME}-source
    FOONATHAN_MEMORY_VENDOR_REPO="https://github.com/eProsima/foonathan_memory_vendor.git"
    FOONATHAN_MEMORY_VENDOR_BRANCH=master
    build_fastdds_extension ${FOONATHAN_MEMORY_VENDOR_SOURCE_DIR} "${FOONATHAN_MEMORY_VENDOR_REPO}" "${FOONATHAN_MEMORY_VENDOR_BRANCH}"
    pushd ${FOONATHAN_MEMORY_VENDOR_SOURCE_DIR}/build >/dev/null
    cmake -G "Ninja" \
      -DCMAKE_INSTALL_PREFIX="${FASTDDS_INSTALL_DIR}" \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" \
      -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON \
      ..
    ninja
    ninja install
    popd >/dev/null
    rm -Rf ${FOONATHAN_MEMORY_VENDOR_SOURCE_DIR}

    log "Build fast cdr"
    FAST_CDR_BASENAME=fast-cdr
    FAST_CDR_SOURCE_DIR=${PWD}/${FAST_CDR_BASENAME}-source
    FAST_CDR_REPO="https://github.com/eProsima/Fast-CDR.git"
    FAST_CDR_BRANCH=1.1.x
    build_fastdds_extension ${FAST_CDR_SOURCE_DIR} "${FAST_CDR_REPO}" "${FAST_CDR_BRANCH}"
    pushd ${FAST_CDR_SOURCE_DIR}/build >/dev/null
    cmake -G "Ninja" \
      -DCMAKE_INSTALL_PREFIX="${FASTDDS_INSTALL_DIR}" \
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" \
      ..
    ninja
    ninja install
    popd >/dev/null
    rm -Rf ${FAST_CDR_SOURCE_DIR}

    log "Build fast dds"
    FAST_DDS_LIB_BASENAME=fast-dds-lib
    FAST_DDS_LIB_SOURCE_DIR=${PWD}/${FAST_DDS_LIB_BASENAME}-source
    FAST_DDS_LIB_REPO="https://github.com/eProsima/Fast-DDS.git"
    FAST_DDS_LIB_BRANCH=2.11.2
    build_fastdds_extension ${FAST_DDS_LIB_SOURCE_DIR} "${FAST_DDS_LIB_REPO}" "${FAST_DDS_LIB_BRANCH}"
    pushd ${FAST_DDS_LIB_SOURCE_DIR}/build >/dev/null
    cmake -G "Ninja" \
      -DCMAKE_INSTALL_PREFIX="${FASTDDS_INSTALL_DIR}" \
      -DCMAKE_CXX_FLAGS=-latomic \
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" \
      -DTHIRDPARTY_Asio=FORCE \
      -DTHIRDPARTY_TinyXML2=FORCE \
      ..
    ninja
    ninja install
    popd >/dev/null
    rm -Rf ${FAST_DDS_LIB_SOURCE_DIR}

    mkdir -p ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
    cp -p ${FASTDDS_LIB}/*.so* ${LIBCARLA_INSTALL_SERVER_FOLDER}/lib/
  fi
fi

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
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -isystem ${LLVM_INCLUDE} ${UNREAL_HOSTED_CFLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "\${CMAKE_CXX_FLAGS} -fno-exceptions" CACHE STRING "" FORCE)
set(CMAKE_CXX_LINK_FLAGS "\${CMAKE_CXX_LINK_FLAGS} -L${LLVM_LIBPATH} ${UNREAL_HOSTED_CFLAGS}" CACHE STRING "" FORCE)
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
set(FASTDDS_INCLUDE_PATH "${FASTDDS_INCLUDE}")
set(FASTDDS_LIB_PATH "${FASTDDS_LIB}")

if (CMAKE_BUILD_TYPE STREQUAL "Server")
  # Here libraries linking libc++.
  set(LLVM_INCLUDE_PATH "${LLVM_INCLUDE}")
  set(LLVM_LIB_PATH "${LLVM_LIBPATH}")
  set(RPCLIB_INCLUDE_PATH "${RPCLIB_LIBCXX_INCLUDE}")
  set(RPCLIB_LIB_PATH "${RPCLIB_LIBCXX_LIBPATH}")
  set(GTEST_INCLUDE_PATH "${GTEST_LIBCXX_INCLUDE}")
  set(GTEST_LIB_PATH "${GTEST_LIBCXX_LIBPATH}")
elseif (CMAKE_BUILD_TYPE STREQUAL "ros2")
  list(APPEND CMAKE_PREFIX_PATH "${FASTDDS_INSTALL_DIR}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Pytorch")
  list(APPEND CMAKE_PREFIX_PATH "${LIBTORCH_PATH}")
  list(APPEND CMAKE_PREFIX_PATH "${LIBTORCHSCATTER_INSTALL_DIR}")
  list(APPEND CMAKE_PREFIX_PATH "${LIBTORCHCLUSTER_INSTALL_DIR}")
  list(APPEND CMAKE_PREFIX_PATH "${LIBTORCHSPARSE_INSTALL_DIR}")
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
