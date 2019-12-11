#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

export CC=/usr/bin/clang-7
export CXX=/usr/bin/clang++-7

source $(dirname "$0")/Environment.sh

mkdir -p ${CARLA_BUILD_FOLDER}
pushd ${CARLA_BUILD_FOLDER} >/dev/null

# ==============================================================================
# -- Get and compile ad-rss ----------------------------------------------------
# ==============================================================================

AD_RSS_BASENAME=ad-rss

if [[ -d "${AD_RSS_BASENAME}-install" ]] ; then
  log "${AD_RSS_BASENAME} already installed."
else
  rm -Rf ${AD_RSS_BASENAME}-source ${AD_RSS_BASENAME}-build

  log "Retrieving ad-rss."

  git clone --depth=1 -b v1.5.0 https://github.com/intel/ad-rss-lib.git ${AD_RSS_BASENAME}-source

  log "Compiling ad-rss."

  mkdir -p ${AD_RSS_BASENAME}-build

  pushd ${AD_RSS_BASENAME}-build >/dev/null

  cmake -G "Ninja" \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX="../${AD_RSS_BASENAME}-install" \
      ../${AD_RSS_BASENAME}-source

  ninja install

  popd >/dev/null

  rm -Rf ${AD_RSS_BASENAME}-source ${AD_RSS_BASENAME}-build

fi

unset AD_RSS_BASENAME

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
