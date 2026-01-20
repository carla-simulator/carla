include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_EIGEN_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving Eigen."
  OFF
)

if (CARLA_EIGEN_USE_FIND_PACKAGE)
    find_package (
        Eigen ${EIGEN_VERSION}
        EXACT
        QUIET
        NO_MODULE
    )
endif ()

if (NOT CARLA_EIGEN_USE_FIND_PACKAGE OR NOT Eigen_FOUND)

    include (ExternalProject)
    
    if (PREFER_CLONE)
        set (
            DOWNLOAD_OPTIONS
            GIT_REPOSITORY
                https://gitlab.com/libeigen/eigen.git
            GIT_TAG ${CARLA_EIGEN_TAG}
            GIT_SUBMODULES_RECURSE ON
            GIT_SHALLOW ON
            GIT_PROGRESS ON
        )
    else ()
        set (
            DOWNLOAD_OPTIONS
            URL https://gitlab.com/libeigen/eigen/-/archive/${CARLA_EIGEN_TAG}/eigen-${CARLA_EIGEN_TAG}.tar.gz
        )
    endif ()

    if (NOT TARGET eigen_project)
        ExternalProject_Add (
            eigen_project
            ${DOWNLOAD_OPTIONS}
            CMAKE_ARGS
                -DEIGEN_BUILD_PKGCONFIG=OFF
                -DBUILD_TESTING=OFF
                -DEIGEN_BUILD_DOC=OFF
                -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
        )
        ExternalProject_Add_StepTargets (eigen_project install) # Generates eigen_project-install
    endif ()

endif ()
