include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_RECAST_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving Recast."
  OFF
)

if (CARLA_RECAST_USE_FIND_PACKAGE)
    find_package (
        RecastNavigation
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_RECAST_USE_FIND_PACKAGE OR NOT RECAST_FOUND)

    include (ExternalProject)
    
    if (PREFER_CLONE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY
                https://github.com/carla-simulator/recastnavigation.git
            GIT_TAG ${RECAST_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL https://github.com/carla-simulator/recastnavigation/archive/refs/heads/${RECAST_TAG}.zip
        )
    endif ()

    if (NOT TARGET recast_project)
        ExternalProject_Add (
            recast_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_CACHE_ARGS
                -DRECASTNAVIGATION_BUILDER:BOOL=ON
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        )
        ExternalProject_Add_StepTargets (recast_project install) # Generates recast_project-install
    endif ()

endif ()
