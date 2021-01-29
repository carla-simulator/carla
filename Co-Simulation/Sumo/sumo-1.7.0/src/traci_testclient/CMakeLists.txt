set(TraCITestClient_SRCS
   tracitestclient_main.cpp
   TraCITestClient.cpp
   TraCITestClient.h
)

add_executable(TraCITestClient ${TraCITestClient_SRCS})
set_target_properties(TraCITestClient PROPERTIES OUTPUT_NAME TraCITestClient${BINARY_SUFFIX})
set_target_properties(TraCITestClient PROPERTIES OUTPUT_NAME_DEBUG TraCITestClient${BINARY_SUFFIX}D)
target_link_libraries(TraCITestClient utils_traci foreign_tcpip)
if (MSVC)
    target_link_libraries(TraCITestClient ws2_32)
endif ()

add_executable(testlibsumo testlibsumo_main.cpp)
set_property(TARGET testlibsumo PROPERTY FOLDER "test_exe")
target_link_libraries(testlibsumo libsumocpp)
add_test(libsumotest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testlibsumo)

add_executable(testlibsumostatic testlibsumo_main.cpp)
set_property(TARGET testlibsumostatic PROPERTY FOLDER "test_exe")
target_link_libraries(testlibsumostatic libsumostatic ${sumolibs} traciserver libsumostatic microsim_cfmodels microsim_devices microsim_engine microsim_output microsim_transportables)
add_test(libsumostatictest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testlibsumostatic)
