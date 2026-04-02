include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_FOONATHAN_MEMORY_VENDOR_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving foonathan_memory_vendor."
  OFF
)

if (CARLA_FOONATHAN_MEMORY_VENDOR_USE_FIND_PACKAGE)
    find_package (
        FoonathanMemoryVendor ${FOONATHAN_MEMORY_VENDOR_VERSION}
        EXACT
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_FOONATHAN_MEMORY_VENDOR_USE_FIND_PACKAGE OR NOT FoonathanMemoryVendor_FOUND)

    include (ExternalProject)
    
    if (PREFER_CLONE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY https://github.com/eProsima/foonathan_memory_vendor.git
            GIT_TAG ${FOONATHAN_MEMORY_VENDOR_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL
            https://github.com/eProsima/foonathan_memory_vendor/archive/refs/heads/${FOONATHAN_MEMORY_VENDOR_TAG}.zip
        )
    endif ()

    if (NOT TARGET foonathan_memory_vendor_project)
        ExternalProject_Add (
            foonathan_memory_vendor_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DFOONATHAN_MEMORY_FORCE_VENDORED_BUILD=ON
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        )
        ExternalProject_Add_StepTargets (foonathan_memory_vendor_project install) # Generates foonathan_memory_vendor_project-install
    endif ()

endif ()
