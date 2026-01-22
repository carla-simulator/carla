project (
  LibCarlaTests
)

enable_testing ()

include (GoogleTest)

set (LIBCARLA_TEST_ROOT ${CMAKE_CURRENT_LIST_DIR})



file (
  GLOB
  ROOT_TEST_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/*.h
  ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)



file (
  GLOB
  COMMON_TEST_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/common/*.h
  ${CMAKE_CURRENT_LIST_DIR}/common/*.cpp
)

add_executable (
  LibCarlaCommonTests
  ${ROOT_TEST_SOURCES}
  ${COMMON_TEST_SOURCES}
)

target_link_libraries (
  LibCarlaCommonTests PRIVATE
  gtest
  carla-client
)

target_include_directories (
  LibCarlaCommonTests PRIVATE
  ${LIBCARLA_TEST_ROOT}
)



file (
  GLOB
  CLIENT_TEST_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/client/*.h
  ${CMAKE_CURRENT_LIST_DIR}/client/*.cpp
)

add_executable (
  LibCarlaClientTests
  ${ROOT_TEST_SOURCES}
  ${CLIENT_TEST_SOURCES}
)

target_link_libraries (
  LibCarlaClientTests PRIVATE
  gtest
  carla-client
)

target_include_directories (
  LibCarlaClientTests PRIVATE
  ${LIBCARLA_TEST_ROOT}
)

target_compile_definitions (
  LibCarlaClientTests PRIVATE
  LIBCARLA_TEST_CONTENT_FOLDER="${CARLA_ROOT}/Unreal/CarlaUnreal/Content/Carla/Maps/OpenDrive"
)



file (
  GLOB
  SERVER_TEST_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/server/*.h
  ${CMAKE_CURRENT_LIST_DIR}/server/*.cpp
)

add_executable (
  LibCarlaServerTests
  ${ROOT_TEST_SOURCES}
  ${SERVER_TEST_SOURCES}
)

target_link_libraries (
  LibCarlaServerTests PRIVATE
  gtest
  carla-server
)

target_include_directories (
  LibCarlaServerTests PRIVATE
  ${LIBCARLA_TEST_ROOT}
)



gtest_discover_tests (LibCarlaCommonTests)
gtest_discover_tests (LibCarlaClientTests)
gtest_discover_tests (LibCarlaServerTests)
