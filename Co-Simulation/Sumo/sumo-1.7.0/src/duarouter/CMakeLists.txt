set(duarouter_SRCS
   duarouter_main.cpp
   RODUAEdgeBuilder.cpp
   RODUAEdgeBuilder.h
   RODUAFrame.cpp
   RODUAFrame.h
)

add_executable(duarouter ${duarouter_SRCS})
set_target_properties(duarouter PROPERTIES OUTPUT_NAME duarouter${BINARY_SUFFIX})
set_target_properties(duarouter PROPERTIES OUTPUT_NAME_DEBUG duarouter${BINARY_SUFFIX}D)
target_link_libraries(duarouter router ${commonvehiclelibs})
add_dependencies(duarouter generate-version-h)

install(TARGETS duarouter RUNTIME DESTINATION bin)
