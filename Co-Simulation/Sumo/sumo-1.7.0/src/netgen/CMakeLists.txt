set(netgenerate_SRCS
   netgen_main.cpp
   NGEdge.cpp
   NGEdge.h
   NGFrame.cpp
   NGFrame.h
   NGNet.h
   NGNet.cpp
   NGNode.cpp
   NGNode.h
   NGRandomNetBuilder.h
   NGRandomNetBuilder.cpp
)

add_executable(netgenerate ${netgenerate_SRCS})
set_target_properties(netgenerate PROPERTIES OUTPUT_NAME netgenerate${BINARY_SUFFIX})
set_target_properties(netgenerate PROPERTIES OUTPUT_NAME_DEBUG netgenerate${BINARY_SUFFIX}D)
target_link_libraries(netgenerate netbuild netimport netwrite ${GDAL_LIBRARY} ${commonlibs})
add_dependencies(netgenerate generate-version-h)

install(TARGETS netgenerate RUNTIME DESTINATION bin)
