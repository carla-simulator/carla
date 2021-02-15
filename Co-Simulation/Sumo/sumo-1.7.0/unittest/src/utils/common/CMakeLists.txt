add_executable(testcommon
        StringTokenizerTest.cpp
        FileHelpersTest.cpp
        StringUtilsTest.cpp
        RGBColorTest.cpp
        ValueTimeLineTest.cpp
        )
set_property(TARGET testcommon PROPERTY FOLDER "test_exe")
add_test(testcommon ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testcommon)

if (MSVC OR APPLE)
    target_link_libraries(testcommon ${commonlibs} ${GTEST_BOTH_LIBRARIES})
else ()
    target_link_libraries(testcommon -Wl,--start-group ${commonlibs} -Wl,--end-group ${GTEST_BOTH_LIBRARIES} -lpthread -lm)
endif ()
