set(marouter_SRCS
   marouter_main.cpp
   ROMAAssignments.cpp
   ROMAAssignments.h
   ROMAEdge.cpp
   ROMAEdge.h
   ROMAEdgeBuilder.cpp
   ROMAEdgeBuilder.h
   ROMAFrame.cpp
   ROMAFrame.h
   ROMARouteHandler.cpp
   ROMARouteHandler.h
)

add_executable(marouter ${marouter_SRCS})
set_target_properties(marouter PROPERTIES OUTPUT_NAME marouter${BINARY_SUFFIX})
set_target_properties(marouter PROPERTIES OUTPUT_NAME_DEBUG marouter${BINARY_SUFFIX}D)
target_link_libraries(marouter router od ${commonvehiclelibs})
add_dependencies(marouter generate-version-h)

install(TARGETS marouter RUNTIME DESTINATION bin)
