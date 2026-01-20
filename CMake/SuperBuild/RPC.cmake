include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_RPC_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving RPCLib."
  OFF
)

if (CARLA_RPC_USE_FIND_PACKAGE)
    find_package (
        RPC
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_RPC_USE_FIND_PACKAGE OR NOT RPC_FOUND)

    include (ExternalProject)
    
    if (PREFER_CLONE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY
                https://github.com/carla-simulator/rpclib.git
            GIT_TAG ${RPC_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL https://github.com/carla-simulator/rpclib/archive/refs/heads/${RPC_TAG}.zip
        )
    endif ()

    if (NOT TARGET rpclib_project)
        ExternalProject_Add (
            rpclib_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_ARGS
                -DRPCLIB_BUILD_TESTS=OFF
                -DRPCLIB_GENERATE_COMPDB=OFF
                -DRPCLIB_BUILD_EXAMPLES=OFF
                -DRPCLIB_ENABLE_LOGGING=OFF
                -DRPCLIB_ENABLE_COVERAGE=OFF
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        )
        ExternalProject_Add_StepTargets (rpclib_project install) # Generates rpclib_project-install
    endif ()

endif ()
