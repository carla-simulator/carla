set(dfrouter_SRCS
   dfrouter_main.cpp
   RODFDetFlowLoader.cpp
   RODFDetFlowLoader.h
   RODFDetector.cpp
   RODFDetector.h
   RODFDetectorFlow.cpp
   RODFDetectorFlow.h
   RODFDetectorHandler.cpp
   RODFDetectorHandler.h
   RODFNet.cpp
   RODFNet.h
   RODFRouteCont.cpp
   RODFRouteCont.h
   RODFRouteDesc.h
   RODFEdgeBuilder.cpp
   RODFEdgeBuilder.h
   RODFEdge.cpp
   RODFEdge.h
   RODFFrame.cpp
   RODFFrame.h
)

add_executable(dfrouter ${dfrouter_SRCS})
set_target_properties(dfrouter PROPERTIES OUTPUT_NAME dfrouter${BINARY_SUFFIX})
set_target_properties(dfrouter PROPERTIES OUTPUT_NAME_DEBUG dfrouter${BINARY_SUFFIX}D)
target_link_libraries(dfrouter router ${commonvehiclelibs})
add_dependencies(dfrouter generate-version-h)

install(TARGETS dfrouter RUNTIME DESTINATION bin)
