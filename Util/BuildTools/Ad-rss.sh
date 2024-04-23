#! /bin/bash


OPTS=`getopt -o h --long python-version: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

PY_VERSION_LIST=3

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version )
      PY_VERSION_LIST="$2";
      shift 2 ;;
    * )
      shift ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================
source $(dirname "$0")/Environment.sh

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

# ==============================================================================
# -- Get ad-rss -------------------------------------------
# ==============================================================================

ADRSS_VERSION=4.4.4
ADRSS_BASENAME=ad-rss-${ADRSS_VERSION}
ADRSS_COLCON_WORKSPACE="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}"
ADRSS_SRC_DIR="${ADRSS_COLCON_WORKSPACE}/src"

if [[ ! -d "${ADRSS_SRC_DIR}" ]]; then
  # ad-rss is built inside a colcon workspace, therefore we have to setup the workspace first
  log "Retrieving ${ADRSS_BASENAME}."

  mkdir -p "${ADRSS_SRC_DIR}"

  # clone ad-rss with all submodules, but remove proj, as CARLA already uses it
  pushd "${ADRSS_SRC_DIR}" >/dev/null
  git clone -b v${ADRSS_VERSION} https://github.com/intel/ad-rss-lib.git && cd ad-rss-lib && git submodule update --init --recursive && rm -rf dependencies/map/dependencies/PROJ4 && cd ..

  # ADRSS_VERSION is designed for older boost, update datatype from boost::array to std::array
  grep -rl "boost::array" | xargs sed -i 's/boost::array/std::array/g'
  grep -rl "find_package(Boost" | xargs sed -i 's/find_package(Boost/find_package(Boost 1.80/g'
  popd

  cat >"${ADRSS_COLCON_WORKSPACE}/colcon.meta" <<EOL
{
    "names": {
        "ad_physics": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_access": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_opendrive_reader": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"],
            "dependencies": ["odrSpiral"]
        },
        "ad_rss": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_rss_map_integration": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "spdlog": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF"]
        }
    }
}

EOL
fi

# ==============================================================================
# -- Build ad-rss -------------------------------------------
# ==============================================================================
ADRSS_INSTALL_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/install"

# Automation doesn't work if the clang installed within the system is not matching the one that is provided by Unreal
# Setup.sh would have to be adapted to reflect it
# so we need to change
#CARLA_LLVM_VERSION_MAJOR=$(cut -d'.' -f1 <<<"$(clang --version | head -n 1 | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')")
#if [ -z "$CARLA_LLVM_VERSION_MAJOR" ] ; then
#  fatal_error "Failed to retrieve the installed version of the clang compiler."
#else
#  echo "Using clang-$CARLA_LLVM_VERSION_MAJOR as the CARLA compiler."
#fi
CXX_TAG=c10

#
# Since it it not possible with boost-python to build more than one python version at once (find_package has some bugs)
# we have to build it for every version in a separate colcon build
#
for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
  ADRSS_BUILD_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python${PY_VERSION}"

  if [[ -d "${ADRSS_INSTALL_DIR}" && -d "${ADRSS_BUILD_DIR}" ]]; then
    log "${ADRSS_BASENAME} for python${PY_VERSION} already installed."
  else
    log "Building ${ADRSS_BASENAME} for python${PY_VERSION}"

    pushd "${ADRSS_COLCON_WORKSPACE}" >/dev/null
    if [[ "${CMAKE_PREFIX_PATH}" == "" ]]; then
      CMAKE_PREFIX_PATH="${CARLA_BUILD_FOLDER}/boost-1.80.0-$CXX_TAG-install;${CARLA_BUILD_FOLDER}/proj-install"
    else
      CMAKE_PREFIX_PATH="${CARLA_BUILD_FOLDER}/boost-1.80.0-$CXX_TAG-install;${CARLA_BUILD_FOLDER}/proj-install;${CMAKE_PREFIX_PATH}"
    fi

    # get the python version of the binding to be built, need to query the binary,
    # because might be just provided a '3' as PY_VERSION and then the symbolic linked python3 is called
    PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
    PYTHON_BINDING_VERSIONS=${PYTHON_VERSION:7}
    PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS%.*}
    echo "PYTHON_BINDING_VERSIONS=${PYTHON_BINDING_VERSIONS}"

    # enforce sequential executor to reduce the required memory for compilation
    colcon build --executor sequential --packages-up-to ad_rss_map_integration --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE="${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake" -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}" -DPYTHON_BINDING_VERSIONS="${PYTHON_BINDING_VERSIONS}" --build-base ${ADRSS_BUILD_DIR} --install-base ${ADRSS_INSTALL_DIR}

    COLCON_RESULT=$?
    if (( COLCON_RESULT )); then
      rm -rf "${ADRSS_INSTALL_DIR}"
      log "Failed !"
    else
      log "Success!"
    fi
    popd >/dev/null
  fi
done
