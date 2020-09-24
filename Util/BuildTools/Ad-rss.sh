#! /bin/bash


PY_VERSION=3

while [[ $# -gt 0 ]]; do
  case "$1" in
    --python-version=*)
      PY_VERSION="${1/--python-version=/}";
      shift ;;
    * )
      shift ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================
source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Get and compile ad-rss -------------------------------------------
# ==============================================================================

ADRSS_BASENAME=ad-rss-4.1.0
ADRSS_INSTALL_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/install"
ADRSS_BUILD_DIR="${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python${PY_VERSION}"

if [[ -d "${ADRSS_INSTALL_DIR}" && -d "${ADRSS_BUILD_DIR}" ]]; then
  log "${ADRSS_BASENAME} for python${PY_VERSION} already installed."
else
  log "Building ${ADRSS_BASENAME} for python${PY_VERSION}."
  if [[ ! -d "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src" ]]; then
    # ad-rss is built inside a colcon workspace, therefore we have to setup the workspace first
    if [[ -d "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python2" ]]; then
      rm -rf "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python2"
    fi
    if [[ -d "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python3" ]]; then
      rm -rf "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/build-python3"
    fi

    mkdir -p "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src"

    cat >"${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src/0001-Workaround-for-boost-python-binding.patch" <<EOL
---
 cmake/python-binding.cmake | 4 ++++
 1 file changed, 4 insertions(+)

diff --git a/cmake/python-binding.cmake b/cmake/python-binding.cmake
index d39dbf5..0f2fbe8 100644
--- a/cmake/python-binding.cmake
+++ b/cmake/python-binding.cmake
@@ -40,6 +40,7 @@ function(find_python_binding_packages)
   endif()

   #python2
+  if("\${PY_VERSION}" STREQUAL "" OR "\${PY_VERSION}" STREQUAL "2")
   unset(PYTHON_INCLUDE_DIR)
   unset(PYTHON_INCLUDE_DIR CACHE)
   unset(PYTHON_LIBRARY)
@@ -67,8 +68,10 @@ function(find_python_binding_packages)
       \${LOCAL_PYTHON_BINDING_PACKAGE_LIBRARIES_PYTHON2}
       PARENT_SCOPE)
   endif()
+  endif()

   #python3
+  if("\${PY_VERSION}" STREQUAL "" OR "\${PY_VERSION}" STREQUAL "3")
   unset(PYTHON_INCLUDE_DIR)
   unset(PYTHON_INCLUDE_DIR CACHE)
   unset(PYTHON_LIBRARY)
@@ -95,6 +98,7 @@ function(find_python_binding_packages)
       \${LOCAL_PYTHON_BINDING_PACKAGE_LIBRARIES_PYTHON3}
       PARENT_SCOPE)
   endif()
+  endif()

   set(PYTHON_BINDINGS \${LOCAL_PYTHON_BINDINGS} PARENT_SCOPE)

--
2.17.1
EOL

    log "Retrieving ${ADRSS_BASENAME}."

    pushd "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src" >/dev/null
    git clone --depth=1 -b v1.7.0 https://github.com/gabime/spdlog.git
    git clone --depth=1 -b 4.9.3 https://github.com/OSGeo/PROJ.git
    git clone --depth=1 -b v2.1.0 https://github.com/carla-simulator/map.git
    #patch
    pushd map </dev/null
    git apply ${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src/0001-Workaround-for-boost-python-binding.patch
    popd
    git clone --depth=1 -b v4.1.0 https://github.com/intel/ad-rss-lib.git
    #patch
    pushd ad-rss-lib </dev/null
    git apply ${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/src/0001-Workaround-for-boost-python-binding.patch
    popd
    popd


    cat >"${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}/colcon.meta" <<EOL
{
    "names": {
        "PROJ4": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_LIBPROJ_SHARED=OFF"]
        },
        "ad_physics": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_access": {
            "cmake-args": ["-DBUILD_PYTHON_BINDING=ON", "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"]
        },
        "ad_map_opendrive_reader": {
            "cmake-args": ["-DCMAKE_POSITION_INDEPENDENT_CODE=ON", "-DBUILD_SHARED_LIBS=OFF", "-DDISABLE_WARNINGS_AS_ERRORS=ON"],
            "dependencies": ["PROJ4"]
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

  log "Compiling ${ADRSS_BASENAME}."

  pushd "${CARLA_BUILD_FOLDER}/${ADRSS_BASENAME}" >/dev/null
  if [ "${CMAKE_PREFIX_PATH}" == "" ]; then
    export CMAKE_PREFIX_PATH=${CARLA_BUILD_FOLDER}/boost-1.72.0-c8-install
  else
    export CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}:${CARLA_BUILD_FOLDER}/boost-1.72.0-c8-install
  fi

  # enforce sequential executor to reduce the required memory for compilation
  colcon build --executor sequential --packages-up-to ad_rss_map_integration --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE="${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake" -DPY_VERSION=${PY_VERSION} --build-base ${ADRSS_BUILD_DIR} --install-base ${ADRSS_INSTALL_DIR}

  COLCON_RESULT=$?
  if (( COLCON_RESULT )); then
    rm -rf "${ADRSS_INSTALL_DIR}"
    log "Failed !"
  else
    log "Success!"
  fi

  # ==============================================================================
  # -- ...and we are done --------------------------------------------------------
  # ==============================================================================

  popd >/dev/null

fi
