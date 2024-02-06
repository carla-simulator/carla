include (Util)

carla_string_option (
  CARLA_PROJ_VERSION
  "Target PROJ version."
  7.2.1
)

carla_string_option (
  CARLA_PROJ_TAG
  "Target PROJ git tag."
  ${CARLA_PROJ_VERSION}
)

carla_declare_git_dependency (
  proj
  https://github.com/OSGeo/PROJ.git
  ${CARLA_PROJ_TAG}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DSQLITE3_INCLUDE_DIR=${sqlite3_SOURCE_DIR}
    -DSQLITE3_LIBRARY=$<TARGET_FILE:libsqlite3>
    -DEXE_SQLITE3=$<TARGET_FILE:sqlite3>
    -DWIN32_LEAN_AND_MEAN=1
    -DVC_EXTRALEAN=1
    -DNOMINMAX=1
    -DENABLE_TIFF=OFF
    -DENABLE_CURL=OFF
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_PROJSYNC=OFF
    -DBUILD_PROJINFO=OFF
    -DBUILD_CCT=OFF
    -DBUILD_CS2CS=OFF
    -DBUILD_GEOD=OFF
    -DBUILD_GIE=OFF
    -DBUILD_PROJ=OFF
    -DBUILD_TESTING=OFF
)

add_dependencies (
  proj
  libsqlite3
)
