include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_PNG_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving LibPNG."
  OFF
)

if (CARLA_PNG_USE_FIND_PACKAGE)
  find_package (
    PNG ${PNG_VERSION}
      EXACT
      QUIET
      NO_MODULE
  )
endif ()

if (NOT CARLA_PNG_USE_FIND_PACKAGE OR NOT PNG_FOUND)
  include (ExternalProject)
  if (PREFER_CLONE)
    set (
      DOWNLOAD_OPTIONS
        GIT_REPOSITORY
        https://github.com/pnggroup/libpng.git
        GIT_TAG ${PNG_TAG}
        GIT_SUBMODULES_RECURSE ON
        GIT_SHALLOW ON
        GIT_PROGRESS ON
    )
  else ()
    set (
      DOWNLOAD_OPTIONS
        URL https://github.com/pnggroup/libpng/archive/refs/tags/${PNG_TAG}.zip
    )
  endif ()
  
  if (NOT TARGET zlib_project-install)
    message (FATAL_ERROR "You must include Dependencies/ZLIB.cmake before Dependencies/PNG.cmake")
  endif ()
  
  if (NOT TARGET png_project)

  set (PNG_ZLIB_OPTIONS)

    if (TARGET zlib_project)
      ExternalProject_get_property (zlib_project INSTALL_DIR)
      if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
        if (WIN32)
          set (ZLIB_NAME zlibstaticd.lib)
        else ()
          set (ZLIB_NAME libzsd.a)
        endif ()
      else ()
        if (WIN32)
          set (ZLIB_NAME zlibstatic.lib)
        else ()
          set (ZLIB_NAME libz.a)
        endif ()
      endif ()
      set (
        PNG_ZLIB_OPTIONS
        -DZLIB_INCLUDE_DIR=${INSTALL_DIR}/include
        -DZLIB_LIBRARY=${INSTALL_DIR}/lib/${ZLIB_NAME}
      )
    endif ()

    ExternalProject_Add (
      png_project
      ${DOWNLOAD_OPTIONS}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPNG_SHARED=ON
        -DPNG_STATIC=ON
        -DPNG_TESTS=OFF
        -DPNG_TOOLS=OFF
        -DPNG_DEBUG=OFF
        -DPNG_HARDWARE_OPTIMIZATIONS=ON
        ${PNG_ZLIB_OPTIONS}
        -DCMAKE_INSTALL_PREFIX:FILEPATH=<INSTALL_DIR>
        -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
    )
    ExternalProject_Add_StepDependencies (png_project configure zlib_project-install)
    ExternalProject_Add_StepTargets (png_project install) # Generates png_project-install
  endif ()

endif ()
