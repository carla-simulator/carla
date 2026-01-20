include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_ZLIB_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving ZLib."
  OFF
)

if (CARLA_ZLIB_USE_FIND_PACKAGE)
    find_package (
        ZLIB ${ZLIB_VERSION}
        EXACT
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_ZLIB_USE_FIND_PACKAGE OR NOT ZLIB_FOUND)

    include (ExternalProject)
    
    if (PREFER_CLONE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY
                https://github.com/madler/zlib.git
            GIT_TAG ${ZLIB_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL https://github.com/madler/zlib/archive/refs/tags/${ZLIB_TAG}.zip
        )
    endif ()

    if (NOT TARGET zlib_project)
        ExternalProject_Add (
            zlib_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_ARGS
                -DZLIB_BUILD_EXAMPLES=OFF
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        )
        ExternalProject_Add_StepTargets (zlib_project install) # Generates eigen_project-install
    endif ()

endif ()
