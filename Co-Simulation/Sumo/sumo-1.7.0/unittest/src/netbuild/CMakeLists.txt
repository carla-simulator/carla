add_executable(testnetbuild NBHeightMapperTest.cpp
        NBTrafficLightLogicTest.cpp)
set_property(TARGET testnetbuild PROPERTY FOLDER "test_exe")
add_test(testnetbuild ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testnetbuild)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(testnetbuild netbuild ${GDAL_LIBRARY} -Wl,--start-group ${commonlibs} -Wl,--end-group ${GTEST_BOTH_LIBRARIES} -lpthread -lm)
else ()
    target_link_libraries(testnetbuild netbuild ${GDAL_LIBRARY} ${commonlibs} ${GTEST_BOTH_LIBRARIES})
endif ()

