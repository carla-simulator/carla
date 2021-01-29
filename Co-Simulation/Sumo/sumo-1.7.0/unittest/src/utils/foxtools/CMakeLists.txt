add_executable(testfoxtools
        FXWorkerThreadTest.cpp
        )
set_property(TARGET testfoxtools PROPERTY FOLDER "test_exe")
add_test(testfoxtools ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testfoxtools)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(testfoxtools ${FOX_LIBRARY} -Wl,--start-group ${commonlibs} -Wl,--end-group ${GTEST_BOTH_LIBRARIES} -lpthread -lm)
else ()
    target_link_libraries(testfoxtools ${FOX_LIBRARY} ${commonlibs} ${GTEST_BOTH_LIBRARIES})
endif ()
