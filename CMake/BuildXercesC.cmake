include (Util)

carla_string_option (
  CARLA_XERCESC_VERSION
  "Target xerces-c version."
  v3.2.4
)

carla_string_option (
  CARLA_XERCESC_TAG
  "Target xerces-c git tag."
  v${CARLA_XERCESC_VERSION}
)

carla_declare_git_dependency (
  xercesc
  https://github.com/apache/xerces-c.git
  ${CARLA_XERCESC_TAG}
  INSTALL_COMMAND ""
  CMAKE_ARGS
    ${CARLA_DEFAULT_DEPENDENCY_ARGS}
    -DBUILD_SHARED_LIBS=OFF
)
