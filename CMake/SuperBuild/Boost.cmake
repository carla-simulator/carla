include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_BOOST_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving boost."
  OFF
)

if (CARLA_BOOST_USE_FIND_PACKAGE)
  find_package (
    Boost ${BOOST_VERSION}
    EXACT
    QUIET
    NO_MODULE
    COMPONENTS
      asio
      geometry
      algorithm
      assert
      gil
  )
endif ()

if (NOT CARLA_BOOST_USE_FIND_PACKAGE OR NOT Boost_FOUND)
  include (ExternalProject)
    
  if (PREFER_CLONE)
    set (
      DOWNLOAD_OPTIONS
      GIT_REPOSITORY
          https://github.com/boostorg/boost.git
      GIT_TAG ${BOOST_TAG}
      GIT_SUBMODULES_RECURSE ON
      GIT_SHALLOW ON
      GIT_PROGRESS ON
    )
  else ()
    set (
      DOWNLOAD_OPTIONS
      URL https://github.com/boostorg/boost/releases/download/${BOOST_TAG}/${BOOST_TAG}.zip
    )
  endif ()

  if (NOT TARGET boost_project)
    set (
      BOOST_INCLUDED_PROJECTS
      asio
      iterator
      python
      date_time
      geometry
      container
      variant2
      filesystem
      gil
    )
    string (REPLACE ";" "," BOOST_INCLUDED_PROJECTS_ESCAPED "${BOOST_INCLUDED_PROJECTS}")
    ExternalProject_Add (
      boost_project
      ${DOWNLOAD_OPTIONS}
      LIST_SEPARATOR ,
      CMAKE_CACHE_ARGS
        -DBOOST_INCLUDE_LIBRARIES:STRING=${BOOST_INCLUDED_PROJECTS_ESCAPED}
        -DBOOST_INSTALL_LAYOUT:STRING="system"
        # -DBOOST_EXCLUDE_LIBRARIES=ON
        -DBOOST_ENABLE_PYTHON:BOOL=ON
        -DBOOST_ENABLE_MPI:BOOL=OFF
        -DBOOST_LOCALE_WITH_ICU:BOOL=OFF
        -DBOOST_LOCALE_WITH_ICONV:BOOL=OFF
        -DBOOST_GIL_BUILD_EXAMPLES:BOOL=OFF
        -DBOOST_GIL_BUILD_HEADER_TESTS:BOOL=OFF
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
        -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
    )
    ExternalProject_Add_StepTargets (boost_project install) # Generates boost_project-install
  endif ()

endif ()
