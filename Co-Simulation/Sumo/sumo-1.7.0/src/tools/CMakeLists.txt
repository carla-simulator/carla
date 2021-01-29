set(emissionsDrivingCycle_SRCS
   emissionsDrivingCycle_main.cpp
   TrajectoriesHandler.cpp
   TrajectoriesHandler.h
)

add_executable(emissionsDrivingCycle ${emissionsDrivingCycle_SRCS})
set_target_properties(emissionsDrivingCycle PROPERTIES OUTPUT_NAME emissionsDrivingCycle${BINARY_SUFFIX})
set_target_properties(emissionsDrivingCycle PROPERTIES OUTPUT_NAME_DEBUG emissionsDrivingCycle${BINARY_SUFFIX}D)
target_link_libraries(emissionsDrivingCycle utils_emissions foreign_phemlight ${commonlibs})
add_dependencies(emissionsDrivingCycle generate-version-h)

add_executable(emissionsMap emissionsMap_main.cpp)
set_target_properties(emissionsMap PROPERTIES OUTPUT_NAME emissionsMap${BINARY_SUFFIX})
set_target_properties(emissionsMap PROPERTIES OUTPUT_NAME_DEBUG emissionsMap${BINARY_SUFFIX}D)
target_link_libraries(emissionsMap utils_emissions foreign_phemlight ${commonlibs})
add_dependencies(emissionsMap generate-version-h)

install(TARGETS emissionsDrivingCycle emissionsMap RUNTIME DESTINATION bin)
