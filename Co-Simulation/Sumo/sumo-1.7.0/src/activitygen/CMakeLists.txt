add_subdirectory(activities)
add_subdirectory(city)

set(activitygen_SRCS
   activitygen_main.cpp
   AGActivityGen.cpp
   AGActivityGen.h
   AGActivityGenHandler.cpp
   AGActivityGenHandler.h
   AGActivityTripWriter.cpp
   AGActivityTripWriter.h
   AGFrame.cpp
   AGFrame.h
)

add_executable(activitygen ${activitygen_SRCS})
set_target_properties(activitygen PROPERTIES OUTPUT_NAME activitygen${BINARY_SUFFIX})
set_target_properties(activitygen PROPERTIES OUTPUT_NAME_DEBUG activitygen${BINARY_SUFFIX}D)
target_link_libraries(activitygen activitygen_activities activitygen_city router ${commonvehiclelibs})
add_dependencies(activitygen generate-version-h)

install(TARGETS activitygen RUNTIME DESTINATION bin)
