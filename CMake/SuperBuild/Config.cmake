set (CARLA_VERSION_MAJOR 0)
set (CARLA_VERSION_MINOR 10)
set (CARLA_VERSION_PATCH 0)

set (
  CARLA_VERSION 
  ${CARLA_VERSION_MAJOR}.${CARLA_VERSION_MINOR}.${CARLA_VERSION_PATCH}
)

set (
  BOOST_VERSION
  "1.84.0"
  CACHE STRING
  "Target boost version."
)

set (
  BOOST_TAG
  "boost-${BOOST_VERSION}"
  CACHE STRING
  "Target boost git tag."
)

set (
  EIGEN_VERSION
  "3.4.0"
  CACHE STRING
  "Target Eigen version."
)

set (
  CARLA_EIGEN_TAG
  "${EIGEN_VERSION}"
  CACHE STRING
  "Target Eigen git tag."
)

set (
  FASTCDR_VERSION
  "1.1.x"
  CACHE STRING
  "Target Fast-CDR version."
)

set (
  FASTCDR_TAG
  "${FASTCDR_VERSION}"
  CACHE STRING
  "Target Fast-CDR git tag."
)

set (
  FASTDDS_VERSION
  "3.4.1"
  CACHE STRING
  "Target Fast-DDS version."
)

set (
  CARLA_FAST_DDS_TAG
  "v${FASTDDS_VERSION}"
  CACHE STRING
  "Target Fast-DDS git tag."
)

set (
  FOONATHAN_MEMORY_VENDOR_VERSION
  "master"
  CACHE STRING
  "Target foonathan_memory_vendor version."
)

set (
  FOONATHAN_MEMORY_VENDOR_TAG
  "${FOONATHAN_MEMORY_VENDOR_VERSION}"
  CACHE STRING
  "Target foonathan_memory_vendor git tag."
)

set (
  GTEST_VERSION
  "1.14.0"
  CACHE STRING
  "Target googletest version."
)

set (
  GTEST_TAG
  "v${GTEST_VERSION}"
  CACHE STRING
  "Target googletest git tag."
)

set (
  LIBOSMSCOUT_VERSION
  "master"
  CACHE STRING
  "Target libosmscout version."
)

set (
  LIBOSMSCOUT_TAG
  "${LIBOSMSCOUT_VERSION}"
  CACHE STRING
  "Target libosmscout git tag."
)

set (
  LUNASVG_VERSION
  "master"
  CACHE STRING
  "Target lunasvg version."
)

set (
  LUNASVG_TAG
  "${LUNASVG_VERSION}"
  CACHE STRING
  "Target lunasvg git tag."
)

set (
  PNG_VERSION
  "1.6.54"
  CACHE STRING
  "Target libpng version."
)

set (
  PNG_TAG
  "v${PNG_VERSION}"
  CACHE STRING
  "Target libpng git tag."
)

set (
  PROJ_VERSION
  "9.7.0"
  CACHE STRING
  "Target PROJ version."
)

set (
  PROJ_TAG
  "${PROJ_VERSION}"
  CACHE STRING
  "Target PROJ git tag."
)

set (
  RECAST_TAG
  "carla"
  CACHE STRING
  "Target recastnavigation git tag."
)

set (
  RPC_TAG
  "carla"
  CACHE STRING
  "Target rpclib git tag."
)

set (
  SQLITE_VERSION
  "3.50.04.00"
  CACHE STRING
  "Target SQLite version (major[1].minor[2].patch[2].revision[2])."
)

string (REPLACE "." "" CARLA_SQLITE_TAG ${SQLITE_VERSION})

set (
  SQLITE_RELEASE_YEAR
  "2025"
  CACHE STRING
  "Year corresponding to the target SQLite release."
)

set (
  STREETMAP_VERSION
  "ue5-dev-carla"
  CACHE STRING
  "Target StreetMap version."
)

set (
  CARLA_STREETMAP_TAG
  "${STREETMAP_VERSION}"
  CACHE STRING
  "Target StreetMap git tag."
)

set (
  XERCESC_VERSION
  "3.3.0"
  CACHE STRING
  "Target xerces-c version."
)

set (
  XERCESC_TAG
  "v${XERCESC_VERSION}"
  CACHE STRING
  "Target xerces-c git tag."
)

set (
  ZLIB_VERSION
  "1.3.1"
  CACHE STRING
  "Target zlib git tag."
)

set (
  ZLIB_TAG
  "v${ZLIB_VERSION}"
  CACHE STRING
  "Target zlib git tag."
)
