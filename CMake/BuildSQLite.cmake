include (Util)

carla_string_option (
  CARLA_SQLITE_VERSION
  "Target SQLite version."
  3.45.0000
)

find_package (
  SQLite3
  VERSION 
)

project (
  sqlite-amalgam
  LANGUAGES C
  VERSION ${CARLA_SQLITE_VERSION}
)

carla_declare_download_dependency (
  sqlite3
  https://www.sqlite.org/2024/sqlite-amalgamation-${CARLA_SQLITE_VERSION}.zip
)

add_library (
  libsqlite3
  ${sqlite3_SOURCE_DIR}/sqlite3.h
  ${sqlite3_SOURCE_DIR}/sqlite3.c
)

add_executable (
  sqlite3
  ${sqlite3_SOURCE_DIR}/shell.c
)

target_link_libraries (
  sqlite3 PRIVATE
  libsqlite3
)
