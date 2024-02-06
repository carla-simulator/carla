include (Util)

carla_string_option (
  CARLA_LIBPNG_VERSION
  "Target libpng version."
  1.6.40
)

carla_string_option (
  CARLA_LIBPNG_TAG
  "Target libpng git tag."
  v${CARLA_LIBPNG_VERSION}
)

carla_declare_git_dependency (
  libpng
  https://github.com/glennrp/libpng.git
  ${CARLA_LIBPNG_TAG}
  INSTALL_COMMAND ""
  DEPENDS
    zlib
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DPNG_TESTS=OFF
    -DPNG_SHARED=OFF
    -DPNG_TOOLS=OFF
)

add_dependencies (libpng zlib)
