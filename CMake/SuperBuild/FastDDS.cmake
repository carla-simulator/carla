include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_FAST_DDS_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving Fast-DDS."
  OFF
)

if (CARLA_FAST_DDS_USE_FIND_PACKAGE)
    find_package (
        FastDDS ${FAST_DDS_VERSION}
        EXACT
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_FAST_DDS_USE_FIND_PACKAGE OR NOT FastDDS_FOUND)

    include (ExternalProject)
    
    # FastDDS may not provide submodule in their release archives:
    set (FASTDDS_CLONE_OVERRIDE ON)

    if (PREFER_CLONE OR FASTDDS_CLONE_OVERRIDE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
            GIT_TAG ${CARLA_FAST_DDS_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL
            https://github.com/eProsima/Fast-DDS/archive/refs/tags/${CARLA_FAST_DDS_TAG}.zip
        )
    endif ()

    if (NOT TARGET fastdds_project)
        ExternalProject_Get_Property (foonathan_memory_vendor_project INSTALL_DIR)
        set (FOONATHAN_MEMORY_VENDOR_INSTALL_DIR ${INSTALL_DIR})
        ExternalProject_Add (
            fastdds_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DBUILD_SHARED_LIBS=ON
                -DBUILD_TESTING=OFF
                -DCOMPILE_EXAMPLES=OFF
                -DCOMPILE_TOOLS=OFF
                -DTHIRDPARTY_Asio=FORCE
                -DTHIRDPARTY_fastcdr=FORCE
                -DTHIRDPARTY_TinyXML2=FORCE
                -DSQLITE3_SUPPORT=OFF
                -DOPENSSL_FOUND:BOOL=ON
                -DOPENSSL_INCLUDE_DIR:FILEPATH=${UE_OPENSSL_INCLUDE}
                -DOPENSSL_SSL_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libssl.a
                -DOPENSSL_CRYPTO_LIBRARY:FILEPATH=${UE_OPENSSL_LIBS}/libcrypto.a
                -DCMAKE_PREFIX_PATH:STRING=${FOONATHAN_MEMORY_VENDOR_INSTALL_DIR}
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
            DEPENDS
                foonathan_memory_vendor_project-install
        )
        ExternalProject_Add_StepTargets (fastdds_project install) # Generates fastdds_project-install
    endif ()

endif ()
