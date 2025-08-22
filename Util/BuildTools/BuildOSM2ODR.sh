#! /bin/bash
DOC_STRING="Build OSM2ODR."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)

REMOVE_INTERMEDIATE=false
BUILD_OSM2ODR=false
GIT_PULL=true
CURRENT_OSM2ODR_COMMIT=1da3c07e39f3e2e0d97f8f709a7255a0df8c6200
OSM2ODR_BRANCH=aaron/defaultsidewalkwidth
OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git

OPTS=`getopt -o h --long help,rebuild,build,clean,carsim,no-pull,chrono,chrono-path: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_OSM2ODR=true;
      shift ;;
    --build )
      BUILD_OSM2ODR=true;
      shift ;;
    --no-pull )
      GIT_PULL=false
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    --chrono )
      shift ;;
    --chrono-path )
      shift 2 ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

source $(dirname "$0")/Environment.sh

function get_source_code_checksum {
  local EXCLUDE='*__pycache__*'
  find "${OSM2ODR_SOURCE_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_OSM2ODR}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${OSM2ODR_BUILD_FOLDER}*

fi

# ==============================================================================
# -- Build library -------------------------------------------------------------
# ==============================================================================

if ${BUILD_OSM2ODR} ; then
  log "Building OSM2ODR."
  if [ ! -d ${OSM2ODR_SOURCE_FOLDER} ] ; then
    cd ${CARLA_BUILD_FOLDER}
    curl --retry 5 --retry-max-time 120 -L -o OSM2ODR.zip https://github.com/carla-simulator/sumo/archive/${CURRENT_OSM2ODR_COMMIT}.zip
    unzip -qq OSM2ODR.zip
    rm -f OSM2ODR.zip
    mv sumo-${CURRENT_OSM2ODR_COMMIT} ${OSM2ODR_SOURCE_FOLDER}
  fi

  mkdir -p ${OSM2ODR_BUILD_FOLDER}
  cd ${OSM2ODR_BUILD_FOLDER}


  export CC="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang"
  export CXX="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++"
  export PATH="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin:$PATH"

  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-stdlib=libstdc++" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/lib/libxerces-c.a

  ninja osm2odr
  ninja install

  mkdir -p ${OSM2ODR_SERVER_BUILD_FOLDER}
  cd ${OSM2ODR_SERVER_BUILD_FOLDER}

  LLVM_BASENAME=llvm-8.0
  LLVM_INCLUDE="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1"
  LLVM_LIBPATH="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu"

  echo $LLVM_INCLUDE
  echo $LLVM_LIBPATH

  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH}" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_SERVER_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install-server/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install-server/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/lib/libxerces-c.a

  ninja osm2odr
  ninja install

  mkdir -p ${OSM2ODR_SERVER_BUILD_FOLDER}
  cd ${OSM2ODR_SERVER_BUILD_FOLDER}

  LLVM_BASENAME=llvm-8.0
  LLVM_INCLUDE="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/include/c++/v1"
  LLVM_LIBPATH="$UE4_ROOT/Engine/Source/ThirdParty/Linux/LibCxx/lib/Linux/x86_64-unknown-linux-gnu"

  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_CXX_FLAGS="-fPIC -std=c++14 -stdlib=libc++ -I${LLVM_INCLUDE} -L${LLVM_LIBPATH}" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_SERVER_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install-server/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install-server/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install-server/lib/libxerces-c.a

  ninja osm2odr
  ninja install

fi

log " OSM2ODR Success!"
