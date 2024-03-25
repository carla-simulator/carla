#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

# ==== BOOST ====

carla_string_option (
  CARLA_BOOST_VERSION
  "Target boost version."
  "1.84.0"
)

carla_string_option (
  CARLA_BOOST_TAG
  "Target boost git tag."
  "boost-${CARLA_BOOST_VERSION}"
)

# ==== EIGEN ====

carla_string_option (
  CARLA_EIGEN_VERSION
  "Target Eigen version."
  "3.4.0"
)

carla_string_option (
  CARLA_EIGEN_TAG
  "Target Eigen git tag."
  "${CARLA_EIGEN_VERSION}"
)

# ==== GOOGLE TEST ====

carla_string_option (
  CARLA_GTEST_VERSION
  "Target googletest version."
  "1.14.0"
)

carla_string_option (
  CARLA_GTEST_TAG
  "Target googletest git tag."
  "v${CARLA_GTEST_VERSION}"
)

# ==== LIBPNG ====

carla_string_option (
  CARLA_LIBPNG_VERSION
  "Target libpng version."
  "1.6.40"
)

carla_string_option (
  CARLA_LIBPNG_TAG
  "Target libpng git tag."
  "v${CARLA_LIBPNG_VERSION}"
)

# ==== PROJ ====

carla_string_option (
  CARLA_PROJ_VERSION
  "Target PROJ version."
  "7.2.1"
)

carla_string_option (
  CARLA_PROJ_TAG
  "Target PROJ git tag."
  "${CARLA_PROJ_VERSION}"
)

# ==== RECAST ====

carla_string_option (
  CARLA_RECAST_TAG
  "Target recastnavigation git tag."
  "carla"
)

# ==== RPCLIB ====

carla_string_option (
  CARLA_RPCLIB_TAG
  "Target rpclib git tag."
  "carla"
)

# ==== SQLITE3 ====

carla_string_option (
  CARLA_SQLITE_VERSION
  "Target SQLite version (major[1].minor[2].patch[2].revision[2])."
  "3.45.01.00"
)

# ==== XERCESC ====

carla_string_option (
  CARLA_XERCESC_VERSION
  "Target xerces-c version."
  "3.2.4"
)

carla_string_option (
  CARLA_XERCESC_TAG
  "Target xerces-c git tag."
  "v${CARLA_XERCESC_VERSION}"
)

# ==== ZLIB ====

carla_string_option (
  CARLA_ZLIB_VERSION
  "Target zlib git tag."
  "1.3.1)"

carla_string_option (
  CARLA_ZLIB_TAG
  "Target zlib git tag."
  "v${CARLA_ZLIB_VERSION})"
  
# ==== LUNASVG ====

carla_string_option (
  CARLA_LUNASVG_VERSION
  "Target lunasvg version."
  "master"
)

carla_string_option (
  CARLA_LUNASVG_TAG
  "Target lunasvg git tag."
  "${CARLA_LUNASVG_VERSION}"
)

# ==== LIBOSMSCOUT ====

carla_string_option (
  CARLA_LIBOSMSCOUT_VERSION
  "Target libosmscout version."
  "master"
)

carla_string_option (
  CARLA_LIBOSMSCOUT_TAG
  "Target libosmscout git tag."
  "${CARLA_LIBOSMSCOUT_VERSION}"
)


# ==== STREETMAP ====

carla_string_option (
  CARLA_STREETMAP_VERSION
  "Target StreetMap version."
  "ue5.3"
)

carla_string_option (
  CARLA_STREETMAP_TAG
  "Target StreetMap git tag."
  "${CARLA_STREETMAP_VERSION}"
)
