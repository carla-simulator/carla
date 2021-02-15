set(jtrrouter_SRCS
   jtrrouter_main.cpp
   ROJTREdge.cpp
   ROJTREdge.h
   ROJTREdgeBuilder.cpp
   ROJTREdgeBuilder.h
   ROJTRRouter.cpp
   ROJTRRouter.h
   ROJTRTurnDefLoader.cpp
   ROJTRTurnDefLoader.h
   ROJTRFrame.cpp
   ROJTRFrame.h
)

add_executable(jtrrouter ${jtrrouter_SRCS})
set_target_properties(jtrrouter PROPERTIES OUTPUT_NAME jtrrouter${BINARY_SUFFIX})
set_target_properties(jtrrouter PROPERTIES OUTPUT_NAME_DEBUG jtrrouter${BINARY_SUFFIX}D)
target_link_libraries(jtrrouter router ${commonvehiclelibs})
add_dependencies(jtrrouter generate-version-h)

install(TARGETS jtrrouter RUNTIME DESTINATION bin)
