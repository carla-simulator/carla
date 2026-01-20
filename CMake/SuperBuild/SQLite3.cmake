include (${CMAKE_CURRENT_LIST_DIR}/Config.cmake)

option (
  CARLA_SQLITE3_USE_FIND_PACKAGE
  "Whether to use find_package when retrieving ZLib."
  OFF
)

if (CARLA_SQLITE3_USE_FIND_PACKAGE)
  find_package (
      SQLite3 ${SQLITE_VERSION}
      EXACT
      QUIET
      NO_MODULE
  )
endif ()

if (NOT CARLA_SQLITE3_USE_FIND_PACKAGE OR NOT SQLite3_FOUND)
  include (FetchContent)
  
  if (NOT TARGET sqlite3_project)
    FetchContent_Declare (
      sqlite3_project
      URL https://www.sqlite.org/${SQLITE_RELEASE_YEAR}/sqlite-amalgamation-${CARLA_SQLITE_TAG}.zip
      SYSTEM OVERRIDE_FIND_PACKAGE EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable (sqlite3_project)
    message (STATUS "sqlite3_project_SOURCE_DIR=${sqlite3_project_SOURCE_DIR}")
    add_library (
      libsqlite3
      ${sqlite3_project_SOURCE_DIR}/sqlite3.h
      ${sqlite3_project_SOURCE_DIR}/sqlite3.c
    )
    add_executable (
      sqlite3
      ${sqlite3_project_SOURCE_DIR}/shell.c
    )
    if (NOT WIN32)
      target_link_libraries (libsqlite3 PRIVATE ${CMAKE_DL_LIBS})
      target_link_libraries (libsqlite3 PRIVATE Threads::Threads)
      find_library (STD_MATH_LIB m)
      target_link_libraries (libsqlite3 PRIVATE ${STD_MATH_LIB})
    endif ()
    target_link_libraries (
      sqlite3 PRIVATE
      libsqlite3
    )
  endif ()
  
endif ()
