file (
  GLOB
  CARLA_PACKAGE_FILES
  ${CARLA_PACKAGE_ARCHIVE_PATH}/*
)

if (WIN32)
  set (
    COMPRESS_PACKAGE_COMMAND
    ${CMAKE_COMMAND}
      -E tar cvf
      ${CARLA_CURRENT_PACKAGE_PATH}.zip
      --format=zip
    ${CARLA_PACKAGE_FILES}
  )
else ()
  set (
    COMPRESS_PACKAGE_COMMAND
    ${CMAKE_COMMAND}
      -E tar cvfz
      ${CARLA_CURRENT_PACKAGE_PATH}.tar.gz
    ${CARLA_PACKAGE_FILES}
  )
endif ()

message (
  STATUS
  "Running \"${COMPRESS_PACKAGE_COMMAND}\""
)

execute_process (
  COMMAND
    ${COMPRESS_PACKAGE_COMMAND}
  WORKING_DIRECTORY
    ${CARLA_PACKAGE_PATH}
)
