add_executable(testgeom
        BoundaryTest.cpp
        GeoConvHelperTest.cpp
        PositionVectorTest.cpp
        GeomHelperTest.cpp
        )
set_property(TARGET testgeom PROPERTY FOLDER "test_exe")
add_test(testgeom ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testgeom)

if (MSVC OR APPLE)
    target_link_libraries(testgeom ${commonlibs} ${GTEST_BOTH_LIBRARIES})
else ()
    target_link_libraries(testgeom -Wl,--start-group ${commonlibs} -Wl,--end-group ${GTEST_BOTH_LIBRARIES} -lpthread -lm)
endif ()
