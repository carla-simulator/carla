include (ExternalProject)
include (FetchContent)

macro (carla_declare_git_dependency NAME URL TAG)
  FetchContent_Declare (
    ${NAME}
    GIT_REPOSITORY ${URL}
    GIT_TAG ${TAG}
    GIT_PROGRESS ON
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
      NAMES ${NAME}
    ${ARGN}
  )
  message ("Downloading \"${NAME}\"")
  FetchContent_MakeAvailable(${NAME})
endmacro ()

macro (carla_declare_download_dependency NAME URL)
  FetchContent_Declare (
    ${NAME}
    URL ${URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    ${ARGN}
  )
  FetchContent_MakeAvailable (${NAME})
endmacro ()