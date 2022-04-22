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

ADRSS_VERSION=4.4.3
ADRSS_BASENAME=ad-rss-${ADRSS_VERSION}
ADRSS_COLCON_WORKSPACE="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}"
ADRSS_SRC_DIR="${ADRSS_COLCON_WORKSPACE}/src"

if [[ ! -d "${ADRSS_SRC_DIR}" ]]; then
  # ad-rss is built inside a colcon workspace, therefore we have to setup the workspace first
  log "Retrieving ${ADRSS_BASENAME}."

  mkdir -p "${ADRSS_SRC_DIR}"

  pushd "${ADRSS_SRC_DIR}" >/dev/null
  git clone --depth=1 -b v1.7.0 https://github.com/gabime/spdlog.git
  git clone --depth=1 -b v2.4.5_hotfix https://github.com/carla-simulator/map.git
  git clone --depth=1 -b v${ADRSS_VERSION} https://github.com/intel/ad-rss-lib.git
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
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
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

CARLA_LLVM_VERSION_MAJOR=$(cut -d'.' -f1 <<<"$(clang -dumpversion)")

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
    if [ "${CMAKE_PREFIX_PATH}" == "" ]; then
      CMAKE_PREFIX_PATH="${CARLA_BUILD_FOLDER}/boost-1.72.0-c$CARLA_LLVM_VERSION_MAJOR-install;${CARLA_BUILD_FOLDER}/proj-install"
    else
      CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH};${CARLA_BUILD_FOLDER}/boost-1.72.0-c$CARLA_LLVM_VERSION_MAJOR-install;${CARLA_BUILD_FOLDER}/proj-install"
    fi

    # get the python version of the binding to be built
    PYTHON_VERSION=$(/usr/bin/env python${PY_VERSION} -V 2>&1)
    PYTHON_BINDING_VERSIONS=${PYTHON_VERSION:7:3}
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
